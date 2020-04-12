#include <emscripten/bind.h>

#include "../../src/MultiCodec.h"

using namespace emscripten;
using namespace transmitter;


EMSCRIPTEN_BINDINGS() {
    class_<MultiCodec>("MultiCodec")
        .constructor()
        .function("setBufferSize", &MultiCodec::setBufferSize)
        .function("encode", &MultiCodec::encode, optional_override(
            [](MultiCodec& self, const std::vector<std::vector<float>> samples, int count, std::vector<uint8_t>& data){
                float** arr[] = { &(samples[0][0]), &(samples[1][0]) };
                data.resize(1500);
                return self.encode(arr, count, &data[0])
                return bla(s.c_str());
            }
        ));
        .function("popSamples", &MultiCodec::popSamples, allow_raw_pointers())
        .function("decode", &MultiCodec::decode, allow_raw_pointers());
}
