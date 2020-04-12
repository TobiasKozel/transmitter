import { Component, OnInit, NgZone } from '@angular/core';
import { interval } from 'rxjs';
import { MatSelectChange } from '@angular/material/select';
import { environment } from 'src/environments/environment';
import { MultiCodecService } from 'src/app/services/multi-codec.service';

@Component({
	selector: 'app-listener',
	templateUrl: './listener.component.html',
	styleUrls: ['./listener.component.css']
})
export class ListenerComponent implements OnInit {

	bufferSize = 512;
	volume = 0.5;

	bufferSizeOptions: number[] = [
		256, 512, 1024, 2048, 4096
	];

	audioContext: AudioContext;
	frameDuration = 0;

	player: ScriptProcessorNode;
	gainNode: GainNode;

	constructor(
		private zone: NgZone,
		public codec: MultiCodecService
	) {
	}

	ngOnInit() {
		if (!environment.production) {
			(window as any).DEBUGListener = this;
			this.play();
		}
	}

	play() {
		if (!this.audioContext) {
			this.audioContext = new AudioContext();
			this.frameDuration = 1000 / this.audioContext.sampleRate;
		}

		if (!this.player) {
			this.player = this.audioContext.createScriptProcessor(
				this.bufferSize, 2, 2
			);
		}

		this.zone.runOutsideAngular(() => {
			// tslint:disable-next-line: deprecation
			this.player.onaudioprocess = (outSignal) => {
				//this.playbackBuffer.fillStereo(outSignal.outputBuffer, this.bufferSize);
				// outSignal.outputBuffer.getChannelData(0).set(stereo.l);
				// outSignal.outputBuffer.getChannelData(1).set(stereo.r);
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
