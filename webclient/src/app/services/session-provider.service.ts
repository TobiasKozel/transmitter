import { Injectable, OnDestroy } from '@angular/core';
import { MultiCodec } from '../classes/MultiCodec';
import { WasmLoaderService } from './wasm-loader.service';
import { URLParser } from '../classes/URLParser';

export class ClientSession {
	private codecInstance: MultiCodec;

	public displayName = "";
	public codecBufferSize = 2048;
	public views = 0;

	constructor(
		private wasm: WasmLoaderService,
		private url: URLParser
	) {
		this.displayName = url.path;
		wasm.contructCodec((c) => {
			this.codecInstance = c;
		});
	}

	public codecBufferSizeChanged(size: number) {
		this.codecBufferSize = size;
		this.codecInstance.setBufferSize(size);
	}

	public destroy() {
		this.wasm.destroyCodec(this.codecInstance);
	}
}

@Injectable({
	providedIn: 'root'
})
export class SessionProviderService implements OnDestroy {
	public sessions: ClientSession[] = [];
	constructor(
		private wasm: WasmLoaderService
	) {
		this.createSession(window.location.href);
	}

	createSession(url: string) {
		if (url.search("!") === -1) { return; }
		this.wasm.constructURLParser(url, (parsed) => {
			if (parsed.valid) {
				let s = new ClientSession(this.wasm, parsed);
				this.sessions.push(s);
			}
		});
	}

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
}
