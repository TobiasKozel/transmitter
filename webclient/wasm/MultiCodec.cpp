#include <emscripten/bind.h>

#include "../../src/MultiCodec.h"

using namespace emscripten;
using namespace transmitter;


EMSCRIPTEN_BINDINGS() {
    class_<MultiCodec>("MultiCodec")
        .constructor()
        .function("setBufferSize", &MultiCodec::setBufferSize)
        .function("encode", &MultiCodec::encode)
        .function("popSamples", &MultiCodec::popSamples)
        .function("decode", &MultiCodec::decode);
}
