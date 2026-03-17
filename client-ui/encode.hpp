#pragma once
#include <opus.h>
#include <stdint.h>

class RCEncoder
{
public:
    RCEncoder() { }
    ~RCEncoder() { }
    bool Initialize(opus_int32 sampleRate, int channels);
    void Shutdown();
    int Encode(const int16_t* pcm, int frameSize, uint8_t* output, int maxBytes);
private:
    opus_int32 _sampleRate;
    int _channels;
    OpusEncoder* _encoder;
};

class RCDecoder
{
public:
    RCDecoder() { }
    ~RCDecoder() { }
    bool Initialize(opus_int32 sampleRate, int channels);
    void Shutdown();
    int Decode(const uint8_t* data, int len, int16_t* pcm, int frameSize);
private:
    opus_int32 _sampleRate;
    int _channels;
    OpusDecoder* _decoder;
};