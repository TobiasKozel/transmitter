# Transmitter
A vst/au plugin and webclient which makes sharing audio easy (Hopefully).

Transmitter consists out of 3 components

## VST/AU Plugin

![screenshot](https://i.imgur.com/XKIu753.png)

It's a plugin which allows you to stream and receive Audio, similar to Reastream.
In Master-Server mode it'll display a URL as soon as it's connected.
Anyone with it can listen to the audio which passes through it.
To get a bidirectional connection between two DAWs simply exchange the URLs and connect on Both ends.
If the plugin is on the master chain in both DAWs and connected to each other it's like sharing the same audio device.
The latency depends on the codec and its settings as well as on the network itself.

## Webclient

![screenshot](https://i.imgur.com/lq9TgrU.png)

The webclient is able to play back streams from the plugin. This isn't as low latency as the plugin but fairly convenient.
More info in the webclient directory. 

## Nodejs Master Server
This is only relevant if you wish to use your own server to relay audio.
It handles sessions and gives out unique IDs to all the clients.
According to the connections between clients it sends packets from broadcasting to listening clients.
Webclients will recieve the data via websocket, while the native plugin uses UDP (IPv4 only) instead.
A broadcasting client can stream to multiple clients, but only listen to one at a time.
More info in the server directory.
