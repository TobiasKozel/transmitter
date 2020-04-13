import { Injectable, OnDestroy } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { MultiCodecService } from './multi-codec.service';
import { MultiCodec } from '../classes/MultiCodec';

@Injectable({
	providedIn: 'root'
})
export class SessionProviderService implements OnDestroy {
	private codecInstance: MultiCodec;
	constructor(
		private http: HttpClient,
		private codec: MultiCodecService
	) {
		codec.contructCodec((c) => {
			this.codecInstance = c;
			http.get<any>("")
		})
	}



	connect(url: string) {

	}

	ngOnDestroy() {
		this.codecInstance.destroy();
	}
}
