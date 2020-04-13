import { Injectable, OnDestroy } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { MultiCodecService } from './multi-codec.service';
import { MultiCodec } from '../classes/MultiCodec';

export class Session {
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
	public sessions: Session[] = [];
	constructor(
		private http: HttpClient,
		private codec: MultiCodecService
	) {

	}

	createSession(url: string): Session {
		let s = new Session(this.http, this.codec, url);
		this.sessions.push(s);
		return s;
	}

	destroySession(s: Session) {
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
