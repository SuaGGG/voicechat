#include "opus_codec.hpp"
#include <stdexcept>
#include <cstring>

namespace voicechat {

OpusCodec::OpusCodec()
    : encoder_(nullptr)
    , decoder_(nullptr)
    , sampleRate_(48000)  // Opus推荐采样率
    , channels_(2)
{
}

OpusCodec::~OpusCodec() {
    if (encoder_) {
        opus_encoder_destroy(encoder_);
    }
    if (decoder_) {
        opus_decoder_destroy(decoder_);
    }
}

bool OpusCodec::initialize(int sampleRate, int channels) {
    sampleRate_ = sampleRate;
    channels_ = channels;
    
    int error;
    
    // 创建编码器
    encoder_ = opus_encoder_create(
        sampleRate_,
        channels_,
        OPUS_APPLICATION_VOIP,  // 针对VoIP优化
        &error
    );
    
    if (error != OPUS_OK || !encoder_) {
        return false;
    }
    
    // 设置编码器参数
    opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(64000));  // 64kbps
    opus_encoder_ctl(encoder_, OPUS_SET_COMPLEXITY(8));   // 复杂度 (0-10)
    opus_encoder_ctl(encoder_, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));  // 针对语音优化
    
    // 创建解码器
    decoder_ = opus_decoder_create(
        sampleRate_,
        channels_,
        &error
    );
    
    if (error != OPUS_OK || !decoder_) {
        if (encoder_) {
            opus_encoder_destroy(encoder_);
            encoder_ = nullptr;
        }
        return false;
    }
    
    return true;
}

std::vector<uint8_t> OpusCodec::encode(const std::vector<float>& pcmData) {
    if (!encoder_ || pcmData.empty()) {
        return {};
    }
    
    std::vector<uint8_t> encodedData(MAX_PACKET_SIZE);
    
    // 确保输入数据大小正确
    if (pcmData.size() != static_cast<size_t>(FRAME_SIZE * channels_)) {
        return {};
    }
    
    // 编码
    opus_int32 encodedBytes = opus_encode_float(
        encoder_,
        pcmData.data(),
        FRAME_SIZE,
        encodedData.data(),
        MAX_PACKET_SIZE
    );
    
    if (encodedBytes < 0) {
        return {};
    }
    
    encodedData.resize(encodedBytes);
    return encodedData;
}

std::vector<float> OpusCodec::decode(const std::vector<uint8_t>& encodedData) {
    if (!decoder_ || encodedData.empty()) {
        return {};
    }
    
    std::vector<float> pcmData(FRAME_SIZE * channels_);
    
    // 解码
    int decodedSamples = opus_decode_float(
        decoder_,
        encodedData.data(),
        encodedData.size(),
        pcmData.data(),
        FRAME_SIZE,
        0  // 不使用FEC
    );
    
    if (decodedSamples < 0) {
        return {};
    }
    
    pcmData.resize(decodedSamples * channels_);
    return pcmData;
}

} // namespace voicechat 