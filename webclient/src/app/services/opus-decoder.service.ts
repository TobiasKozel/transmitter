import { Injectable } from '@angular/core';
import { environment } from 'src/environments/environment.prod';
import { StereoBuffer } from '../classes/stereoBuffer';
import { WssBaseService } from './wss-base.service';
import { OpusProviderService, opusWorkerInit, opusWorkerInitOnessage } from './opus-provider.service';
import { WrappedWorker } from '../classes/wrappedWorker';

@Injectable({
	providedIn: 'root'
})
export class OpusDecoderService extends WssBaseService {
	useWorker = false;
	decoderWorker: WrappedWorker;
	callback: (interleaved: Float32Array) => void;
	lastPacketNumber = 0;
	constructor(
		private opus: OpusProviderService
	) {
		super();
		if (!environment.production) {
			(window as any).DEBUGDecOpus = this;
		}
		// this.wssAddress = environment.listenerWss; //  TODO new single socket system
		if (window.location.href.search("decWorker") !== -1) {
			this.useWorker = true;
			this.decoderWorker = new WrappedWorker(
				opusWorkerInitOnessage, opusWorkerInit, [OpusProviderService]
			);
		} else {
			opus.init();
		}
	}

	onMessage(data: ArrayBuffer) {
		if (this.callback && this.opus.ready) {
			const packet = new Uint8Array(data);
			/** Some code to evaluate the packet number and slice it off for the decoder */
			// const uint8Data = packet.subarray(0, packet.length - 4);
			// const int32 = new Uint32Array(packet.buffer.slice(packet.length - 4, packet.length))[0];
			// if (this.lastPacketNumber + 1 !== int32) {
			// 	console.log("Packed number mismatch! Expected " + (this.lastPacketNumber + 1) + " got " + int32);
			// }
			// this.lastPacketNumber = int32;

			if (this.useWorker) {
				this.decoderWorker.run({
					task: "decodeFloat",
					data: packet
				}).then((data: Float32Array) => {
					this.callback(data);
				});
			} else {
				const interleavedStereo = this.opus.decodeFloat(packet);
				this.callback(interleavedStereo);
			}
		}
	}

	debugMessage(data: Uint8Array, expectedFrameSize: number) {
		const buData = new Uint8Array(data).subarray(0, data.length - 4);
		const packetNumberUint = new Uint8Array(data).subarray(data.length - 4, data.length);
		const int32 = new Uint32Array(packetNumberUint.buffer.slice(0, 4));
		console.log(int32);

		// setTimeout(() => {
		if (this.callback && this.opus.ready) {
			const interleavedStereo = this.opus.decodeFloat(buData);
			if (interleavedStereo.length / 2 !== expectedFrameSize) {
				console.log("Framesize mismatch, is " + interleavedStereo.length / 2 + " should be " + expectedFrameSize);
			}
			this.callback(interleavedStereo);
		}
		// }, 1000);
	}

}
