import { Injectable } from '@angular/core';

// <reference path="@types/emscripten" />
declare const Module;

@Injectable({
	providedIn: 'root'
})

/**
 * Loads the wasm module and handles construction of
 * objects from c++
 */
export class WasmLoaderService {
	private pendingConstructions: (() => void)[] = [];
	private isLoaded = false;

	constructor() {
		const onload = () => {
			console.log("*asm module loaded!");
			this.isLoaded = true;
			const c = this.pendingConstructions;
			this.pendingConstructions = [];
			for (let i of c) { i(); }
		};
		
		/** Load it via a script tag */
		const script = document.createElement("script");
		const url = window.location.href;
		if (url.search("nowasm") === -1) {
			console.log("Loading wasm module...");
			script.onload = () => {
				Module.onRuntimeInitialized = onload;
			};
			script.src = "/assets/multicodec_wasm.js";
		} else {
			/** Allow using the non wasm version if the url contains "nowasm" */
			script.onload = onload;
			console.log("Loading nawm module");
			script.src = "/assets/multicodec_nasm.js";
		}
		document.getElementsByTagName("head")[0].appendChild(script);
	}

	/**
	 * Everything relying on the wasm module should be constructed
	 * via this function to ensure wasm is ready
	 * @param callback Function to be called once
	 *                 the wasm module is loaded
	 */
	public construct(callback: () => void) {
		if (this.isLoaded) {
			callback();
		} else {
			this.pendingConstructions.push(callback);
		}
	}
}
