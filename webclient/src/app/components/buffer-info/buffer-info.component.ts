import { Component, OnInit, Input, ChangeDetectorRef, Renderer2 } from '@angular/core';
import { StereoBuffer } from 'src/app/classes/stereoBuffer';

@Component({
	selector: 'app-buffer-info',
	templateUrl: './buffer-info.component.html',
	styleUrls: ['./buffer-info.component.css']
})
export class BufferInfoComponent implements OnInit {
	@Input() sampleRate = 48000;
	@Input() buffer: StereoBuffer;
	frameDuration = 1;
	constructor(
		private cdr: ChangeDetectorRef
	) {
		cdr.detach();
	}

	ngOnInit() {
		this.frameDuration = 1000 / this.sampleRate;
		setInterval(() => {
			this.cdr.detectChanges();
		}, 300);
	}

}
