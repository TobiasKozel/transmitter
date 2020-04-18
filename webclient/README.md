# Low Latency Opus streaming

This project was generated with [Angular CLI](https://github.com/angular/angular-cli) version 8.3.3.

## Development server

Run `ng serve` for a dev server. Navigate to `http://localhost:4200/`. The app will automatically reload if you change any of the source files.

## Build

Run `ng build` to build the project. The build artifacts will be stored in the `dist/` directory. Use the `--prod` flag for a production build.
To serve it from the master server it needs to be copied over.

## Front end

The frontend can listen to a stream by recieving opus packets from a web socket.
It uses a emscripten compiled version of the opus reference encoder and some additional wrapper classes.

## Back end

The backend server is in ../server/
A simple NodeJs server which simply distributes the packets from a streamer across all listeners.

## Compiling the C++ part

Instructions in the ./wasm folder
