# Low Latency Opus streaming

This project was generated with [Angular CLI](https://github.com/angular/angular-cli) version 8.3.3.

## Development server

Run `ng serve` for a dev server. Navigate to `http://localhost:4200/`. The app will automatically reload if you change any of the source files.

## Build

Run `ng build` to build the project. The build artifacts will be stored in the `dist/` directory. Use the `--prod` flag for a production build.

## Front end

The frontend can stream and listen to a stream by sending or recieving opus packets from a web socket url.

It uses a emscripten compiled version of the opus reference encoder.

## Back end

./backend/

A simple NodeJs server which simply distributes the packets from a streamer across all listeners.

## Compiling Opus

Instructions in the ./opus folder
