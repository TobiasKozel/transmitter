import { Component, OnInit, Input, Output, EventEmitter, ViewChild, TemplateRef } from "@angular/core";
import { MatDialog, MatDialogRef } from "@angular/material/dialog";

@Component({
	selector: "app-popup",
	template: `
		<ng-template #dialogContent>
			<div style="font-family: Roboto,'Helvetica Neue',sans-serif;">
				<ng-content></ng-content>
			</div>
		</ng-template>
	`
})
export class PopupComponent implements OnInit {

	@ViewChild("dialogContent", { static: true }) dialogContent: TemplateRef<any>;

	@Input() set open(state: boolean) {
		if (state) { this.openModal(); } else { this.closeModal(); }
	}
	get open(): boolean {
		return this._modalOpen;
	}

	/**
	 * Emits a event once the modal is closed
	 */
	@Output() readonly closed: EventEmitter<any> = new EventEmitter<any>();


	private _modalOpen = false;

	private _dialogRef: MatDialogRef<any>;

	private delayedOpenTimer: any = null;

	constructor(
		public popup: MatDialog
	) { }

	ngOnInit() {
	}

	openModal() {
		if (this._modalOpen) {
			return;
		}
		this._modalOpen = true;
		if (this.delayedOpenTimer) {
			clearTimeout(this.delayedOpenTimer);
		}
		this.delayedOpenTimer = setTimeout(() => {
			if (!this._modalOpen) {
				return;
			}
			this._dialogRef = this.popup.open(this.dialogContent);

			this._dialogRef.afterClosed().subscribe(() => {
				this._modalOpen = false;
				this.closed.emit();
			});
		}, 0);
	}

	closeModal() {
		if (this.delayedOpenTimer) {
			clearTimeout(this.delayedOpenTimer);
		}
		if (this._modalOpen && this._dialogRef) {
			this._dialogRef.close();
		} else { this._modalOpen = false; }
	}

}
