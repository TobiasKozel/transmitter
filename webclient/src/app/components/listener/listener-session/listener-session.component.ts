import { Component, OnInit, Input } from '@angular/core';
import { ClientSession, SessionProviderService } from 'src/app/services/session-provider.service';

@Component({
	selector: 'app-listener-session',
	templateUrl: './listener-session.component.html',
	styleUrls: ['./listener-session.component.css']
})
export class ListenerSessionComponent implements OnInit {
	public muted = false;
	public volume = 1.0;
	public codecBufferSize = 2048;
	public views = 0;
	@Input("session") session: ClientSession;

	constructor(private sessionProvider: SessionProviderService) { }

	ngOnInit(): void {
	}

	close() {
		this.sessionProvider.destroySession(this.session);
	}

	toggleMute() {
		this.muted = !this.muted;
	}

}
