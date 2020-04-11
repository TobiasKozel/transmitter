OPUS_NATIVE_DIR=../../thirdparty/opus/
OUT_DIR=../src/assets/

EMCC_OPTS=-fno-stack-protector -Wall -O3 --llvm-lto 3 -s ALLOW_MEMORY_GROWTH=1 --memory-init-file 0 -s NO_FILESYSTEM=1 -s EXPORTED_RUNTIME_METHODS="['setValue', 'getValue']" -s EXPORTED_FUNCTIONS="['_malloc', '_opus_strerror']" -s MODULARIZE=0 -s BUILD_AS_WORKER=0
EMCC_OPTSD=-fno-stack-protector -Wall -g -O0 --llvm-lto 0 -s ALLOW_MEMORY_GROWTH=1 --memory-init-file 0 -s NO_FILESYSTEM=1 -s EXPORTED_RUNTIME_METHODS="['setValue', 'getValue']" -s EXPORTED_FUNCTIONS="['_malloc', '_opus_strerror']" -s MODULARIZE=0 -s BUILD_AS_WORKER=0

EMCC_NASM_OPTS=-s WASM=0
EMCC_WASM_OPTS=-s WASM=1 -s WASM_ASYNC_COMPILATION=1

all: init compile
clean:
	rm ${OUT_DIR}*
	cd $(OPUS_NATIVE_DIR); \
	make clean
autogen:
	cd $(OPUS_NATIVE_DIR); \
	./autogen.sh
configure:
	cd $(OPUS_NATIVE_DIR); \
	emconfigure ./configure --disable-extra-programs --disable-doc --disable-intrinsics
bind:
	cd $(OPUS_NATIVE_DIR); \
	emmake make CFLAGS="-fno-stack-protector";
init: autogen configure bind
compile:
	em++ ${EMCC_OPTS} ${EMCC_NASM_OPTS} --bind -o ${OUT_DIR}opusscript_native_nasm.js opusscript_encoder.cpp ${OPUS_NATIVE_DIR}/.libs/libopus.a; \
	em++ ${EMCC_OPTS} ${EMCC_WASM_OPTS} --bind -o ${OUT_DIR}opusscript_native_wasm.js opusscript_encoder.cpp ${OPUS_NATIVE_DIR}/.libs/libopus.a; \
	cp -f $(OPUS_NATIVE_DIR)/COPYING ${OUT_DIR}COPYING.libopus;
debug:
	em++ ${EMCC_OPTSD} ${EMCC_WASM_OPTS} --bind -o ${OUT_DIR}opusscript_native_wasm.js opusscript_encoder.cpp ${OPUS_NATIVE_DIR}/.libs/libopus.a; \