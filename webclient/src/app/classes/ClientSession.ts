import { MultiCodec } from './MultiCodec';
import { WasmLoaderService } from '../services/wasm-loader.service';
import { URLParser } from './URLParser';
import { ApiService } from '../services/api.service';
import { SessionProviderService } from '../services/session-provider.service';

export class ClientSession {
    /**
     * The main codec instance which can decode and encode
     * all the codevs supported by transmitter
     * It has it's own buffers for packets and audio
     */
    private codecInstance: MultiCodec;

    /**
     * The script processor which is used to insert the
     * decoder as a source and will do the audio codecs
     */
    private processor: ScriptProcessorNode;

    /**
     * The processor is connected to it and it will controll the volume
     */
    public gainNode: GainNode;

    public muted = false;
    
    /**
     * Socket to send and recieve the audio data
     */
    private socket: WebSocket;

    /**
     * Will be true if the socket should be closed
     * so it won't autoreconnect
     */
    private shutdown = false;

    private updateIntervalTimer = -1;


    public valid = false;
    public error = false;

    /**
     * All api requests will be sent there, accessed from the api provider
     */
    public apiAddress = "";

    public id = "";
    public peerId = "";

    /**
     * The name to be displayed in the gui
     */
	public displayName = "";
	public codecBufferSize = 2048;
	public views = 0;

	constructor(
        private provider: SessionProviderService,
        private wasm: WasmLoaderService,
        private api: ApiService,
		private url: URLParser
	) {
        this.displayName = url.path;
        this.apiAddress = url.noPath;

        api.getApiInfo(this).subscribe((info) => {
            // API is reachable
            console.log("Connected to API. Version " + info.version);
            api.getId(this).subscribe((resp) => {
                // got an id assigned
                this.id = resp.id;
                this.peerId = url.path.substring(1); // Trim away the slash
                this.openSocket();
                // We'll continue in socket.onopen
            });
        });
        
		wasm.contructCodec((c) => {
			this.codecInstance = c;
        });

        const update: TimerHandler = () => {
            if (this.valid) {
                api.updateListenerCount(this);
            }
        };
        this.updateIntervalTimer = setInterval(update, 2000);
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
                // Now that's a callstack
                if (resp.success) {
                    this.valid = true;
                }
            });
        };

        this.socket.onmessage = (message: MessageEvent) => {
            if (this.muted) { return; }
            const packet = new Uint8Array(message.data);
            this.codecInstance.decode(packet);
        };

    }

    public cleanUpAudioContext() {
        if (this.processor) {
            this.processor.disconnect();
        }
        if (this.gainNode) {
            this.gainNode.disconnect();
        }
    }

    /**
     * Called from the session provider service which also has the audio context
     * @param ctx 
     * @param buffersize 
     */
    public setUpAudioContext(ctx: AudioContext, buffersize: number): GainNode {
        this.cleanUpAudioContext();
        this.codecInstance.setSampleRate(ctx.sampleRate);
        this.processor = ctx.createScriptProcessor(
            buffersize, 0, 2
        );
        this.processor.onaudioprocess = (outSignal) => {
            if (this.valid) {
                this.codecInstance.popSamples(
                    [
                        outSignal.outputBuffer.getChannelData(0),
                        outSignal.outputBuffer.getChannelData(1)
                    ],
                    buffersize
                );
            }
        };

        this.gainNode = ctx.createGain();
        this.processor.connect(this.gainNode);
        return this.gainNode;
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
        clearTimeout(this.updateIntervalTimer);
        this.cleanUpAudioContext();
        this.closeSocket();
		this.wasm.destroyCodec(this.codecInstance);
	}
}