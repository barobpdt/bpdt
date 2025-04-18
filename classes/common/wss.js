<script module="@wss">	 
	init(port) {
		@server=Baro.server('websocket')
		@port = nvl(port, 8092)
		server.var(type, 'websocket')
		this.start()
	}
	start() {
		this.var(useClass, true)
		server.start(port, this, this.acceptClient)
		server.callbackClient(this, this.websocketClient)
	}
	acceptClient(client) {
		print("@@ web socket server accept clinet", clinet)
	}
	websocketClient(client, type) {
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
			    socketHandshake( client, data );
			} else {
				data=client.readWs()
				if(data) {
					this.socketMessageProc(client, data);
				}
			}
		default:
			client.close();
		}
	}
	
	socketMessageProc(client, &data) {
		config=client.config()
		line0=data.findPos("\r\n");
		line1=data.findPos("\r\n\r\n");
		if( line0.ch('@')) {
			line0.incr();
		} else {
			config.errorCode="400"
			print("웹소켓 헤더오류 ",config.recvRemainSize );
			return
		}
		type		= line0.findPos('::').trim();
		header		= line0.trim();
		
		size		= line1.findPos('::').trim()
		contentType	= line1.findPos('::').trim()
		info		= line1.trim();
		
		print("@@ 웹소켓 메시지처리 시작 $type ==>  $header", data.size(), size, contentType )
		if( size > data.size() ) {
			/*
			config.recvRemainSize = size - data.size();
			config.recvData=data;
			config.with(type, header, contentType)
			return;
			*/
			print("@@ 소켓 요청 크기오류 [$type : $header]", type, header, contentType, size, config.recvRemainSize);
		}
		this.socketMessageApply(client, type, header, contentType, data, info)
	}
	socketMessageApply(client, type, &header, contentType, &data, &info) {
		print("socketMessageApply", type, header, data.size() )	
		if(type.start('req_')) {
			requestType = type
		} else {
			requestType = "req_$type"
		}
		resultContentType = contentType
		resultInfo = info.trim()
		param=null
		fc = this.get(requestType)
		if(contentType.eq('json')) {
			param =_node()
			param.parseJson(data)
		}
		print("@@ socketMessageApply", type, requestType, fc )	
		if(typeof(fc,'func')) {
			arr = args().reuse()
			arr.add(client, requestType, header, resultInfo, resultContentType, data, param )
			return call(fc, this, arr)
		}
		errorCode = 200
		responseData = ''
		if(type=='connection') {
			type = 'req_login' 
		} else {
			errorCode = 400
			resultType = contentType
			responseData = data
		}
		not(type) return;
		size=responseData.size();
		client.sendWs("@${type}::${errorCode}\r\n${size}::${requestType}::${resultContentType}::${resultInfo}\r\n\r\n${responseData}")
	}
	req_login(client, requestType, &header, resultInfo, resultContentType, data, param) {
		type='res_login'
		resultContentType='text'
		responseData='login ok'
		size=responseData.size();
		client.sendWs("@${type}::${errorCode}\r\n${size}::${requestType}::${resultContentType}::${resultInfo}\r\n\r\n${responseData}")		
	}
	req_api(client, requestType, &header, resultInfo, resultContentType, data, param) {
		type='res_api'
		service=header.findPos("/").trim()
		uri=header.trim()
		param=_node()
		result=this.apiService(service, uri, param, buffer);
		not(result) result=param;
		err=Cf.error();
		if(err) {
			errCode = 500
			if(typeof(result,'node')) result.error=err;
		} else {
			errorCode = 200
		}
		resultInfo = uri
		if( typeof(result,'node')) {
			if(result.var(checkSend)) return;
			responseData=@json.listData(result)
			resultContentType='json'
		} else {
			responseData=result.trim()
			resultContentType='text'
		}
		size=responseData.size();
		client.sendWs("@${type}::${errorCode}\r\n${size}::${requestType}::${resultContentType}::${resultInfo}\r\n\r\n${responseData}")
	}
	req_chunkFileUpload(client, requestType, &header, resultInfo, resultContentType, data, param) {
		type='res_imageSend'
		errorCode = 200
		user=header.findPos("/").trim()
		name=header.trim()
		savePath="data/clipboard_captures/$name";
		fileWrite(savePath, data);
		resultContentType='file'
		responseData=savePath
		size=responseData.size();
		client.sendWs("@${type}::${errorCode}\r\n${size}::${requestType}::${resultContentType}::${resultInfo}\r\n\r\n${responseData}")
	
	}
	apiService(service, &uri, param, buffer) {
		return was().service(service, &uri, param, buffer)
	}
</script>