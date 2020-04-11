OPUS_NATIVE_DIR=./../thirdparty/opus/

EMCC_OPTS=-Wall -O3 --llvm-lto 3 -s ALLOW_MEMORY_GROWTH=1 --memory-init-file 0 -s NO_FILESYSTEM=1 -s EXPORTED_RUNTIME_METHODS="['setValue', 'getValue']" -s EXPORTED_FUNCTIONS="['_malloc', '_opus_strerror']" -s MODULARIZE=0 -s BUILD_AS_WORKER=0

EMCC_NASM_OPTS=-s WASM=0
EMCC_WASM_OPTS=-s WASM=1 -s WASM_ASYNC_COMPILATION=1

all: init compile
autogen:
	cd $(OPUS_NATIVE_DIR); \
	make clean; \
	./autogen.sh
configure:
	cd OPUS_NATIVE_DIR; \
	emconfigure ./configure --disable-extra-programs --disable-doc --disable-intrinsics
bind:
	cd $(OPUS_NATIVE_DIR); \
	emmake make;
init: autogen configure bind
compile:
	em++ ${EMCC_OPTS} ${EMCC_NASM_OPTS} --bind -o ../src/assets/opusscript_native_nasm.js opusscript_encoder.cpp ${OPUS_NATIVE_DIR}/.libs/libopus.a; \
	em++ ${EMCC_OPTS} ${EMCC_WASM_OPTS} --bind -o ../src/assets/opusscript_native_wasm.js opusscript_encoder.cpp ${OPUS_NATIVE_DIR}/.libs/libopus.a; \
	cp -f $(OPUS_NATIVE_DIR)/COPYING ../src/assets/COPYING.libopus;