import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { ClientSession } from 'src/app/classes/ClientSession';

@Injectable({
	providedIn: 'root'
})
export class ApiService {

	constructor(
		private http: HttpClient
	) { }

	public keepAlive(s: ClientSession) {
		this.http.get(s.apiAddress + "/keep_alive?id=" + s.id).subscribe();
	}

	public updateListenerCount(s: ClientSession) {
		this.http.get<{listeners: number, valid: boolean}>(
			s.apiAddress + "/get_status?id=" + s.peerId
		).subscribe((data) => {
			s.views = data.listeners;
			s.valid = data.valid;
		});
	}

	public getApiInfo(s: ClientSession) {
		return this.http.get<{version: number, port: number}>(
			s.apiAddress + "/get_api_info"
		);
	}

	public getId(s: ClientSession) {
		return this.http.get<{id: string}>(
			s.apiAddress + "/get_id?port=0"
		);
	}

	public connectAsListener(s: ClientSession) {
		return this.http.get<{success: boolean}>(
			s.apiAddress + "/connect_as_listener?id=" + s.id + "&peer=" + s.peerId
		);
	}
}
