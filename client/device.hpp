#pragma once
#include <portaudio.h>
#include <atomic>

class SpeakeasyEngine;



class AudioDevice
{
public:
    bool Initialize(SpeakeasyEngine* engine);
    bool Start();
    bool State();
    void Stop();
    void Shutdown();
    bool Mute();
    bool Deaf();
    bool ToggleLoopback();
    AudioDevice();
    ~AudioDevice();
private:
    static int AudioCallback(
        const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData
    );
    int ProcessAudio(const void* input, void* output, unsigned long frameCount);

    uint8_t _encoded[4000];
    int16_t _decoded[DEFAULT_FRAMES_PER_BUFFER];

    std::atomic<bool> muted_ = false; //Determines whether the client is muted.
    std::atomic<bool> deafen_ = false; //Determines whether the client is deaf (cant hear, cant say.).
    std::atomic<bool> loopback_ = false; //Determines whether the client is hearing himself.


    SpeakeasyEngine* voiceEngine;
    PaStream* stream_;
    double sampleRate_;
    unsigned long framesPerBuffer_;
};