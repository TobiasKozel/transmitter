/**
 * Crazy construct which allows creating a webworker with code from inside angular
 * and also can hold persistent data
 */
export class WrappedWorker {
	private worker: Worker;
	private runningTasks = new Map<number, (data: any) => void>();
	private taskCounter = 0;
	/**
	 * @param onMessage The function called when the service recieves a message
	 * @param init The init function called as soon as the webworker is created
	 * @param persistentData The data like classes objects etc which will persist as long as the worker exists
	 */
	constructor(
		onMessage: (message: any, callback: (data?: any) => void) => void,
		init: () => void,
		persistentData: [{toString: () => string}]
	) {
		let persitantDataTemplate = "";
		for (const i of persistentData) {
			persitantDataTemplate += i.toString() + "\n";
		}
		const template =
		`${ persitantDataTemplate }
		self.addEventListener("message", function (e) {
			(
				${ onMessage.toString() }
			)(e.data.payload, function (data) {
				postMessage({
					payload: data,
					promiseId: e.data.promiseId
				});
			})
		});
		(${ init.toString() })();`;

		const blob = new Blob([template], { type: "text/javascript" });
		const url = URL.createObjectURL(blob);
		const worker = new Worker(url);
		worker.addEventListener("message", (ev: MessageEvent) => {
			const id = ev.data.promiseId;
			if (this.runningTasks.has(ev.data.promiseId)) {
				const callback = this.runningTasks.get(id);
				this.runningTasks.delete(id);
				callback(ev.data.payload);
			}
		});
		this.worker = worker;
	}

	run(data: any): Promise<any> {
		const id = this.taskCounter++;
		const promise = new Promise((resolve, reject) => {
			this.runningTasks.set(id, resolve);
			this.worker.postMessage({
				payload: data,
				promiseId: id
			});
		});
		return promise;
	}
}