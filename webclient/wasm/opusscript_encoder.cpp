#include <emscripten/bind.h>

#include "../../thirdparty/opus/include/opus.h"

using namespace emscripten;

int create_encoder(int sampling_rate, int channels, int application) {
    int err;
    OpusEncoder* encoder;
    encoder = opus_encoder_create(sampling_rate, channels, application, &err);
    if(err < 0) {
        return 0;
    } else {
        return (int)encoder;
    }
}

int create_decoder(int sampling_rate, int channels) {
    int err;
    OpusDecoder* decoder;
    decoder = opus_decoder_create(sampling_rate, channels, &err);
    if(err < 0) {
        return 0;
    } else {
        return (int)decoder;
    }
}

int encoder_ctl(int p_encoder, int ctl, int arg) {
    OpusEncoder *encoder = reinterpret_cast<OpusEncoder*>(p_encoder);
    return opus_encoder_ctl(encoder, ctl, arg);
}

int decoder_ctl(int p_decoder, int ctl, int arg) {
    OpusDecoder *decoder = reinterpret_cast<OpusDecoder*>(p_decoder);
    return opus_decoder_ctl(decoder, ctl, arg);
}

int encode_float(int p_encoder, int p_buf, int buf_len, int p_packet, int packet_len) {
    float *buf = reinterpret_cast<float*>(p_buf);
    unsigned char *packet = reinterpret_cast<unsigned char*>(p_packet);
    OpusEncoder *encoder = reinterpret_cast<OpusEncoder*>(p_encoder);
    return opus_encode_float(encoder, buf, buf_len, packet, packet_len);
}

int decode_float(int p_decoder, int p_packet, int packet_len, int p_buf, int buf_len) {
    unsigned char *packet = reinterpret_cast<unsigned char*>(p_packet);
    float *buf = reinterpret_cast<float*>(p_buf);
    OpusDecoder *decoder = reinterpret_cast<OpusDecoder*>(p_decoder);
    return opus_decode_float(decoder, packet, packet_len, buf, buf_len, 0);
}

void destroy_encoder(int p_encoder) {
    OpusEncoder *encoder = reinterpret_cast<OpusEncoder*>(p_encoder);
    opus_encoder_destroy(encoder);
}

void destroy_decoder(int p_decoder) {
    OpusDecoder *decoder = reinterpret_cast<OpusDecoder*>(p_decoder);
    opus_decoder_destroy(decoder);
}

EMSCRIPTEN_BINDINGS() {
    function("create_encoder", &create_encoder);
    function("create_decoder", &create_decoder);
    function("encoder_ctl", &encoder_ctl);
    function("decoder_ctl", &decoder_ctl);
    function("encode_float", &encode_float);
    function("decode_float", &decode_float);
    function("destroy_decoder", &destroy_decoder);
    function("destroy_encoder", &destroy_encoder);
}
