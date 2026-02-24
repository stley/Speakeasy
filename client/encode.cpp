#include "encode.hpp"



bool RCEncoder::Initialize(opus_int32 sampleRate, int channels)
{
    _sampleRate = sampleRate;
    _channels = channels;
    int error;
    _encoder = opus_encoder_create(sampleRate, _channels, OPUS_APPLICATION_VOIP, &error);

    if (error != OPUS_OK || !_encoder)
    {
        return false;
    }
    opus_encoder_ctl(_encoder, OPUS_SET_BITRATE(OPUS_AUTO));
    opus_encoder_ctl(_encoder, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(_encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    return true;
}

void RCEncoder::Shutdown()
{
    if (_encoder)
    {
        opus_encoder_destroy(_encoder);
        _encoder = nullptr;
    }
}

int RCEncoder::Encode(const int16_t* pcm, int frameSize, uint8_t* output, int maxBytes)
{
    int ret = 0;
    if (_encoder)
    {
        ret = opus_encode(_encoder, pcm, frameSize, reinterpret_cast<unsigned char*>(output), maxBytes);
    }
    return ret;
}

bool RCDecoder::Initialize(opus_int32 sampleRate, int channels)
{
    _sampleRate = sampleRate;
    _channels = channels;
    int error;
    _decoder = opus_decoder_create(sampleRate, _channels, &error);

    if (error != OPUS_OK || !_decoder)
    {
        return false;
    }
    return true;
}

void RCDecoder::Shutdown()
{
    if (_decoder)
    {
        opus_decoder_destroy(_decoder);
    }
}

int RCDecoder::Decode(const uint8_t* data, int len, int16_t* pcm, int frameSize)
{
    int ret = 0;
    if (_decoder)
    {
        ret = opus_decode(_decoder, reinterpret_cast<const unsigned char*>(data), len, pcm, frameSize, 0);
    }
    return ret;
}