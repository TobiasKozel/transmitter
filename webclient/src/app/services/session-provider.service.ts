import { Injectable, OnDestroy } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { MultiCodecService } from './multi-codec.service';
import { MultiCodec } from '../classes/MultiCodec';

export class ClientSession {
	private codecInstance: MultiCodec;
	constructor(
		private http: HttpClient,
		private codec: MultiCodecService,
		public url: string
	) {
		codec.contructCodec((c) => {
			this.codecInstance = c;
		});
	}

	destroy() {
		this.codec.destroyCodec(this.codecInstance);
	}
}

@Injectable({
	providedIn: 'root'
})
export class SessionProviderService implements OnDestroy {
	public sessions: ClientSession[] = [];
	constructor(
		private http: HttpClient,
		private codec: MultiCodecService
	) {

	}

	createSession(url: string): ClientSession {
		let s = new ClientSession(this.http, this.codec, url);
		this.sessions.push(s);
		return s;
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
