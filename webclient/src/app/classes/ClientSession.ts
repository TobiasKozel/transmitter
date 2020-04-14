import { MultiCodec } from './MultiCodec';
import { WasmLoaderService } from '../services/wasm-loader.service';
import { URLParser } from './URLParser';
import { environment } from 'src/environments/environment';
import { ApiService } from '../services/api.service';

export class ClientSession {
    public codecInstance: MultiCodec;
    private socket: WebSocket;
    private shutdown = false;

    public valid = false;
    public error = false;
    public apiAddress = "";
    public id = "";
    public peerId = "";

	public displayName = "";
	public codecBufferSize = 2048;
	public views = 0;

	constructor(
        private wasm: WasmLoaderService,
        private api: ApiService,
		private url: URLParser
	) {
        this.displayName = url.path;

        if (!environment.production && "a") {
            this.apiAddress = "https://localhost:55555";
        } else {
            this.apiAddress = url.noPath;
        }

        api.getApiInfo(this).subscribe((info) => {
            console.log("Connected to API. Version " + info.version);
            api.getId(this).subscribe((resp) => {
                this.id = resp.id;
                this.peerId = url.path.substring(1); // Trim away the slash
                this.openSocket();
            });
        });
        
		wasm.contructCodec((c) => {
			this.codecInstance = c;
		});
    }
    
    private openSocket() {
        if (this.socket) { return; }
        this.error = false;
        this.socket = new WebSocket(this.apiAddress.replace("http", "ws") + "/" + this.id);
        this.socket.binaryType = "arraybuffer";
        console.log("Socket connection opened");
        this.socket.onclose = () => {
            if (!this.shutdown) {
                console.log("Socket timedout...");
                this.openSocket();
            } else {
                console.log("Socket closed...");
                this.shutdown = false;
            }
        };
        this.socket.onerror = (error) => {
            console.error("Socket error!");
            console.error(error);
            this.error = true;
        };

        this.socket.onopen = () => {
            this.api.connectAsListener(this).subscribe((resp) => {
                if (resp.success) {
                    this.valid = true;
                }
            });
        };

        this.socket.onmessage = (message: MessageEvent) => {
            const packet = new Uint8Array(message.data);
            this.codecInstance.decode(packet);
        };

    }

    private closeSocket() {
        if (!this.socket) { return; }
        this.shutdown = true;
        this.socket.close();
        this.socket = null;
    }

	public codecBufferSizeChanged(size: number) {
		this.codecBufferSize = size;
		this.codecInstance.setBufferSize(size);
	}

	public destroy() {
        this.closeSocket();
		this.wasm.destroyCodec(this.codecInstance);
	}
}