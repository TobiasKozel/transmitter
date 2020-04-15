import { Injectable, OnDestroy } from '@angular/core';
import { WasmLoaderService } from './wasm-loader.service';
import { ClientSession } from '../classes/ClientSession';
import { ApiService } from './api.service';

@Injectable({
	providedIn: 'root'
})
export class SessionProviderService implements OnDestroy {

	private audioContext: AudioContext;
	private deviceBufferSize = 1024;

	public autoStartSession = "";


	/**
	 * UI bound
	 */
	public sessions: ClientSession[] = [];
	constructor(
		private wasm: WasmLoaderService,
		private api: ApiService
	) {
		var isMobile = false; //initiate as false
		// device detection
		if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|ipad|iris|kindle|Android|Silk|lge |maemo|midp|mmp|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows (ce|phone)|xda|xiino/i.test(navigator.userAgent) 
    		|| /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(navigator.userAgent.substr(0,4))) { 
    		isMobile = true;
		}
		if (isMobile) {
			this.deviceBufferSize = 4096;
		}
		/**
		 * Try decoding the url since it may contain a peer id
		 */
		const url = window.location.href;
		if (url.search("!") !== -1) {
			this.autoStartSession = window.location.href;
		}

	}

	/**
	 * Will create a listener session and setup the audio context for it
	 * @param url the url which will be checked beofre creating the session
	 */
	createSession(url: string) {
		if (!this.audioContext) {
			/**
			 * The audio context needs to be created from some kind of user interaction
			 * like a click on an element. That's why it's setup here
			 */
			this.setUpAudioContext(() => {
				this.createSession(url);
			});
			return;
		}
		if (url.search("!") === -1) { return; }
		this.wasm.constructURLParser(url, (parsed) => {
			if (parsed.valid) {
				let s = new ClientSession(this, this.wasm, this.api, parsed);
				this.setUpClientAudioConext(s);
				this.sessions.push(s);
			}
		});
	}

	/**
	 * Will destory the session and clean up its audio context
	 * @param s W
	 */
	destroySession(s: ClientSession) {
		let index = this.sessions.indexOf(s);
		if (index !== -1) {
			this.sessions[index].destroy();
			this.sessions.splice(index, 1);
		}
	}

	ngOnDestroy() {
		let sessions = this.sessions;
		this.sessions = [];
		for (let i of sessions) {
			i.destroy();
		}
	}

	/**
	 * Cleans up the audio context for the browser and all the session
	 */
	private cleanUpAudioContext() {
		for (let i of this.sessions) {
			i.cleanUpAudioContext();
		}
		if (this.audioContext) {
			return this.audioContext.close();
		}
		return null;
	}

	/**
	 * Will setup the audio context for a single session and connect it
	 */
	private setUpClientAudioConext(c:ClientSession) {
		c.setUpAudioContext(this.audioContext, this.deviceBufferSize).connect(
			this.audioContext.destination
		);
	}

	/**
	 * Will setup the audio context for all session and the browser
	 * does a clean up if there was already a context
	 * so it can be called to update a change in deviceBufferSize
	 */
	private setUpAudioContext(callback: () => void = null) {
		let cleanup = this.cleanUpAudioContext();
		const setup = () => {
			let ctx = window.AudioContext
				|| (<any>window).webkitAudioContext || false;
			if (!ctx) {
				console.error("Can't create an audio context!");
				return;
			}
			this.audioContext = new ctx(
				{latencyHint: "playback", sampleRate: 48000}
			);
			console.log("Audio context created with a samplerate of " + this.audioContext.sampleRate);
			if (this.audioContext.sampleRate !== 48000) {
				console.error("Did not get the desired 48000Hz samplerate. Resampling needed.");
			}
			for (let i of this.sessions) {
				this.setUpClientAudioConext(i);
			}
			if (callback) {
				callback();
			}
		};
		if (cleanup) {
			cleanup.then(setup);
		} else {
			setup();
		}

	}

	public getDeviceBufferSize() {
		return this.deviceBufferSize;
	}

	public setDeviceBufferSize(s: number) {
		this.deviceBufferSize = s;
		this.setUpAudioContext();
	}
}
