import { Component, OnInit, NgZone, OnDestroy } from '@angular/core';
import { interval } from 'rxjs';
import { MatSelectChange } from '@angular/material/select';
import { environment } from 'src/environments/environment';
import { MultiCodecService } from 'src/app/services/multi-codec.service';
import { MultiCodec } from 'src/app/classes/MultiCodec';

@Component({
	selector: 'app-listener',
	templateUrl: './listener.component.html',
	styleUrls: ['./listener.component.css']
})
export class ListenerComponent implements OnInit, OnDestroy {

	bufferSize = 512;
	volume = 0.5;

	bufferSizeOptions: number[] = [
		256, 512, 1024, 2048, 4096
	];

	audioContext: AudioContext;

	player: ScriptProcessorNode;
	gainNode: GainNode;
	codecInstance: MultiCodec = undefined;

	constructor(
		private zone: NgZone,
		public codecProvider: MultiCodecService
	) {
	}

	ngOnInit() {
		if (!environment.production) {
			(window as any).DEBUGListener = this;
			// this.play();
		}
		this.codecProvider.contructCodec((codec) => {
			this.codecInstance = codec;
		});
	}

	ngOnDestroy() {
		this.codecProvider.destroyCodec(this.codecInstance);
	}

	play() {
		if (!this.audioContext) {
			this.audioContext = new AudioContext();
		}

		if (!this.player) {
			this.player = this.audioContext.createScriptProcessor(
				this.bufferSize, 2, 2
			);
		}

		this.zone.runOutsideAngular(() => {
			this.player.onaudioprocess = (outSignal) => {
				if (!this.codecInstance) { return; }
				this.codecInstance.popSamples(
					[
						outSignal.outputBuffer.getChannelData(0),
						outSignal.outputBuffer.getChannelData(1)
					],
					this.bufferSize
				);
			};
		});

		if (!this.gainNode) {
			this.gainNode = this.audioContext.createGain();
		}

		this.player.connect(this.gainNode);
		this.gainNode.connect(this.audioContext.destination);
	}

	stop() {

	}

	bufferSizeChaned(event: MatSelectChange) {
		this.bufferSize = event.value;
		this.stop();
		this.player.disconnect();
		this.player = null;
		this.play();
	}

}
