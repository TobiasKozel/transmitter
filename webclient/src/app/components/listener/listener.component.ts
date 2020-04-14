import { Component, OnInit, NgZone, OnDestroy } from '@angular/core';
import { MatSelectChange } from '@angular/material/select';
import { environment } from 'src/environments/environment';
import { MultiCodec } from 'src/app/classes/MultiCodec';
import { SessionProviderService } from 'src/app/services/session-provider.service';

@Component({
	selector: 'app-listener',
	templateUrl: './listener.component.html',
	styleUrls: ['./listener.component.scss']
})
export class ListenerComponent implements OnInit, OnDestroy {

	constructor(
		public sessionProvider: SessionProviderService
	) {
	}

	ngOnInit() {
		if (!environment.production) {
			(window as any).DEBUGListener = this;
		}
	}

	ngOnDestroy() {
	}

	connect(address: string) {
		console.log(address);
		this.sessionProvider.createSession(address);
	}

}
