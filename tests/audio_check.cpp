#include "audio_decode.h"

#include <cmath>
#include <cstdio>

int main(int argc, char** argv) {
    if (argc < 2) return 2;
    std::vector<float> samples;
    int channels = 0, rate = 0;
    if (!decodeAudioFile(argv[1], samples, channels, rate)) {
        std::puts("audio_check failed: decode");
        return 1;
    }
    const double seconds = static_cast<double>(samples.size()) / channels / rate;
    float peak = 0;
    for (float v : samples) peak = std::fmax(peak, std::fabs(v));
    std::printf("decoded %d ch %d Hz %.3f s peak %.3f\n", channels, rate, seconds, peak);
    const bool ok = seconds > 1.2 && seconds < 1.5 && peak > 0.3 && peak < 1.0;
    std::puts(ok ? "audio_check ok" : "audio_check failed: content");
    return ok ? 0 : 1;
}
