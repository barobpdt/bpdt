@baro.websocketStart(parent, port) {
	not(port) port=8092
	server=Baro.server('websocket')
	server.var(type, 'websocket')
	server.start(port, @baro.websocketAccept)
	server.callbackClient(@baro.websocketDispatch)
	return server;
}
@baro.websocketMessage(client, data) {
	param = _node()
	param.parseJson(data)
	switch(param.type) {
	case join:
		client.set('mode','dev')
		@baro.websocketSendMessage(client, 'joined', 'message:ok')
	case loadPagePath:
		root = Cf.rootNode()
		config = root.get('@watcherFiles').get('webpageWatcher')
		if( data.path) {
			path = data.path
			names = config.watcherNames
			@baro.filePathInfo(path).inject(watchPath, fileName)
			if(config.target==watchPath ) {
				not(names.find(fileName)) {
					names.add(fileName)
				}
			}
			conf('baro.loadPagePath', path, true)
		}
	default:
	}
}
@baro.websocketAccept(client) {
	print("@@ web socket server accept clinet", clinet)
}

@baro.websocketDispatch(client, type) {
	print("@@ web socket client type==$type")
	config=client.config()
	switch(type) {
	case start:
		client.first=true;
		client.var(type, 'websocket')
	case connect:
		print("client connect", config);
	case recv:
		if( client.first ) {
			config.recvRemainSize=0;
			config.recvData='';
			client.first=false;
			data=client.readAll();
			@baro.websocketHandshake( client, data );
		} else {
			data=client.readWs()
			if(data) {
				@baro.websocketMessage(client, data);
			} else {
				print("@@ client recv close");
				client.close()
			}
		}
	default:
		client.close();
	}
}
@baro.websocketSendMessage(client, type, data) {
	param=_node()
	param.parseJson(data)
	data = json().nodeStr(param)
	message = Cf.val('{"type":', Cf.jsValue(type), ', "data":', data, '}')
	if( client) {
		print("@@ websocketSendMessage message: $message")
		client.sendWs(message)
	} else {
		return print("@@ websocketSendMessage client 미정의 (메시지:$message)");
	}
}
@baro.websocketHandshake(client, &data) {	   
	header=data.findPos("\r\n\r\n");
	print("websocket handshake header==$header");
	while( header.valid(), n, 0 ) {
		line=header.findPos("\r\n");
		if( n.eq(0) ) {
			client.set("reqInfo", line.trim() );
		} else {
			key=line.findPos(':').trim();
			value=line.trim();
			client.set(key, value);
		}
	}
	rst='';
	key=client.get('Sec-WebSocket-Key');
	if( key ) {
		key.add('258EAFA5-E914-47DA-95CA-C5AB0DC85B11');
		accept=Cf.handshakeKey(key);
		rst.add("HTTP/1.1 101 Switching Protocols\r\n");
		rst.add("Upgrade: websocket\r\n");
		rst.add("Connection: Upgrade\r\n");
		rst.add("Sec-WebSocket-Accept: ${accept}\r\n");
		// rst.add("Sec-WebSocket-Protocol: chat\r\n");
		rst.add("\r\n");
		print("@@ websocket responce==$rst, key=$key");
		client.sendData(rst);
	}
}