import { Component, OnInit, Input } from '@angular/core';
import { SessionProviderService } from 'src/app/services/session-provider.service';
import { ClientSession } from 'src/app/classes/ClientSession';

@Component({
	selector: 'app-listener-session',
	templateUrl: './listener-session.component.html',
	styleUrls: ['./listener-session.component.scss']
})
export class ListenerSessionComponent implements OnInit {
	@Input("session") session: ClientSession;

	constructor(private sessionProvider: SessionProviderService) { }

	ngOnInit() {
	}

	close() {
		this.sessionProvider.destroySession(this.session);
	}

	toggleMute() {
		this.session.muted = !this.session.muted;
	}

}
