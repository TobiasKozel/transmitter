import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientModule } from '@angular/common/http';

import { AppComponent } from './app.component';
import { ListenerComponent } from './components/listener/listener.component';
import { StartComponent } from './components/start/start.component';
import { MaterialModule } from "./../material.module";
import { ListenerSessionComponent } from './components/listener/listener-session/listener-session.component';

@NgModule({
	declarations: [
		AppComponent,
		ListenerComponent,
		StartComponent,
		ListenerSessionComponent,
	],
	imports: [
		HttpClientModule,
		ReactiveFormsModule,
		FormsModule,
		BrowserModule,
		BrowserAnimationsModule,
		MaterialModule
	],
	providers: [],
	bootstrap: [AppComponent]
})
export class AppModule { }
