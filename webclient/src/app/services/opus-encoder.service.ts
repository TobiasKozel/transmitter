import { Injectable } from '@angular/core';
import { environment } from 'src/environments/environment.prod';
import { StereoBuffer } from '../classes/stereoBuffer';
import { WssBaseService } from './wss-base.service';
import { OpusProviderService, opusWorkerInit, opusWorkerInitOnessage } from './opus-provider.service';
import { OpusDecoderService } from './opus-decoder.service';
// import { OpusEncoderWorker, opusEncoderInit, opusEncoderOnessage } from '../classes/opusEncoderWorker';
import { WrappedWorker } from '../classes/wrappedWorker';

@Injectable({
	providedIn: 'root'
})
export class OpusEncoderService extends WssBaseService {
	useWorker = false;
	frameSize: number;
	packetCounter = new Uint32Array(1);
	encoderWorker: WrappedWorker;
	bufferSizeOptions: number[] = [120, 240, 480, 960, 1920, 2880];
	constructor(
		private opus: OpusProviderService,
		private getopus: OpusDecoderService
	) {
		super();
		if (!environment.production) {
			(window as any).DEBUGEncOpus = this;
		}
		this.wssAddress = environment.streamWss;
		this.frameSize = 960;
		this.packetCounter[0] = 0;
		if (window.location.href.search("encWorker") !== -1) {
			this.useWorker = true;
			this.encoderWorker = new WrappedWorker(
				opusWorkerInitOnessage, opusWorkerInit, [OpusProviderService]
			);
		} else {
			opus.init();
		}
	}

	encodeAndSend(buffer: StereoBuffer) {
		if (this.socket.readyState === WebSocket.OPEN) {
			while (buffer.l.length >= this.frameSize) {
				const interleavedStereo = buffer.popInterleaved(this.frameSize);
				if (this.useWorker) {
					this.encoderWorker.run({
						task: "encodeFloat",
						data: interleavedStereo
					}).then((data: Uint8Array) => {
						this.send(data);
					});
				} else {
					const packet = this.opus.encodeFloat(interleavedStereo);
					if (!packet) { return; }

					/** Some test code to add a packet number */
					// this.packetCounter[0]++;
					// const int32 = this.packetCounter.buffer.slice(0, 4);
					// const uint8 = new Uint8Array(int32);
					// const wrapptedPacket = new Uint8Array(packet.length + 4);
					// wrapptedPacket.set(packet);
					// wrapptedPacket.set(uint8, packet.length);
					// console.log(new Uint32Array(wrapptedPacket.buffer.slice(wrapptedPacket.length - 4, wrapptedPacket.length))[0]);
	
					this.send(packet);

					/** Just for debugging, which bypasses the whole serverside */
					// this.getopus.debugMessage(packet, this.frameSize);
				}
			}
		}
	}
}
