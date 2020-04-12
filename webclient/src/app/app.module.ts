import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';


import { AppComponent } from './app.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { ListenerComponent } from './components/listener/listener.component';
import { StartComponent } from './components/start/start.component';
import { MaterialModule } from "./../material.module";

@NgModule({
	declarations: [
		AppComponent,
		ListenerComponent,
		StartComponent,
	],
	imports: [
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
