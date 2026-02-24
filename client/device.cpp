#include "device.hpp"
#include "voice.hpp"
#include <algorithm>

AudioDevice::AudioDevice()
{
    sampleRate_ = DEFAULT_SAMPLE_RATE;
    framesPerBuffer_ = DEFAULT_FRAMES_PER_BUFFER;
    stream_ = nullptr;
}
AudioDevice::~AudioDevice()
{

}

bool AudioDevice::Initialize(SpeakeasyEngine* engine)
{
    voiceEngine = engine;
    Pa_Initialize();
    PaError err;
    

    err = Pa_OpenDefaultStream(&stream_, 1, 1, paInt16, sampleRate_, framesPerBuffer_, &AudioDevice::AudioCallback, this);

    if (err != paNoError)
        return false;
    else
        return true;
}

bool AudioDevice::Start()
{
    PaError err = Pa_StartStream(stream_);
    if (err != paNoError)
        return false;
    else
        return true;
}

bool AudioDevice::State()
{
    return (Pa_IsStreamActive(stream_));
}

void AudioDevice::Stop()
{
    Pa_StopStream(stream_);
}

void AudioDevice::Shutdown()
{
    Pa_CloseStream(stream_);
    Pa_Terminate();
}

bool AudioDevice::Mute()
{
    muted_ = !muted_;
    return muted_;
}

bool AudioDevice::Deaf()
{
    deafen_ = !deafen_;
    return deafen_;
}

bool AudioDevice::ToggleLoopback()
{
    loopback_ = !loopback_;
    return loopback_;
}



int AudioDevice::ProcessAudio(const void* input, void* output, unsigned long frameCount)
{
    int16_t* out = static_cast<int16_t*>(output);

    if (!deafen_)
    {
        voiceEngine->MixOutput(out);
    }
    else 
    {
        memset(output, 0, frameCount * sizeof(int16_t));
    }

    if (input && !deafen_)
    {
        if (!muted_)
        {
            voiceEngine->OnAudioInput((int16_t*)input, frameCount);

            if(loopback_)
            {
                const int16_t* in = static_cast<const int16_t*>(input);
                for (int i = 0; i < DEFAULT_FRAMES_PER_BUFFER; i++)
                {
                    int32_t mix = out[i] + in[i];
                    mix = std::clamp(mix, -32768, 32767);
                    out[i] = static_cast<int16_t>(mix);
                }
            }       
        }
    }
    return 0;
}

int AudioDevice::AudioCallback(
    const void* input,
    void* output,
    unsigned long frameCount, 
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
)
{
    AudioDevice* myself = static_cast<AudioDevice*>(userData);
    myself->ProcessAudio(input, output, frameCount);
    return paContinue;
}