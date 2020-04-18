# Transmitter
A vst/au plugin and webclient which makes sharing audio easy. (Hopefully)
Transmitter consists out of 3 components

## VST/AU Plugin
It's a plugin which allow to stream and recieve Audio like for example Reastream.
In Master-Server mode it'll display a URL as soon as it's connected.
Anyone with it can listen to the audio which passes through it.
To get a bidirectional connection between two DAWs simply exchange the URLs and connect.

## Webclient
The webclient is able to play back streams from the plugin. This isn't as low latency as the plugin but fairly convenient.
More info in the webclient directory. 

## Nodejs Master Server
This is only relevant if you wish to use your own server to relay audio.
It handles sessions and gives out unique IDs to all the clients.
According to the connections between clients it sends packets from broadcasting to listening clients.
Webclients will recieve the data via websocket, while the native plugin uses UDP (IPv4 only) instead.
A broadcasting client can stream to multiple clients, but only listen to one at a time.
More info in the server directory.
