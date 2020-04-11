### Simple JS Bindings for libopus

Very simplified version of https://github.com/abalabahaha/opusscript

Compiled using emscripten

Initialize the submoules

Simply set up the emsdk environment and run `make all`

If you get this error

`wasm-ld: error: ./opus-native/.libs/libopus.a(opus_decoder_0a377ef1.o): undefined symbol: __stack_chk_guard`

Go into the opus-native directory and clean it up using `make clean`

Go back again and run `make all CFLAGS="-fno-stack-protector"` instead

Put the files in build into the asset folder of angular