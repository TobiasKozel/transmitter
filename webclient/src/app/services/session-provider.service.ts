import { Injectable, OnDestroy } from '@angular/core';
import { WasmLoaderService } from './wasm-loader.service';
import { ClientSession } from '../classes/ClientSession';
import { ApiService } from './api.service';

@Injectable({
	providedIn: 'root'
})
export class SessionProviderService implements OnDestroy {

	private audioContext: AudioContext;
	private deviceBufferSize = 512;

	public autoStartSession = "";


	/**
	 * UI bound
	 */
	public sessions: ClientSession[] = [];
	constructor(
		private wasm: WasmLoaderService,
		private api: ApiService
	) {
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
			this.audioContext = new AudioContext(
				{latencyHint: "playback", sampleRate: 48000}
			);
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
