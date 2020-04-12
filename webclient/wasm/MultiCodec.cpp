#include <emscripten/bind.h>

#include "../../src/MultiCodec.h"

using namespace emscripten;
using namespace transmitter;


EMSCRIPTEN_BINDINGS() {
    class_<MultiCodec>("MultiCodec")
        .constructor()
        .function("setBufferSize", &MultiCodec::setBufferSize)
        .function("encode", &MultiCodec::encode, allow_raw_pointers())
        .function("popSamples", &MultiCodec::popSamples, allow_raw_pointers())
        .function("decode", &MultiCodec::decode, allow_raw_pointers());
}
