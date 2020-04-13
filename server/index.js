const http = require("http");
const https = require("https");
const fs = require("fs");
const ws = require("ws");
const WSOPEN = ws.OPEN;
const WebSocketServer = ws.Server;
var url = require("url");

const dgram = require("dgram");
const udp4 = dgram.createSocket("udp4");

/**
 * API config
 */
const API = {
	VERSION: 0.1,
	/**
	 * A https server listen to requests here and handle most of
	 * the management and meta data for clients
	 */
	PORT: 55555,
	/**
	 * The low latency opus stream will use this port
	 */
	UDP_PORT: 55556,
	/**
	 * Websocket audio needs the connection to be https
	 * so we also need certs
	 */
	CREDENTIALS: {
		key: fs.readFileSync("./cert/privkey.pem"),
		cert: fs.readFileSync("./cert/cert.pem")
	},
	TIME_OUT: 20, // time in seconds clients with no sign of beeing alive will be timed out after
	TIME_OUT_INTERVAL: 5 // Time in seconds to update the timeout
};

var tempId = 0;
const UTIL = {
	/**
	 * Will simply return a unique id
	 * TODO make this random
	 * @returns {string} the generated ID
	 */
	generateId: function () {
		return "!" + (tempId++);
	},
	/**
	 * @param {?http.IncomingMessage} req The request to convert to an ip address
	 * @returns {string} the generated ID
	 */
	ipFromReq: function(req) {
		return req.headers['x-forwarded-for'] || req.connection.remoteAddress || 
		req.socket.remoteAddress || (req.connection.socket ? req.connection.socket.remoteAddress : null);
	},
	/**
	 * Log function which adds a timestamp
	 * @param {string} s Message
	 * @param {?http.IncomingMessage} req The request to log an ip address
	 */
	log: function(s, req = undefined) {
		var ip = "";
		var d = new Date();
		var date = ("0" + d.getDate()).slice(-2) + "-" + ("0"+(d.getMonth()+1)).slice(-2) + "-" +
			d.getFullYear() + " " + ("0" + d.getHours()).slice(-2) + ":" + ("0" + d.getMinutes()).slice(-2);
		if (req) {
			ip = this.ipFromReq(req);
		}
		if (ip) {
			console.log(date + " | " + ip + " | " + s);
		}
		else {
			console.log(date + " | " + s);
		}
	},
	/**
	 * Will remove a object from an array
	 * @param {any[]} arr The array to remove an object from
	 * @param {any} el The element to remove
	 * @return {any[]} The new array with the object removed
	 */
	removeFromArray: function(arr, el) {
		const index = arr.indexOf(el);
		if (index >= 0) {
			arr.splice(index, 1);
		}
		return arr;
	}
}

class Connection {
	/**
	 * @param {Client} from The broadcasting client
	 * @param {Client} to The listening client
	 */
	constructor(from, to) {
		this.from = from;
		this.to = to;
	}
}

class Client {
	/**
	 * 
	 * @param {string} ip ipv4 of the client
	 * @param {number} port the port the client uses to send and receive audio data
	 * @param {?string} id The id the client is used to identify
	 */
	constructor(ip, port, id = undefined) {
		/**
		 * A unique id generated by the server
		 */
		if (id) {
			this.id = id;
		} else {
			this.id = UTIL.generateId();
		}

		this.ip = ip;

		this.port = port;
		/**
		 * If it's a wesocket connection
		 */
		this.webSocket = null;
		this.timeOut = 0;
	}

	/**
	 * 
	 * @param {string} ip The ip address
	 * @param {number} port The port
	 */
	setAddress(ip, port) {
		this.ip = ip;
		this.port = port;
	}
}

/**
 * List of all clients currently registered
 * @type {Client[]}
 */
let clients = [];

/**
 * List of all connections
 * @type {Connection[]}
 */
let connections = [];

/**
 * 
 * @param {Uint8Array} packet The packet to distribute to all the listeners
 * @param {*} port 
 * @param {*} address 
 */
function handleOpusPacket(packet, port, address) {
	for (let from of clients) {
		if (from.ip === address && from.port === port) {
			// Found out who sent the packet
			from.timeOut = 0;
			// Figure out who is connected to it as a listener
			for (let connection of connections) {
				if (connection.from === from) {
					connection.to.timeOut = 0;
					if (connection.to.webSocket) {
						// todo
					} else {
						udp4.send(packet, connection.to.port, connection.to.ip);
					}
				}
			}
			// debugger;
			break;
		}
	}
}


/**
 * Will register a client and generate an id for it if none is provided
 * @param {Number} port The port of the clinet
 * @param {string} ip IP of the client
 * @param {string} id The id of the clinet, can be empty if the client is new
 * @returns {string} Will return the client has
 */
function registerClient(port, ip, id) {
	try {
		port = parseInt(port, 10);
	} catch (error) {
		UTIL.log("Register: Failed to parsed Port " + ip + " id " + id + " port " + port);
		return;
	}
	
	if (ip === "::ffff:127.0.0.1" || ip === "::1") {
		ip = "127.0.0.1"; // ipv6 is confusing
	}
	if (id) {
		for (let i of clients) {
			if (i.id === id) {
				i.setAddress(ip, port);
				UTIL.log("Register: Updated existing client " + ip + " id " + id + " port " + port);
				return id;
			}
		}
	}
	const c = new Client(ip, port, id);
	clients.push(c);
	UTIL.log("Register: new client " + ip + " id " + c.id + " port " + port);
	return c.id;
}

/**
 * Will add a listener to the client from selfIf
 * @param {string} selfId Id from the listener
 * @param {string} peerId Id of the client the listener wants to listen to
 */
function startListenTo(selfId, peerId) {
	let self;
	let peer;
	// find the actual objects
	for (let i of clients) {
		if (i.id === selfId) {
			self = i;
		}
		if (i.id === peerId) {
			peer = i;
		}
	}

	if (self && peer) {
		/**
		 * A listener can only listen to one stream
		 * So we'll get rid of any old connections which have
		 * the target as a listener
		 */
		for (let i of connections) {
			if (i.to === self) {
				connections = UTIL.removeFromArray(connections, i);
			}
		}

		connections.push(new Connection(peer, self));
		UTIL.log("Added listener " + self.id + " to " + peer.id);
		return true;
	}
	return false;
}

/**
 * Will connect clients in both ways
 * @param {string} selfId Id from client a
 * @param {string} peerId Id from client b
 */
function connectClients(selfId, peerId) {
	let success = true;
	success = success && startListenTo(selfId, peerId);
	success = success && startListenTo(peerId, selfId);
	return success;
}

/**
 * Will remove all connections to and from a client and the client it self
 * @param {string} selfId The id of the client
 */
function disconnectClient(selfId) {
	for (let i of clients) {
		if (i.id === selfId) {
			for (let j of connections) {
				if (j.to === i || j.from === i) {
					connections = UTIL.removeFromArray(connections, j);
				}
			}
			clients = UTIL.removeFromArray(clients, i);
			UTIL.log("Disconnected client " + selfId);
			return;
		}
	}
}

/**
 * The main API server handler
 * @param {http.IncomingMessage} req
 * @param {http.ServerResponse} res
 */
const handleAPIRequest = function(req, res) {
	const parsed = url.parse(req.url, true);
	const id = parsed.query["id"];
	const pathname = parsed.pathname;
	var response = { type: "not_set" };

	if ("/keep_alive" === pathname) {
		for (let i of clients) {
			if (i.id === id) {
				i.timeOut = 0;
			}
		}
	}

	if ("/get_status" === pathname) {
		response.type = "return_status";
		let listeners = 0;
		for (let i of connections) {
			if (i.from.id === id) {
				listeners++;
			}
		}
		response.listeners = listeners;
	}

	if ("/get_api_info" === pathname) {
		response.type = "return_api_info";
		response.port = API.UDP_PORT;
		response.version = API.VERSION;
	}

	/**
	 * All clients will have to register first, even if they already have an id from some earlier session
	 */
	if ("/get_id" === pathname) {
		response.type = "return_id";
		response.id = registerClient(parsed.query["port"], UTIL.ipFromReq(req), id);
	}

	/**
	 * Will connect only one way which is going to be used for the web listener
	 */
	if ("/connect_as_listener" === pathname) {
		response.type = "connection_result";
		response.success = startListenTo(id, parsed.query["peer"]);
	}

	res.writeHead(200, {"Content-Type": "text/html"});
	res.end(JSON.stringify(response));
};

let apiServer = https.createServer(API.CREDENTIALS, handleAPIRequest).listen(API.PORT);
UTIL.log("API server started on " + API.PORT);

/**
 * The websocket server for opus packets
 */
let wssServer = new WebSocketServer({ server: apiServer });
wssServer.on("listening", () => {
	UTIL.log("WSS server started on " + API.PORT);
});

wssServer.on("connection", (ws, request, client) => {
	// TODO need to find the correct client first
	debugger;
	ws.on("message", (message) => {
		// TODO send the packet to all the, not a priority
		let test = client;
		debugger;
		handleOpusPacket(message, 0, 0);
	});
	ws.on("close", () => {
		// TODO get rid of the client from this connection
		debugger;
	});
});

/**
 * The udp server for opus packets
 */
udp4.on("message", (message, rinfo) => {
	handleOpusPacket(message, rinfo.port, rinfo.address);
});

udp4.on("listening", () => {
	UTIL.log("UDP server started on " + API.UDP_PORT);
});

udp4.bind(API.UDP_PORT);

/**
 * Will check on all clients and throw out the ones that timed out
 */
setInterval(() => {
	for (let i of clients) {
		i.timeOut += API.TIME_OUT_INTERVAL;
		if (i.timeOut > API.TIME_OUT) {
			disconnectClient(i.id);
		}
	}
}, API.TIME_OUT_INTERVAL * 1000);
