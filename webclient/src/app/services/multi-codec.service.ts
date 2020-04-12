import { Injectable } from '@angular/core';
import { MultiCodec } from 'src/app/classes/MultiCodec'
import { environment } from 'src/environments/environment'

// <reference path="@types/emscripten" />
declare const Module;
@Injectable({
    providedIn: 'root'
})
export class MultiCodecService {

    private isLoaded = false;

    constructor() {
        if (!environment.production) {
            (<any>window).DEBUGcodecs = [];
        }
        this.loadCodec();
    }

    public contructCodec(): MultiCodec {
        if (this.isLoaded) {
            return new MultiCodec();
        }
        return null;
    }

    public destroyCodec(codec: MultiCodec) {
        codec.destroy();
    }

    private loadCodec() {
        const onload = () => {
			console.log("codecs loaded!");
			this.isLoaded = true;
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
