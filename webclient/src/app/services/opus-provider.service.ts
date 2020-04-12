import { Injectable, ModuleWithComponentFactories } from '@angular/core';
import { environment } from 'src/environments/environment';
declare function importScripts(...urls: string[]): void;
declare const self: { encoder: OpusProviderService };

declare const Module;

@Injectable({
	providedIn: 'root'
})
/**
 * This Loads the emscripten compiled opus reference encoder/decoder and provides a interface to it.
 */
export class OpusProviderService {
	/**
	 * The most important opus enums for readability
 	*/
	OPUS_APPLICATION_VOIP = 2048;
	OPUS_APPLICATION_AUDIO = 2049;
	OPUS_APPLICATION_RESTRICTED_LOWDELAY = 2051;
	OPUS_SET_APPLICATION_REQUEST = 4000;
	OPUS_SET_BITRATE_REQUEST = 4002;
	OPUS_SET_COMPLEXITY_REQUEST = 4010;
	OPUS_SET_PACKET_LOSS_PERC_REQUEST = 4014;
	OPUS_SET_DTX_REQUEST = 4016;
	OPUS_SET_LSB_DEPTH_REQUEST = 4036;
	OPUS_SET_PHASE_INVERSION_DISABLED_REQUEST = 4046;
	OPUS_OK = 0;
	OPUS_BAD_ARG = -1;
	OPUS_BUFFER_TOO_SMALL = -2;
	OPUS_INTERNAL_ERROR = -3;
	OPUS_INVALID_PACKET = -4;
	OPUS_UNIMPLEMENTED = -5;
	OPUS_INVALID_STATE = -6;
	OPUS_ALLOC_FAIL = -7;
	OPUS_FRAMESIZES = [120, 240, 480, 960, 1920, 2880];

	public ready = false;

	private encoderHandle = 0;
	private decoderHandle = 0;


	/**
	 * Encoder settings, used for initialisation and keep track of changes
	 */
	public sampleRate = 48000;
	public channels = 2;
	public bitrate = 96000;
	public complexity = 10;
	public application = this.OPUS_APPLICATION_RESTRICTED_LOWDELAY;
	public packetLossPerc = 0;
	public lsbDepth = 24;
	public dtx = 1;
	public noPhaseInversion = 1;

	/**
	 * The audio buffers for encoding and decoding
	 */
	private encBufferPtr = 0;
	private encBuffer: Float32Array;
	private encBufferLength = 2880 * this.channels;
	private decBufferPtr = 0;
	private decBuffer: Float32Array;
	private decBufferLength = 2880 * this.channels;

	/**
	 * The packets used to to store encoded data
	 */
	private encPacketsPtr = 0;
	private encPackets: Uint8Array;
	private encPacketsLength = 4000;
	private decPacketsPtr = 0;
	private decPackets: Uint8Array;
	private decPacketsLength = 4000;

	private initialized = false;
	private initializing = false;

	private isWorker = typeof(window) === "undefined";

	constructor() {
		if (!this.isWorker && !environment.production) {
			(window as any).DEBUGOpusProvider = this;
		}
	}

	public init() {
		if (this.initializing) { return; }
		this.initializing = true;
		const onload = () => {
			console.log("opus loaded!");
			this.allocateAudioBuffers();
			this.allocateDataPackets();
			this.createEncoder();
			this.createDecoder();
			this.ready = true;
		};
		if (this.isWorker) {
			importScripts("http://localhost:4200/assets/opusscript_native_nasm.js");
			onload();
		} else {
			/** Load it via a script tag */
			const script = document.createElement("script");
			const url = window.location.href;
			if (url.search("nowasm") === -1) {
				console.log("Using wasm");
				script.onload = () => {
					Module.onRuntimeInitialized = onload;
				};
				script.src = "/assets/opusscript_native_wasm.js";
			} else {
				/** Allow using the non wasm version if the url contains "nowasm" */
				script.onload = onload;
				console.log("Not using wasm");
				script.src = "/assets/opusscript_native_nasm.js";
			}
			console.log("Loading opus");
			document.getElementsByTagName("head")[0].appendChild(script);
		}
	}

	private createEncoder() {
		this.encoderHandle =
			(Module as any).create_encoder(
				this.sampleRate, this.channels, this.application
			);
		if (!this.encoderHandle) {
			console.error("Couldn't create opus encoder!");
			return;
		}
	}

	private createDecoder() {
		this.decoderHandle =
			(Module as any).create_decoder(this.sampleRate, this.channels);
		if (!this.decoderHandle) {
			console.error("Couldn't create opus decoder!");
			return;
		}
	}

	private allocateAudioBuffers() {
		/** 
		 * Malloc is exposed from emscripten and used to allocate the buffers
		 * Using 32Bit float so buffer langth * 4 Bytes
		 */
		this.encBufferPtr = Module._malloc(this.encBufferLength * 4);
		/**
		 * Get the allocated memory as a float32 sub array for convenient use
		 * This needs actual indecies (eg. i + 1 = stride of 4 Bytes) so the pointer address is devided by 4 again
		 */
		this.encBuffer = Module.HEAPF32.subarray(
			this.encBufferPtr / 4, this.encBufferPtr / 4 + this.encBufferLength
		);
		this.decBufferPtr = Module._malloc(this.decBufferLength * 4);
		this.decBuffer = Module.HEAPF32.subarray(
			this.decBufferPtr / 4, this.decBufferPtr / 4 + this.decBufferLength
		);
	}

	private allocateDataPackets() {
		/**
		 * The encoded packets simply use chars, so no shady pointer math
		 */
		this.encPacketsPtr = Module._malloc(this.encPacketsLength);
		this.encPackets = Module.HEAPU8.subarray(
			this.encPacketsPtr, this.encPacketsPtr + this.encPacketsLength
		);
		this.decPacketsPtr = Module._malloc(this.decPacketsLength);
		this.decPackets = Module.HEAPU8.subarray(
			this.decPacketsPtr, this.decPacketsPtr + this.decPacketsLength
		);
	}

	public encodeFloat(buffer: Float32Array): Uint8Array {
		if (!this.initialized) {
			/** Doing the init here prevented some underruns */
			this.initialized = true;
			this.initEncoderSettings();
		}
		this.encBuffer.set(buffer);
		const outLength = (Module as any).encode_float(
			this.encoderHandle,
			this.encBufferPtr,
			buffer.length / this.channels,
			this.encPacketsPtr,
			this.encPacketsLength
		);
		if (outLength < 0) {
			console.error("Couldn't encode frame");
			console.error(outLength);
			return null;
		}
		return this.encPackets.subarray(0, outLength);
	}

	public decodeFloat(packets: Uint8Array): Float32Array {
		this.decPackets.set(packets);
		const outLength = (Module as any).decode_float(
			this.decoderHandle,
			this.decPacketsPtr,
			packets.length,
			this.decBufferPtr,
			this.decBufferLength
		);
		return this.decBuffer.subarray(0, outLength * this.channels);
	}

	/**
	 * Convenience function to access the encoder controls from the public methods below
	 */
	private encoderControl(ctlId: number, property: string, value: number): void {
		this[property] = value;
		const err = (Module as any).encoder_ctl(
			this.encoderHandle, ctlId, value
		);
		if (err) {
			console.error("Couldn't set the target " + property + " to " + value + "!");
		}
	}

	public initEncoderSettings() {
		this.setBitrate();
		this.setComplexity();
		this.setApplication();
		this.setPacketLossPerc();
		this.setLsbDepth();
		this.setNoPhaseInversion();
		this.setDtx();
	}

	public setBitrate(bitrate = this.bitrate) {
		this.encoderControl(
			this.OPUS_SET_BITRATE_REQUEST, "bitrate", bitrate
		);
	}

	public setComplexity(complexity = this.complexity) {
		this.encoderControl(
			this.OPUS_SET_COMPLEXITY_REQUEST, "complexity", complexity
		);
	}

	public setApplication(application = this.application) {
		this.encoderControl(
			this.OPUS_SET_APPLICATION_REQUEST, "application", application
		);
	}

	public setPacketLossPerc(packetLossPerc = this.packetLossPerc) {
		this.encoderControl(
			this.OPUS_SET_PACKET_LOSS_PERC_REQUEST, "packetLossPerc", packetLossPerc
		);
	}

	public setLsbDepth(lsbDepth = this.lsbDepth) {
		this.encoderControl(
			this.OPUS_SET_LSB_DEPTH_REQUEST, "lsbDepth", lsbDepth
		);
	}

	public setNoPhaseInversion(noPhaseInversion = this.noPhaseInversion) {
		this.encoderControl(
			this.OPUS_SET_PHASE_INVERSION_DISABLED_REQUEST, "noPhaseInversion", noPhaseInversion
		);
	}

	public setDtx(dtx = this.dtx) {
		this.encoderControl(
			this.OPUS_SET_DTX_REQUEST, "dtx", dtx
		);
	}

	private destroyEncoder() {
		/**
		 * Not used, should also release the memory for all the buffers
		 */
		if (this.encoderHandle) {
			(Module as any).destroy_encoder(this.encoderHandle);
		} else {
			console.error("No encoder to destroy!");
		}
		if (this.encoderHandle) {
			debugger;
		}
	}

	private destroyDecoder() {
		if (this.encoderHandle) {
			(Module as any).destroy_decoder(this.decoderHandle);
		} else {
			console.error("No decoder to destroy!");
		}
	}
}


/**
 * Functions to use this service from a worker
 */
export const opusWorkerInit = () => {
	self.encoder = new OpusProviderService();
	self.encoder.init();
};

export const opusWorkerInitOnessage = (message: any, callback: (data?: any) => void): void => {
	if (message.task) {
		if (self.encoder[message.task].constructor === Function) {
			callback(self.encoder[message.task](message.data));
		}
	} else {
		callback();
	}
};
