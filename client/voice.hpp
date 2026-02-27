#pragma once
#include "RakPeerInterface.h"

#include <stdint.h>
#include <map>
#include <atomic>
#include <queue>
#include <thread>
#include "device.hpp"
#include "encode.hpp"
#include <array>
#include <mutex>


constexpr unsigned long DEFAULT_SAMPLE_RATE = 48000;
constexpr uint32_t DEFAULT_FRAMES_PER_BUFFER  = 960;

struct VoiceBuffer
{
    int16_t data_[DEFAULT_FRAMES_PER_BUFFER];
};

struct RemoteVoice
{
    RCDecoder decoder;
    std::queue<std::array<int16_t, DEFAULT_FRAMES_PER_BUFFER>> jitter;
    bool started = false;
    float speakerVolume = 1.0f;
};

using namespace RakNet;

enum EngineStates
{
    ENGINE_NONE = 0x00,
    ENGINE_ERROR,
    ENGINE_OK
};

class SpeakeasyEngine
{
public:
    SpeakeasyEngine(RakPeerInterface* p);
    ~SpeakeasyEngine();
    void Shutdown();
    void OnAudioInput(int16_t* pcm, unsigned long frameCount);
    void OnNetworkVoice(uint16_t id, uint8_t* data, uint16_t size);
    void spkThread();
    AudioDevice* GetDevice();
    void MixOutput(int16_t* out);
    void SetMasterVolume(float value);
    float GetMasterVolume() { return masterVolume; }
    uint8_t GetState() { return engineState; }
private:
    
    std::queue<VoiceBuffer> captureQueue;
    std::mutex captureMutex;

    std::queue<VoiceBuffer> playbackQueue;
    std::mutex voicesMutex;

    std::atomic<bool> running_{false};

    std::thread spkThr;

    RakPeerInterface* peer;

    AudioDevice device_;
    RCEncoder encoder_;

    uint8_t engineState = ENGINE_NONE;

    std::map<uint16_t, RemoteVoice> RemoteSpeakers;

    float masterVolume = 0.5f;
};


