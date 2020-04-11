import { environment } from 'src/environments/environment.prod';

export class WssBaseService {
	socket: WebSocket;
	intentionalClose = false;
	wssAddress = "";
	error = "";
	packedNumber = 0;

	constructor() {
	}

	onMessage(data: ArrayBuffer) {
	}

	connect() {
		if (!this.socket) {
			this.packedNumber = 0;
			this.socket = new WebSocket(this.wssAddress);
			this.socket.binaryType = "arraybuffer";

			this.socket.onclose = () => {
				if (!environment.production) {
					console.log("Streamer Socket closed");
				}
				if (!this.intentionalClose) {
					this.connect();
				} else {
					this.intentionalClose = false;
				}
			};

			this.socket.onerror = (error) => {
				this.error = error.type;
			};

			this.socket.onopen = () => {
				this.error = "";
			};

			this.socket.onmessage = (message: MessageEvent) => {
				this.onMessage(message.data);
			};
		} else if (this.socket.readyState === WebSocket.CLOSED) {
			delete this.socket;
			this.connect();
		}
	}

	send(data: ArrayBuffer): boolean {
		if (this.socket.readyState === WebSocket.OPEN) {
			this.socket.send(data);
			return true;
		} else {
			return false;
		}
	}

	disconnect() {
		this.error = "";
		this.intentionalClose = true;
		this.socket.close();
	}

	isConnected(): boolean {
		if (!this.socket) {
			return false;
		}
		return this.socket.readyState === WebSocket.OPEN;
	}

	isIntermediate(): boolean {
		if (!this.socket) {
			return false;
		}
		return this.socket.readyState === WebSocket.CONNECTING ||
				this.socket.readyState === WebSocket.CLOSING;
	}

	isClosed(): boolean {
		if (!this.socket) {
			return true;
		}
		return this.socket.readyState === WebSocket.CLOSED;
	}
}
