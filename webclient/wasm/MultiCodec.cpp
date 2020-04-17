#include <emscripten/bind.h>

#include "../../src/MultiCodec.h"
#include "../../src/UrlParser.h"
#include <vector>

using namespace emscripten;
using namespace transmitter;


EMSCRIPTEN_BINDINGS() {
    class_<MultiCodec>("MultiCodec")
        .constructor()
        .function("encode", optional_override(
            [](MultiCodec& self, int _input, int count, int _packet) {
                float** input = reinterpret_cast<float**>(_input);
                unsigned char* packet = reinterpret_cast<unsigned char*>(_packet);
                return self.encode(input, count, packet);
            })
        ).function("decode", optional_override(
            [](MultiCodec& self, int _packet, int count) {
                unsigned char* packet = reinterpret_cast<unsigned char*>(_packet);
                self.decode(packet, count);
            })
        ).function("popSamples", optional_override(
            [](MultiCodec& self, int _output, int requestedSamples) {
                float** output = reinterpret_cast<float**>(_output);
                return self.popSamples(output, requestedSamples);
            })
        ).function("setEncoder", optional_override(
            [](MultiCodec& self, std::string name) {
                self.setEncoder(name.c_str());
            })
        ).function("setBitRate",
            &MultiCodec::setBitRate
        ).function("setSampleRate",
            &MultiCodec::setSampleRate
        ).function("setBufferSize",
            &MultiCodec::setBufferSize
        );

        class_<URLParser>("URLParser")
            .constructor<const std::string&>()
            .property("protocol", &URLParser::protocol)
            .property("host", &URLParser::host)
            .property("path", &URLParser::path)
            .property("query", &URLParser::query)
            .property("port", &URLParser::port)
            .property("ssl", &URLParser::ssl)
            .property("valid", &URLParser::valid)
            .property("nonDefaultPort", &URLParser::nonDefaultPort)
            .function("reconstruct", &URLParser::reconstruct);
}
