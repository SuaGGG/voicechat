#pragma once

#include "audio_interface.hpp"
#include <opus/opus.h>
#include <vector>
#include <memory>

namespace voicechat {

class OpusCodec : public IAudioCodec {
public:
    OpusCodec();
    ~OpusCodec() override;

    bool initialize(int sampleRate, int channels) override;
    std::vector<uint8_t> encode(const std::vector<float>& pcmData) override;
    std::vector<float> decode(const std::vector<uint8_t>& encodedData) override;

private:
    // Opus编码器和解码器
    OpusEncoder* encoder_;
    OpusDecoder* decoder_;
    
    int sampleRate_;
    int channels_;
    
    // Opus推荐的帧大小（以采样点为单位，20ms @ 48kHz = 960）
    static constexpr int FRAME_SIZE = 960;
    // 最大数据包大小
    static constexpr int MAX_PACKET_SIZE = 1275;
};

} // namespace voicechat 