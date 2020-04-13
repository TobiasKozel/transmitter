import { Injectable } from '@angular/core';
import { MultiCodec } from 'src/app/classes/MultiCodec'
import { environment } from 'src/environments/environment'

// <reference path="@types/emscripten" />
declare const Module;
@Injectable({
	providedIn: 'root'
})
export class MultiCodecService {
	private pendingConstructions: ((param: MultiCodec) => void)[] = [];
	private isLoaded = false;

	constructor() {
		if (!environment.production) {
			(<any>window).DEBUGcodecs = [];
		}
		this.loadCodec();
	}

	/**
	 * Use this to contruct the object since the wasm might not
	 * be loaded yet
	 */
	public contructCodec(callback: (param: MultiCodec) => void) {
		if (this.isLoaded) {
			callback(new MultiCodec());
		} else {
			this.pendingConstructions.push(callback);
		}
	}

	/**
	 * Use this to destroy a decoder
	 * Doen't do anything special, but might in the future
	 * @param codec The codec object to destroy
	 */
	public destroyCodec(codec: MultiCodec) {
		codec.destroy();
	}

	/**
	 * Will load the wasm or plain js version of the codec
	 * if the url contains nowasm
	 */
	private loadCodec() {
		const onload = () => {
			console.log("codecs loaded!");
			this.isLoaded = true;
			for (let i of this.pendingConstructions) {
				this.contructCodec(i);
			}
			this.pendingConstructions = [];
		};
		
		/** Load it via a script tag */
		const script = document.createElement("script");
		const url = window.location.href;
		if (url.search("nowasm") === -1) {
			console.log("Using wasm for codecs");
			script.onload = () => {
				Module.onRuntimeInitialized = onload;
			};
			script.src = "/assets/multicodec_wasm.js";
		} else {
			/** Allow using the non wasm version if the url contains "nowasm" */
			script.onload = onload;
			console.log("Not using wasm for codecs");
			script.src = "/assets/multicodec_nasm.js";
		}
		console.log("Loading codecs...");
		document.getElementsByTagName("head")[0].appendChild(script);
	}
}
