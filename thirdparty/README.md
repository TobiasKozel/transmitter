# Thirdparty libs and code

## libopus
https://github.com/xiph/opus

The well known opus codec is used to get high quality low latency audio from a to b.
Pre compiled versions should be in the ./static folder.
The wasm version has to be compiled using the makefile in the webclient/wasm directory.

## httplib
https://github.com/yhirose/cpp-httplib

To communicate with the API from the C++ plugin.

## libssl
https://github.com/openssl/openssl

To get https requests with httplib we'll need this. Also precompiled in the ./static folder.
Noone should suffer through the process of compiling this.

## WDL resample
To get realtime resampling I stripped the resampler included in cockos WDL from its dependecies and threw out the
sinc resampler so it can be used for the wasm build too.

## WDL circlebuf
The circlebuffer from WDL was also stipped its deps.

## json
https://github.com/nlohmann/json

Used to parse and encode json in the plugin.

## netlib
https://github.com/univrsal/netlib

SDL_net stripped from SDL dependencies.

## Threadpool
https://github.com/vit-vit/CTPL

Not used for now, maybe at some pointer the codecs can run in another thread