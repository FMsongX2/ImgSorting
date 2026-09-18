#include "audio_decode.h"
#include "sorts.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stb_image.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <numeric>
#include <random>
#include <string>
#include <utility>

extern const unsigned char kDefaultWav[];
extern const std::size_t kDefaultWavSize;
extern const unsigned char kStalinJpeg[];
extern const std::size_t kStalinJpegSize;

namespace {

constexpr int kStripCount = 100;
constexpr float kSortSeconds = 10.f;
constexpr float kAfterDoneSilence = 0.8f;
constexpr float kSweepSeconds = 1.f;
constexpr float kSweptHoldSeconds = 0.2f;
constexpr float kMaxFrameSeconds = 0.1f;
constexpr float kHeaderHeight = 36.f;
constexpr float kOnsetRatio = 0.1f;
constexpr float kPreRollSeconds = 0.005f;
constexpr float kTriggerSeconds = 2048.f / 48000.f;
constexpr int kVoiceCount = 32;
constexpr float kVoiceGain = 0.25f;
constexpr float kPitchSpread = 0.05f;
constexpr float kSyllableGapRatio = 0.02f;
constexpr float kSyllableGapSeconds = 0.02f;
constexpr float kMaxSyllableSeconds = 1.5f;
constexpr float kControlWidth = 560.f, kControlHeight = 180.f;
constexpr int kListColumns = 3;
constexpr float kListItemW = (kControlWidth - 8.f * (kListColumns + 1)) / kListColumns;
constexpr float kListItemH = 22.f;
constexpr int kGeneralCount = static_cast<int>(
    std::count_if(std::begin(kSortAlgorithms), std::end(kSortAlgorithms), [](const SortAlgorithm& a) { return a.general; }));
constexpr float kGlyph = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
constexpr const char* kSavedAudioExtensions[] = {".wav", ".m4a"};

struct Player {
    std::mt19937 rng{std::random_device{}()};
    int algorithm = 0;
    std::vector<int> initial;
    std::vector<int> order;
    std::vector<Op> ops;
    size_t step = 0;
    bool running = false;
    bool usesWrites = false;
    long comparisons = 0, swaps = 0, writes = 0;
    float elapsed = 0, stepBudget = 0;
    bool stepped = false;
    bool finalePlayed = false;
    bool gaveUp = false;
    float doneSilence = 0;
    float sweep = 0;
    Op last{Op::Compare, -1, -1};

    std::vector<int> shuffled() {
        std::vector<int> v(kStripCount);
        std::iota(v.begin(), v.end(), 0);
        std::shuffle(v.begin(), v.end(), rng);
        return v;
    }

    void load(int algo, std::vector<int> start) {
        algorithm = algo;
        initial = std::move(start);
        order = initial;
        ops = recordSort(algorithm, initial);
        usesWrites = std::any_of(ops.begin(), ops.end(), [](const Op& op) { return op.kind == Op::Write; });
        step = 0;
        comparisons = swaps = writes = 0;
        elapsed = stepBudget = doneSilence = sweep = 0;
        finalePlayed = gaveUp = false;
        last = {Op::Compare, -1, -1};
    }

    bool finished() const { return step >= ops.size(); }

    int sweptStrips() const {
        if (!finished() || gaveUp) return 0;
        return std::min(kStripCount, static_cast<int>(kStripCount * sweep / kSweepSeconds));
    }
    bool revealed() const { return !gaveUp && sweep >= kSweepSeconds + kSweptHoldSeconds; }

    void apply(const Op& op) {
        applyOp(order, op);
        (op.kind == Op::Compare ? comparisons : op.kind == Op::Swap ? swaps : writes)++;
        stepped = true;
        last = op;
    }

    void update(float dt) {
        stepped = false;
        if (!running) return;
        dt = std::min(dt, kMaxFrameSeconds);
        if (finished()) {
            last = {Op::Compare, -1, -1};
            sweep += dt;
            return;
        }
        elapsed += dt;
        const bool endless = kSortAlgorithms[algorithm].endless;
        if (endless && elapsed >= kSortSeconds) {
            gaveUp = true;
            ops.clear();
            step = 0;
            return;
        }
        stepBudget += dt * static_cast<float>(ops.size()) / kSortSeconds;
        while (stepBudget >= 1.f && !finished()) {
            apply(ops[step++]);
            stepBudget -= 1.f;
            if (finished() && endless && !std::is_sorted(order.begin(), order.end())) {
                ops = recordSort(algorithm, order);
                step = 0;
            }
        }
    }
};

struct SwapSound {
    static constexpr SDL_AudioSpec kSpec{SDL_AUDIO_F32, 2, 48000};
    SDL_AudioDeviceID device = 0;
    SDL_AudioStream* voices[kVoiceCount] = {};
    SDL_AudioStream* finale = nullptr;
    std::vector<float> samples;
    size_t syllableEnd = 0;
    Uint64 lastTriggerNs = 0;
    std::mt19937 rng{std::random_device{}()};
    float volume = 1.f;

    void open() {
        device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (!device) {
            SDL_Log("audio device open failed: %s", SDL_GetError());
            return;
        }
        for (SDL_AudioStream*& voice : voices) {
            voice = bind(SDL_CreateAudioStream(&kSpec, &kSpec));
            if (voice) SDL_SetAudioStreamGain(voice, kVoiceGain);
        }
        finale = bind(SDL_CreateAudioStream(&kSpec, &kSpec));
    }
    SDL_AudioStream* bind(SDL_AudioStream* stream) const {
        if (stream && SDL_BindAudioStream(device, stream)) return stream;
        SDL_DestroyAudioStream(stream);
        return nullptr;
    }
    bool load(const char* path) {
        if (load(SDL_IOFromFile(path, "rb"), path)) return true;
        std::vector<float> decoded;
        int channels = 0, rate = 0;
        if (!decodeAudioFile(path, decoded, channels, rate)) {
#if defined(__linux__)
            SDL_SetError("%s: cannot decode (WAV or M4A; M4A needs ffmpeg on Linux)", path);
#else
            SDL_SetError("%s: cannot decode (WAV or M4A)", path);
#endif
            return false;
        }
        const SDL_AudioSpec spec{SDL_AUDIO_F32, channels, rate};
        return setSamples(spec, reinterpret_cast<const Uint8*>(decoded.data()), static_cast<int>(decoded.size() * sizeof(float)), path);
    }
    bool load(SDL_IOStream* io, const char* label) {
        SDL_AudioSpec wavSpec{};
        Uint8* wavData = nullptr;
        Uint32 wavSize = 0;
        if (!SDL_LoadWAV_IO(io, true, &wavSpec, &wavData, &wavSize)) return false;
        const bool ok = setSamples(wavSpec, wavData, static_cast<int>(wavSize), label);
        SDL_free(wavData);
        return ok;
    }
    bool setSamples(const SDL_AudioSpec& spec, const Uint8* data, int size, const char* label) {
        Uint8* converted = nullptr;
        int convertedSize = 0;
        if (!SDL_ConvertAudioSamples(&spec, data, size, &kSpec, &converted, &convertedSize)) return false;
        const auto* all = reinterpret_cast<const float*>(converted);
        const size_t count = static_cast<size_t>(convertedSize) / sizeof(float);
        const size_t start = soundStart(all, count);
        samples.assign(all + start, all + count);
        SDL_free(converted);
        syllableEnd = std::min(firstGap(samples), static_cast<size_t>(kMaxSyllableSeconds * kSpec.freq) * kSpec.channels);
        SDL_Log("swap sound: %s, leading silence trimmed %.0f ms, syllable %.0f ms, full %.0f ms", label,
                1000.0 * static_cast<double>(start) / kSpec.channels / kSpec.freq,
                1000.0 * static_cast<double>(syllableEnd) / kSpec.channels / kSpec.freq,
                1000.0 * static_cast<double>(samples.size()) / kSpec.channels / kSpec.freq);
        return true;
    }
    static size_t soundStart(const float* s, size_t count) {
        float peak = 0;
        for (size_t i = 0; i < count; ++i) peak = std::max(peak, std::fabs(s[i]));
        if (peak == 0) return 0;
        size_t i = 0;
        while (i < count && std::fabs(s[i]) < peak * kOnsetRatio) ++i;
        const size_t frame = i / kSpec.channels;
        const size_t preRoll = static_cast<size_t>(kPreRollSeconds * kSpec.freq);
        return (frame > preRoll ? frame - preRoll : 0) * kSpec.channels;
    }
    static size_t firstGap(const std::vector<float>& s) {
        float peak = 0;
        for (float v : s) peak = std::max(peak, std::fabs(v));
        const size_t need = static_cast<size_t>(kSyllableGapSeconds * kSpec.freq);
        size_t run = 0;
        for (size_t f = 0; f < s.size() / kSpec.channels; ++f) {
            const float a = std::max(std::fabs(s[f * 2]), std::fabs(s[f * 2 + 1]));
            run = a < peak * kSyllableGapRatio ? run + 1 : 0;
            if (run >= need) return (f + 1 - run) * kSpec.channels;
        }
        return s.size();
    }
    void trigger() {
        const Uint64 now = SDL_GetTicksNS();
        const Uint64 period = static_cast<Uint64>(kTriggerSeconds * 1e9f);
        if (samples.empty() || now - lastTriggerNs < period) return;
        for (SDL_AudioStream* voice : voices)
            if (voice && SDL_GetAudioStreamQueued(voice) == 0) {
                SDL_SetAudioStreamFrequencyRatio(voice, 1.f + std::uniform_real_distribution<float>(-kPitchSpread, kPitchSpread)(rng));
                put(voice, syllableEnd);
                lastTriggerNs = now - lastTriggerNs < 2 * period ? lastTriggerNs + period : now;
                return;
            }
    }
    void playFull() {
        if (!finale || samples.empty()) return;
        SDL_ClearAudioStream(finale);
        put(finale, samples.size());
    }
    void put(SDL_AudioStream* stream, size_t count) {
        SDL_PutAudioStreamData(stream, samples.data(), static_cast<int>(count * sizeof(float)));
        SDL_FlushAudioStream(stream);
    }
    void setVolume(float v) {
        volume = v;
        SDL_SetAudioDeviceGain(device, v * v);
    }
    bool idle() const {
        if (finale && SDL_GetAudioStreamQueued(finale) > 0) return false;
        for (SDL_AudioStream* voice : voices)
            if (voice && SDL_GetAudioStreamQueued(voice) > 0) return false;
        return true;
    }
    void close() {
        for (SDL_AudioStream*& voice : voices) {
            SDL_DestroyAudioStream(voice);
            voice = nullptr;
        }
        SDL_DestroyAudioStream(finale);
        finale = nullptr;
        SDL_CloseAudioDevice(device);
        device = 0;
    }
};

enum class Action { ImportImage, ImportAudio, Fit, General, Prev, Next, Start, Stop, Restart, Shuffle };

struct Button {
    SDL_FRect rect;
    const char* label;
    Action action;
};

constexpr SDL_FRect kSortNameRow{56, 52, 448, 40};
constexpr SDL_FRect kVolumeTrack{96, 144, 392, 28};
const Button kButtons[] = {
    {{8, 8, 164, 36}, "Import Image", Action::ImportImage},
    {{180, 8, 164, 36}, "Import Audio", Action::ImportAudio},
    {{352, 8, 80, 36}, "Fit", Action::Fit},
    {{440, 8, 112, 36}, "General", Action::General},
    {{8, 52, 48, 40}, "<", Action::Prev},
    {{504, 52, 48, 40}, ">", Action::Next},
    {{8, 100, 130, 36}, "Start", Action::Start},
    {{146, 100, 130, 36}, "Stop", Action::Stop},
    {{284, 100, 130, 36}, "Restart", Action::Restart},
    {{422, 100, 130, 36}, "Shuffle", Action::Shuffle},
};

struct PendingPath {
    std::mutex mutex;
    std::string path;

    std::string take() {
        std::lock_guard lock(mutex);
        return std::exchange(path, {});
    }
};

void SDLCALL onFileChosen(void* userdata, const char* const* files, int) {
    if (!files || !files[0]) return;
    auto* pending = static_cast<PendingPath*>(userdata);
    std::lock_guard lock(pending->mutex);
    pending->path = files[0];
}

SDL_Texture* uploadPixels(SDL_Renderer* renderer, stbi_uc* pixels, int w, int h, const char* label) {
    if (!pixels) {
        SDL_SetError("%s: %s", label, stbi_failure_reason());
        return nullptr;
    }
    SDL_Surface* surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, w * 4);
    SDL_Texture* texture = surface ? SDL_CreateTextureFromSurface(renderer, surface) : nullptr;
    SDL_DestroySurface(surface);
    stbi_image_free(pixels);
    return texture;
}

SDL_Texture* loadImage(SDL_Renderer* renderer, const char* path) {
    int w = 0, h = 0, channels = 0;
    return uploadPixels(renderer, stbi_load(path, &w, &h, &channels, 4), w, h, path);
}

SDL_Texture* loadImage(SDL_Renderer* renderer, const unsigned char* data, std::size_t size, const char* label) {
    int w = 0, h = 0, channels = 0;
    return uploadPixels(renderer, stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &channels, 4), w, h, label);
}

SDL_Texture* makeGradient(SDL_Renderer* renderer, int w, int h) {
    SDL_Surface* surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    for (int y = 0; y < h; ++y) {
        auto* row = static_cast<Uint8*>(surface->pixels) + y * surface->pitch;
        const float value = 1.f - 0.6f * y / h;
        for (int x = 0; x < w; ++x) {
            const float hue = 6.f * x / w;
            const float f = hue - std::floor(hue);
            const float rgb[6][3] = {{1, f, 0}, {1 - f, 1, 0}, {0, 1, f}, {0, 1 - f, 1}, {f, 0, 1}, {1, 0, 1 - f}};
            const float* c = rgb[static_cast<int>(hue) % 6];
            for (int i = 0; i < 3; ++i) row[x * 4 + i] = static_cast<Uint8>(255 * value * c[i]);
            row[x * 4 + 3] = 255;
        }
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}

void fitWindowToImage(SDL_Window* window, SDL_Texture* image) {
    const SDL_WindowFlags flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN) SDL_SetWindowFullscreen(window, false);
    if (flags & SDL_WINDOW_MAXIMIZED) SDL_RestoreWindow(window);
    if (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED)) SDL_SyncWindow(window);
    float imageW = 0, imageH = 0;
    SDL_GetTextureSize(image, &imageW, &imageH);
    const SDL_DisplayID display = SDL_GetDisplayForWindow(window);
    SDL_Rect usable{};
    SDL_GetDisplayUsableBounds(display, &usable);
    int top = 0, left = 0, bottom = 0, right = 0;
    SDL_GetWindowBordersSize(window, &top, &left, &bottom, &right);
    const float maxW = static_cast<float>(usable.w - left - right);
    const float maxH = static_cast<float>(usable.h - top - bottom) - kHeaderHeight;
    const float scale = std::min({1.f, maxW / imageW, maxH / imageH});
    SDL_SetWindowSize(window, static_cast<int>(std::lround(imageW * scale)),
                      static_cast<int>(std::lround(imageH * scale + kHeaderHeight)));
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display));
}

void drawText(SDL_Renderer* renderer, float density, float x, float y, float scale, const char* text, bool bold = false) {
    SDL_SetRenderScale(renderer, density * scale, density * scale);
    SDL_RenderDebugText(renderer, x / scale, y / scale, text);
    if (bold) SDL_RenderDebugText(renderer, (x + 1) / scale, y / scale, text);
    SDL_SetRenderScale(renderer, density, density);
}

void drawSortWindow(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* image, SDL_Texture* stalinImage,
                    const Player& player) {
    const float density = SDL_GetWindowPixelDensity(window);
    int winW = 0, winH = 0;
    SDL_GetWindowSize(window, &winW, &winH);
    const float w = static_cast<float>(winW), h = static_cast<float>(winH);
    SDL_SetRenderScale(renderer, density, density);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 12, 12, 12, 255);
    const SDL_FRect header{0, 0, w, kHeaderHeight};
    SDL_RenderFillRect(renderer, &header);
    const SortAlgorithm& algo = kSortAlgorithms[player.algorithm];
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    drawText(renderer, density, 12, (kHeaderHeight - 2 * kGlyph) / 2, 2, algo.name, true);
    char writes[40] = "";
    if (player.usesWrites) std::snprintf(writes, sizeof writes, " | Writes: %ld", player.writes);
    char stats[200];
    std::snprintf(stats, sizeof stats, "Comparisons: %ld | Swaps: %ld%s | Time: %.2fs | %s", player.comparisons,
                  player.swaps, writes, player.elapsed, algo.complexity);
    const float statsX = 12 + 2 * kGlyph * std::strlen(algo.name) + 24;
    const float statsScale = std::clamp((w - statsX - 12) / (kGlyph * std::strlen(stats)), 1.f, 1.5f);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    drawText(renderer, density, statsX, (kHeaderHeight - statsScale * kGlyph) / 2, statsScale, stats);

    float imageW = 0, imageH = 0;
    SDL_GetTextureSize(image, &imageW, &imageH);
    const float dw = w, dh = h - kHeaderHeight;
    const float dx = 0, dy = kHeaderHeight;
    const float srcStrip = imageW / kStripCount, dstStrip = dw / kStripCount;
    float stalinW = 0, stalinH = 0;
    if (stalinImage) SDL_GetTextureSize(stalinImage, &stalinW, &stalinH);
    const float stalinStrip = stalinW / kStripCount;
    const float inset = std::min(0.5f, srcStrip * 0.25f), stalinInset = std::min(0.5f, stalinStrip * 0.25f);
    for (int i = 0; i < kStripCount; ++i) {
        const SDL_FRect dst{dx + i * dstStrip, dy, dstStrip, dh};
        if (player.order[i] == kPurged) {
            if (!stalinImage) continue;
            const SDL_FRect src{i * stalinStrip + stalinInset, 0, stalinStrip - 2 * stalinInset, stalinH};
            SDL_RenderTexture(renderer, stalinImage, &src, &dst);
            continue;
        }
        const SDL_FRect src{player.order[i] * srcStrip + inset, 0, srcStrip - 2 * inset, imageH};
        SDL_RenderTexture(renderer, image, &src, &dst);
    }
    const int swept = player.sweptStrips();
    if (dstStrip >= 4) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
        for (int i = swept + 1; i < kStripCount; ++i) {
            const SDL_FRect line{dx + i * dstStrip - 0.5f, dy, 1, dh};
            SDL_RenderFillRect(renderer, &line);
        }
    }

    const Op& op = player.last;
    if (op.a >= 0) {
        if (op.kind == Op::Compare) SDL_SetRenderDrawColor(renderer, 60, 220, 90, 190);
        else SDL_SetRenderDrawColor(renderer, 235, 60, 60, 190);
        const float barW = std::max(dstStrip, 2.f);
        for (int index : {op.a, op.kind == Op::Write ? -1 : op.b}) {
            if (index < 0) continue;
            const SDL_FRect bar{dx + index * dstStrip + (dstStrip - barW) / 2, dy, barW, dh};
            SDL_RenderFillRect(renderer, &bar);
        }
    }

    if (swept > 0 && !player.revealed()) {
        SDL_SetRenderDrawColor(renderer, 120, 200, 255, 80);
        const SDL_FRect tint{dx, dy, swept * dstStrip, dh};
        SDL_RenderFillRect(renderer, &tint);
    }
    SDL_RenderPresent(renderer);
}

bool listed(int k, bool general) { return !general || kSortAlgorithms[k].general; }

int stepSort(int current, int delta, bool general) {
    int k = current;
    do k = (k + delta + kSortCount) % kSortCount;
    while (!listed(k, general));
    return k;
}

int listRows(bool general) { return ((general ? kGeneralCount : kSortCount) + kListColumns - 1) / kListColumns; }

SDL_FRect listItemRect(int slot, bool general) {
    const int rows = listRows(general);
    const int col = slot / rows, row = slot % rows;
    return {8 + col * (kListItemW + 8), kControlHeight + row * (kListItemH + 4), kListItemW, kListItemH};
}

void setListOpen(SDL_Window* window, bool open, bool general) {
    const float listHeight = listRows(general) * (kListItemH + 4.f) + 4.f;
    const int h = static_cast<int>(kControlHeight + (open ? listHeight : 0));
    SDL_SetWindowSize(window, static_cast<int>(kControlWidth), h);
    int x = 0, y = 0, top = 0, left = 0, bottom = 0, right = 0;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_GetWindowBordersSize(window, &top, &left, &bottom, &right);
    SDL_Rect usable{};
    SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(window), &usable);
    const int screenBottom = usable.y + usable.h;
    if (y + h > screenBottom) SDL_SetWindowPosition(window, x, std::max(usable.y + top, screenBottom - h));
}

float volumeAt(float x) { return std::clamp((x - kVolumeTrack.x) / kVolumeTrack.w, 0.f, 1.f); }

void drawControlWindow(SDL_Window* window, SDL_Renderer* renderer, int current, bool listOpen, bool general, float volume) {
    const float density = SDL_GetWindowPixelDensity(window);
    SDL_SetRenderScale(renderer, density, density);
    SDL_SetRenderDrawColor(renderer, 32, 32, 36, 255);
    SDL_RenderClear(renderer);
    SDL_FPoint mouse{-1, -1};
    if (SDL_GetMouseFocus() == window) SDL_GetMouseState(&mouse.x, &mouse.y);
    constexpr float kLabelScale = 1.5f;
    for (const Button& b : kButtons) {
        const bool hover = SDL_PointInRectFloat(&mouse, &b.rect);
        if (b.action == Action::General && general) SDL_SetRenderDrawColor(renderer, 60, 120, 200, 255);
        else SDL_SetRenderDrawColor(renderer, hover ? 90 : 62, hover ? 90 : 62, hover ? 100 : 70, 255);
        SDL_RenderFillRect(renderer, &b.rect);
        SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
        const float textW = kLabelScale * kGlyph * std::strlen(b.label);
        drawText(renderer, density, b.rect.x + (b.rect.w - textW) / 2, b.rect.y + (b.rect.h - kLabelScale * kGlyph) / 2,
                 kLabelScale, b.label);
    }
    const bool nameHover = SDL_PointInRectFloat(&mouse, &kSortNameRow);
    SDL_SetRenderDrawColor(renderer, nameHover ? 60 : 44, nameHover ? 60 : 44, nameHover ? 70 : 52, 255);
    SDL_RenderFillRect(renderer, &kSortNameRow);
    constexpr float kNameScale = 2.f;
    const char* sortName = kSortAlgorithms[current].name;
    const float nameW = kNameScale * kGlyph * std::strlen(sortName);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    drawText(renderer, density, kSortNameRow.x + (kSortNameRow.w - nameW) / 2,
             kSortNameRow.y + (kSortNameRow.h - kNameScale * kGlyph) / 2, kNameScale, sortName, true);
    SDL_SetRenderDrawColor(renderer, 170, 170, 180, 255);
    drawText(renderer, density, kSortNameRow.x + kSortNameRow.w - 22, kSortNameRow.y + (kSortNameRow.h - kLabelScale * kGlyph) / 2,
             kLabelScale, listOpen ? "^" : "v");
    const float volumeTextY = kVolumeTrack.y + (kVolumeTrack.h - kLabelScale * kGlyph) / 2;
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    drawText(renderer, density, 12, volumeTextY, kLabelScale, "Volume");
    const SDL_FRect track{kVolumeTrack.x, kVolumeTrack.y + kVolumeTrack.h / 2 - 3, kVolumeTrack.w, 6};
    SDL_SetRenderDrawColor(renderer, 62, 62, 70, 255);
    SDL_RenderFillRect(renderer, &track);
    const SDL_FRect filled{track.x, track.y, track.w * volume, track.h};
    SDL_SetRenderDrawColor(renderer, 60, 120, 200, 255);
    SDL_RenderFillRect(renderer, &filled);
    const Uint8 knobShade = SDL_PointInRectFloat(&mouse, &kVolumeTrack) ? 255 : 215;
    SDL_SetRenderDrawColor(renderer, knobShade, knobShade, knobShade, 255);
    const SDL_FRect knob{kVolumeTrack.x + kVolumeTrack.w * volume - 5, kVolumeTrack.y + 4, 10, kVolumeTrack.h - 8};
    SDL_RenderFillRect(renderer, &knob);
    char percent[8];
    std::snprintf(percent, sizeof percent, "%d%%", static_cast<int>(std::lround(volume * 100)));
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    drawText(renderer, density, kControlWidth - 12 - kLabelScale * kGlyph * std::strlen(percent), volumeTextY, kLabelScale, percent);
    if (listOpen) {
        for (int k = 0, slot = 0; k < kSortCount; ++k) {
            if (!listed(k, general)) continue;
            const SDL_FRect item = listItemRect(slot++, general);
            const bool hover = SDL_PointInRectFloat(&mouse, &item);
            if (k == current) SDL_SetRenderDrawColor(renderer, 60, 120, 200, 255);
            else SDL_SetRenderDrawColor(renderer, hover ? 90 : 52, hover ? 90 : 52, hover ? 100 : 60, 255);
            SDL_RenderFillRect(renderer, &item);
            SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
            drawText(renderer, density, item.x + 6, item.y + (item.h - kGlyph) / 2, 1.f, kSortAlgorithms[k].name);
        }
    }
    SDL_RenderPresent(renderer);
}

bool importImage(const std::string& path, SDL_Renderer* renderer, SDL_Texture*& image, Player& player, SDL_Window* parent) {
    SDL_Texture* loaded = loadImage(renderer, path.c_str());
    if (!loaded) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Image import failed", SDL_GetError(), parent);
        return false;
    }
    SDL_DestroyTexture(image);
    image = loaded;
    player.load(player.algorithm, player.shuffled());
    return true;
}

void keepCopy(const std::string& from, const std::string& to) {
    std::error_code ec;
    if (to.empty() || std::filesystem::equivalent(from, to, ec)) return;
    if (!SDL_CopyFile(from.c_str(), to.c_str())) SDL_Log("failed to keep %s: %s", from.c_str(), SDL_GetError());
}

void importSound(const std::string& path, SwapSound& sound, const std::string& savedBase, SDL_Window* parent) {
    if (!sound.load(path.c_str())) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio import failed", SDL_GetError(), parent);
        return;
    }
    sound.playFull();
    if (savedBase.empty()) return;
    std::string ext = std::filesystem::path(path).extension().string();
    for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (ext != ".m4a") ext = ".wav";
    for (const char* other : kSavedAudioExtensions)
        if (ext != other) SDL_RemovePath((savedBase + other).c_str());
    keepCopy(path, savedBase + ext);
}

}

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    SDL_Window* sortWindow = nullptr;
    SDL_Renderer* sortRenderer = nullptr;
    SDL_Window* controlWindow = nullptr;
    SDL_Renderer* controlRenderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("ImgSorting", 960, 540 + static_cast<int>(kHeaderHeight),
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &sortWindow, &sortRenderer) ||
        !SDL_CreateWindowAndRenderer("Controls", static_cast<int>(kControlWidth), static_cast<int>(kControlHeight),
                                     SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY, &controlWindow,
                                     &controlRenderer)) {
        SDL_Log("window creation failed: %s", SDL_GetError());
        return 1;
    }
    SDL_SetRenderVSync(sortRenderer, 1);
    SDL_Rect usable{};
    SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(sortWindow), &usable);
    SDL_SetWindowPosition(controlWindow, usable.x + usable.w - static_cast<int>(kControlWidth) - 24,
                          usable.y + usable.h - static_cast<int>(kControlHeight) - 48);

    char* prefDir = SDL_GetPrefPath("FMsongX2", "ImgSorting");
    const std::string savedImagePath = prefDir ? std::string(prefDir) + "last_image" : "";
    const std::string savedAudioBase = prefDir ? std::string(prefDir) + "last_audio" : "";
    SDL_free(prefDir);

    SDL_Texture* image = argc > 1 ? loadImage(sortRenderer, argv[1]) : nullptr;
    if (argc > 1 && !image) SDL_Log("failed to load %s: %s", argv[1], SDL_GetError());
    if (!image && !savedImagePath.empty()) image = loadImage(sortRenderer, savedImagePath.c_str());
    if (!image) image = makeGradient(sortRenderer, 960, 540);
    SDL_Texture* stalinImage = loadImage(sortRenderer, kStalinJpeg, kStalinJpegSize, "built-in stalin.jpeg");
    if (!stalinImage) SDL_Log("stalin image not loaded: %s", SDL_GetError());

    Player player;
    player.load(0, player.shuffled());
    SwapSound swapSound;
    swapSound.open();
    bool savedLoaded = false;
    for (const char* ext : kSavedAudioExtensions)
        if (!savedLoaded && !savedAudioBase.empty()) savedLoaded = swapSound.load((savedAudioBase + ext).c_str());
    if (!savedLoaded && !swapSound.load(SDL_IOFromConstMem(kDefaultWav, kDefaultWavSize), "built-in default.wav"))
        SDL_Log("swap sound not loaded: %s", SDL_GetError());
    PendingPath pendingImage, pendingAudio;
    bool listOpen = false;
    bool general = false;
    bool volumeDragging = false;
    static constexpr SDL_DialogFileFilter kImageFilters[] = {{"Images (PNG, JPG, BMP)", "png;jpg;jpeg;bmp"}};
    static constexpr SDL_DialogFileFilter kAudioFilters[] = {{"Audio (WAV, M4A)", "wav;m4a"}};

    Uint64 prev = SDL_GetTicksNS();
    for (bool quit = false; !quit;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) quit = true;
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) volumeDragging = false;
            if (event.type == SDL_EVENT_MOUSE_MOTION && volumeDragging && event.motion.windowID == SDL_GetWindowID(controlWindow))
                swapSound.setVolume(volumeAt(event.motion.x));
            if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT ||
                event.button.windowID != SDL_GetWindowID(controlWindow))
                continue;
            const SDL_FPoint p{event.button.x, event.button.y};
            if (SDL_PointInRectFloat(&p, &kVolumeTrack)) {
                volumeDragging = true;
                swapSound.setVolume(volumeAt(p.x));
                continue;
            }
            if (SDL_PointInRectFloat(&p, &kSortNameRow)) {
                listOpen = !listOpen;
                setListOpen(controlWindow, listOpen, general);
                continue;
            }
            for (int k = 0, slot = 0; listOpen && k < kSortCount; ++k) {
                if (!listed(k, general)) continue;
                const SDL_FRect item = listItemRect(slot++, general);
                if (!SDL_PointInRectFloat(&p, &item)) continue;
                player.load(k, player.shuffled());
                listOpen = false;
                setListOpen(controlWindow, false, general);
            }
            for (const Button& b : kButtons) {
                if (!SDL_PointInRectFloat(&p, &b.rect)) continue;
                switch (b.action) {
                case Action::ImportImage:
                    SDL_ShowOpenFileDialog(onFileChosen, &pendingImage, controlWindow, kImageFilters, 1, nullptr, false);
                    break;
                case Action::ImportAudio:
                    SDL_ShowOpenFileDialog(onFileChosen, &pendingAudio, controlWindow, kAudioFilters, 1, nullptr, false);
                    break;
                case Action::Fit: fitWindowToImage(sortWindow, image); break;
                case Action::General:
                    general = !general;
                    if (!listed(player.algorithm, general)) player.load(stepSort(player.algorithm, 1, general), player.shuffled());
                    if (listOpen) setListOpen(controlWindow, true, general);
                    break;
                case Action::Shuffle: player.load(player.algorithm, player.shuffled()); break;
                case Action::Prev: player.load(stepSort(player.algorithm, -1, general), player.shuffled()); break;
                case Action::Next: player.load(stepSort(player.algorithm, 1, general), player.shuffled()); break;
                case Action::Start: player.running = true; break;
                case Action::Stop: player.running = false; break;
                case Action::Restart:
                    player.load(player.algorithm, player.initial);
                    player.running = true;
                    break;
                }
            }
        }

        if (const std::string path = pendingImage.take(); !path.empty() && importImage(path, sortRenderer, image, player, controlWindow))
            keepCopy(path, savedImagePath);
        if (const std::string path = pendingAudio.take(); !path.empty())
            importSound(path, swapSound, savedAudioBase, controlWindow);

        const Uint64 now = SDL_GetTicksNS();
        const float dt = (now - prev) / 1e9f;
        prev = now;
        player.update(dt);
        if (player.stepped) swapSound.trigger();
        if (player.running && player.finished()) {
            if (!player.finalePlayed) {
                if (!player.gaveUp) swapSound.playFull();
                player.finalePlayed = true;
            } else if (!swapSound.idle() || (!player.gaveUp && !player.revealed())) {
                player.doneSilence = 0;
            } else if ((player.doneSilence += std::min(dt, kMaxFrameSeconds)) >= kAfterDoneSilence) {
                if (const int next = stepSort(player.algorithm, 1, general); next > player.algorithm) player.load(next, player.shuffled());
                else quit = true;
            }
        }
        drawSortWindow(sortWindow, sortRenderer, image, stalinImage, player);
        drawControlWindow(controlWindow, controlRenderer, player.algorithm, listOpen, general, swapSound.volume);
    }

    swapSound.close();
    SDL_DestroyTexture(stalinImage);
    SDL_DestroyTexture(image);
    SDL_Quit();
    return 0;
}
