declare const Module;

import { environment } from 'src/environments/environment'

/**
 * A Float 2d buffer to hold audio allocated to be used with
 * emscripten and directly from javascript
 */
class FloatBuffer {
    // pointer to the array holding the float arrays
    private mainPtr = 0;
    // Pointers to the actual arrays
    private channelArrayPtrs: Uint32Array = undefined;
    // The float arrays themselves
    private sampleArrays: Float32Array[] = [];

    constructor(private length, private channels = 2) {
        if (!(<any>window).WASMLock) {
            console.error("Trying to contruct a WASM object on its own! Use the wasm loader service instead!");
            debugger;
            return;
        }
        // will return a pointer, addressing bytes
        this.mainPtr = Module._malloc(channels * 4);
        // will turn it into a array index for 4 byte elements
        const uint32index = this.mainPtr / 4;
        // space for pointers to the actual float arrays
        this.channelArrayPtrs = Module.HEAPU32.subarray(uint32index, uint32index + channels);

        for (let i = 0; i < channels; i++) {
            const ptr = Module._malloc(length * 4);
            this.channelArrayPtrs[i] = ptr;
            const index = ptr / 4;
            this.sampleArrays[i] = Module.HEAPF32.subarray(index, index + length);
        }
    }

    /**
     * Returns the pointer to the array of pointers which contain the
     * actual sample data for each channel
     */
    public getPtr(): number {
        return this.mainPtr;
    }

    /**
     * Copy in the audio provided
     * @param buf The data to copy in, each float array represents a channel
     */
    public set(buf: Float32Array[]) {
        for (let i = 0; i < Math.min(buf.length, this.channels); i++) {
            this.sampleArrays[i].set(buf[i]);
        }
    }

    /**
     * Copy out the audio
     * @param buf The arrays to store the result in
     */
    public get(buf: Float32Array[], count: number) {
        for (let i = 0; i < Math.min(buf.length, this.channels); i++) {
            buf[i].set(this.sampleArrays[i].subarray(0, count));
        }
    }

    /**
     * Will free all the allocated arrays
     * be sure to call this
     */
    public destroy() {
        for (let i = 0; i < this.channels; i++) {
            Module._free(this.channelArrayPtrs[i]);
        }
        Module._free(this.mainPtr);
    }

}

/**
 * A unint8 buffer allocated to be used with emscripten
 * and javascript
 */
class CharBuffer {
    private ptr: number = 0;
    private arr: Uint8Array = undefined;
    
    constructor(private length) {
        this.ptr = Module._malloc(length);
        this.arr = Module.HEAPU8.subarray(this.ptr, this.ptr + length);
    }

    public getPtr(): number {
        return this.ptr;
    }

    public get(buf: Uint8Array) {
        buf.set(this.arr);
    }

    public set(buf: Uint8Array) {
        this.arr.set(buf);
    }

    public destroy() {
        Module._free(this.ptr);
    }
}

/**
 * A class wrapping the multicodec from c++
 * also allocates the arrays to communicate with it
 */
export class MultiCodec {
    /**
     * The emscripten object
     */
    private emMultiCodec: any = undefined;

    /**
     * Buffers shared with javascript and the c++ code
     * to copy data in and out
     */
    private bufferDecode: FloatBuffer = undefined;
    private bufferEncode: FloatBuffer = undefined;
    private packetDecode: CharBuffer = undefined;
    private packetEncode: CharBuffer = undefined;

    constructor() {
        this.emMultiCodec = new Module.MultiCodec();

        this.bufferDecode = new FloatBuffer(1024, 2);
        this.bufferEncode = new FloatBuffer(1024, 2);

        /**
         * Roughly the MTU for a UDP packet
         */
        this.packetDecode = new CharBuffer(1500);
        this.packetEncode = new CharBuffer(1500);
        
        if (!environment.production) {
            if ((<any>window).DEBUGcodecs === undefined) {
                // Just to keep track of the codec object
                (<any>window).DEBUGcodecs = [];
            }
            (<any>window).DEBUGcodecs.push(this);
        }
    }

    /**
     * This will encode the samples in bufferEncode
     * The emcoded packet will be in packetEncode
     * @param sampleCount The number of audio samples (per channel) available in bufferEncode
     * @return The size of the encoded packet in bytes
     */
    public encode(sampleCount: number): number {
        return this.emMultiCodec.encode(this.bufferEncode.getPtr(), sampleCount, this.packetEncode.getPtr());
    }

    /**
     * Will decode the packet in packetDecode and add it onto the buffer
     * @param packetSize The size of the packet to decode
     */
    public decode(packet: Uint8Array) {
        this.packetDecode.set(packet);
        this.emMultiCodec.decode(this.packetDecode.getPtr(), packet.length);
    }

    /**
     * Will copy over the decoded samples to bufferDecode. If more samples were
     * requested than currently decoded, the rest of the buffer will be filled with silence
     * @param requestedSamples The samples to copy in bufferDecode
     */
    public popSamples(buf: Float32Array[], requestedSamples: number) {
        this.emMultiCodec.popSamples(this.bufferDecode.getPtr(), requestedSamples);
        this.bufferDecode.get(buf, requestedSamples);
    }

    public setBufferSize(size: number) {
        this.emMultiCodec.setBufferSize(size);
    }

    public setSampleRate(sr: number) {
        this.emMultiCodec.setSampleRate(sr);
    }

    /**
     * Acts as a destructor
     */
    public destroy() {
        this.bufferDecode.destroy();
        this.bufferEncode.destroy();
        this.packetDecode.destroy();
        this.packetEncode.destroy();
        this.emMultiCodec.delete();

        if (!environment.production) {
            for (let i in (<any>window).DEBUGcodecs) {
                if ((<any>window).DEBUGcodecs[i] === this) {
                    (<any>window).DEBUGcodecs[i] = null;
                }
            }
        }
    }
}