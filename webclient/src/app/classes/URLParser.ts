declare const Module;

export class URLParser {
    private emURLObject: any = undefined;

    public protocol: string = "";
    public host: string = "";
    public path: string = "";
    public query: string = "";
    public port: number = 0;
    public ssl: boolean = false;
    public valid: boolean = false;
    public nonDefaultPort: boolean = false;
    public noPath: string = "";

    constructor(url: string) {
        this.emURLObject = new Module.URLParser(url);
        this.protocol = this.emURLObject.protocol;
        this.host = this.emURLObject.host;
        this.path = this.emURLObject.path;
        this.query = this.emURLObject.query;
        this.port = this.emURLObject.port;
        this.ssl = this.emURLObject.ssl;
        this.valid = this.emURLObject.valid;
        this.nonDefaultPort = this.emURLObject.nonDefaultPort;
        this.noPath = this.emURLObject.reconstruct(true, false, false);
        this.emURLObject.delete();
    }
}