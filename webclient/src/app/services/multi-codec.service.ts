import { Injectable } from '@angular/core';
import { MultiCodec } from 'src/app/classes/MultiCodec'
import { environment } from 'src/environments/environment'
import { WasmLoaderService } from './wasm-loader.service';

@Injectable({
	providedIn: 'root'
})
export class MultiCodecService {
	constructor(private wasm: WasmLoaderService) {
		if (!environment.production) {
			// Just to keep track of the codec object
			(<any>window).DEBUGcodecs = [];
		}
	}

	/**
	 * Use this to contruct the object since the wasm might not
	 * be loaded yet
	 */
	public contructCodec(callback: (param: MultiCodec) => void) {
		this.wasm.construct(() => {
			callback(new MultiCodec());
		});
	}

	/**
	 * Use this to destroy a decoder
	 * Doens't do anything special, but might in the future
	 * @param codec The codec object to destroy
	 */
	public destroyCodec(codec: MultiCodec) {
		if (codec) {
			codec.destroy();
		}
	}
}
