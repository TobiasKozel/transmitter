# Server

## Endpoints

### get_api_info
Called before a client opens a session. Will return API version and a UDP port where the server is listening for packets.

### get_id
This will register the client session to the server. Called with a "port" query which is used to
link up a session ID and a UDP port which sends data. If the client already has an ID it will also be passed with an "id" query. The server will return the ID the client has from now on.

### connect_as_listener
Will register a client as a listener to another client. Called with the query "id" (listener) and "peer" (id of the broadcasting client). Returns success.

### get_status
Returns the viewcount and whether the client is valid.

## Serving the client
If none of these endpoints is hit, node will serve the static weblient files in the ./static folder.
If you build it, make sure to copy it over from the dist folder and make sure the index.html is ./static/

## Websocket server
The websocket server will also listen to the same address.

If you wish to use for example nginx to forward all requests to the node server, it needs to be able to upgrade the connection  like this:
`proxy_set_header Upgrade $http_upgrade;
proxy_set_header Connection "upgrade";`

## Certificats
In order to use WSS connections and an AudioConext the connection needs to be secure.
So you'll need to provide certificates in the ./cert folder (privkey.pem and cert.pem).

# Installing and Running
`npm i && node index.js`