import { Component, OnInit, Input, ViewChild, ElementRef, NgZone } from '@angular/core';
import { StereoBuffer } from 'src/app/classes/stereoBuffer';

@Component({
	selector: 'app-vu-meter',
	templateUrl: './vu-meter.component.html',
	styleUrls: ['./vu-meter.component.css']
})
export class VuMeterComponent implements OnInit {
	@Input() buffer: StereoBuffer;
	@Input() steps = 1;
	@Input() resetSpeed = 0.9;
	@Input() frameSkip = 1;
	@ViewChild("canvas", { static: true }) canvas: ElementRef<HTMLCanvasElement>;
	context2d: CanvasRenderingContext2D;
	width: number;
	height: number;
	private volume = 0;
	private frame = 0;
	constructor(
		private zone: NgZone
	) { }

	ngOnInit() {
		this.context2d = this.canvas.nativeElement.getContext("2d");
		this.height = this.canvas.nativeElement.height;
		this.width = this.canvas.nativeElement.width;
		this.zone.runOutsideAngular(() => {
			this.animate();
		});
	}

	animate() {
		if (this.buffer.l.length) {
			if (this.frame++ === this.frameSkip) {
				this.frame = 0;
				let value = 0;
				let count = 0;
				for (let i = 0; i < this.buffer.l.length; i += this.steps) {
					const sample = this.buffer.l[i];
					value += sample * sample;
					count++;
				}
				value = value / count;
				value = Math.sqrt(value);
				this.volume = Math.max(value, this.resetSpeed * this.volume);
				value = this.volume * this.width * 4;
				this.context2d.clearRect(0, 0, this.width, this.height);
				this.context2d.fillStyle = "#FFFFFF";
				this.context2d.fillRect(0, 0, value, this.height);
			}
		}
		requestAnimationFrame(() => {
			this.animate();
		});
	}
}
