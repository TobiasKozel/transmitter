import { Injectable, OnDestroy } from '@angular/core';
import { WasmLoaderService } from './wasm-loader.service';
import { ClientSession } from '../classes/ClientSession';
import { ApiService } from './api.service';

@Injectable({
	providedIn: 'root'
})
export class SessionProviderService implements OnDestroy {

	/**
	 * UI bound
	 */
	public sessions: ClientSession[] = [];
	constructor(
		private wasm: WasmLoaderService,
		private api: ApiService
	) {
		this.createSession(window.location.href);
	}

	createSession(url: string) {
		if (url.search("!") === -1) { return; }
		this.wasm.constructURLParser(url, (parsed) => {
			if (parsed.valid) {
				let s = new ClientSession(this.wasm, this.api, parsed);
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
