#include "audio_decode.h"

#if defined(__APPLE__)

#include <AudioToolbox/AudioToolbox.h>

#include <cstring>

bool decodeAudioFile(const char* path, std::vector<float>& samples, int& channels, int& rate) {
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(nullptr, reinterpret_cast<const UInt8*>(path),
                                                           static_cast<CFIndex>(std::strlen(path)), false);
    if (!url) return false;
    ExtAudioFileRef file = nullptr;
    const OSStatus opened = ExtAudioFileOpenURL(url, &file);
    CFRelease(url);
    if (opened != noErr) return false;

    AudioStreamBasicDescription source{};
    UInt32 size = sizeof(source);
    bool ok = ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileDataFormat, &size, &source) == noErr &&
              source.mChannelsPerFrame > 0 && source.mSampleRate > 0;
    const UInt32 frameChannels = source.mChannelsPerFrame;
    if (ok) {
        AudioStreamBasicDescription client{};
        client.mSampleRate = source.mSampleRate;
        client.mFormatID = kAudioFormatLinearPCM;
        client.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        client.mBitsPerChannel = 32;
        client.mChannelsPerFrame = frameChannels;
        client.mFramesPerPacket = 1;
        client.mBytesPerFrame = 4 * frameChannels;
        client.mBytesPerPacket = client.mBytesPerFrame;
        ok = ExtAudioFileSetProperty(file, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client) == noErr;
    }
    constexpr UInt32 kChunkFrames = 4096;
    std::vector<float> chunk(kChunkFrames * frameChannels);
    while (ok) {
        UInt32 frames = kChunkFrames;
        AudioBufferList list{};
        list.mNumberBuffers = 1;
        list.mBuffers[0].mNumberChannels = frameChannels;
        list.mBuffers[0].mDataByteSize = static_cast<UInt32>(chunk.size() * sizeof(float));
        list.mBuffers[0].mData = chunk.data();
        if (ExtAudioFileRead(file, &frames, &list) != noErr) {
            ok = false;
            break;
        }
        if (frames == 0) break;
        samples.insert(samples.end(), chunk.begin(), chunk.begin() + frames * frameChannels);
    }
    ExtAudioFileDispose(file);
    channels = static_cast<int>(frameChannels);
    rate = static_cast<int>(source.mSampleRate);
    return ok && !samples.empty();
}

#elif defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

#include <string>

using Microsoft::WRL::ComPtr;

namespace {

bool readAll(const wchar_t* path, std::vector<float>& samples, int& channels, int& rate) {
    const DWORD stream = static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
    ComPtr<IMFSourceReader> reader;
    if (FAILED(MFCreateSourceReaderFromURL(path, nullptr, &reader))) return false;
    ComPtr<IMFMediaType> wanted;
    if (FAILED(MFCreateMediaType(&wanted))) return false;
    wanted->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    wanted->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
    if (FAILED(reader->SetCurrentMediaType(stream, nullptr, wanted.Get()))) return false;
    ComPtr<IMFMediaType> actual;
    if (FAILED(reader->GetCurrentMediaType(stream, &actual))) return false;
    UINT32 frameChannels = 0, sampleRate = 0;
    actual->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &frameChannels);
    actual->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
    if (frameChannels == 0 || sampleRate == 0) return false;
    for (;;) {
        DWORD flags = 0;
        ComPtr<IMFSample> sample;
        if (FAILED(reader->ReadSample(stream, 0, nullptr, &flags, nullptr, &sample))) return false;
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        if (!sample) continue;
        ComPtr<IMFMediaBuffer> buffer;
        if (FAILED(sample->ConvertToContiguousBuffer(&buffer))) return false;
        BYTE* data = nullptr;
        DWORD length = 0;
        if (FAILED(buffer->Lock(&data, nullptr, &length))) return false;
        const float* values = reinterpret_cast<const float*>(data);
        samples.insert(samples.end(), values, values + length / sizeof(float));
        buffer->Unlock();
    }
    channels = static_cast<int>(frameChannels);
    rate = static_cast<int>(sampleRate);
    return !samples.empty();
}

}

bool decodeAudioFile(const char* path, std::vector<float>& samples, int& channels, int& rate) {
    const int wideLength = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
    if (wideLength <= 0) return false;
    std::wstring widePath(static_cast<size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path, -1, widePath.data(), wideLength);
    const HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool ok = false;
    if (SUCCEEDED(MFStartup(MF_VERSION))) {
        ok = readAll(widePath.c_str(), samples, channels, rate);
        MFShutdown();
    }
    if (SUCCEEDED(com)) CoUninitialize();
    return ok;
}

#else

#include <cstdio>
#include <string>

bool decodeAudioFile(const char* path, std::vector<float>& samples, int& channels, int& rate) {
    std::string quoted = "'";
    for (const char* c = path; *c; ++c) quoted += *c == '\'' ? std::string("'\\''") : std::string(1, *c);
    quoted += "'";
    const std::string command = "ffmpeg -v error -nostdin -i " + quoted + " -f f32le -ac 2 -ar 48000 - 2>/dev/null";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;
    float chunk[4096];
    for (size_t read; (read = std::fread(chunk, sizeof(float), 4096, pipe)) > 0;) samples.insert(samples.end(), chunk, chunk + read);
    const int status = pclose(pipe);
    channels = 2;
    rate = 48000;
    return status == 0 && !samples.empty();
}

#endif
