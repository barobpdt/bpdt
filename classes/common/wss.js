<script>
	socketHandshake(client, &data) {	   
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
</script>
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
	sendOk(type, resultType, data) {
		size = data.size()
		client.sendWs("@${type}::200\r\n${size}::callback::${resultType}::1.0\r\n\r\n${data}")
	}
	socketMessageProc(client, &data) {
		config=client.config()
		line0=data.findPos("\r\n");
		line1=data.findPos("\r\n\r\n");
		if( line0.ch('@')) {
			line0.incr();
		} else {
			config.errorCode="400"
			print("웹소켓 헤더오류 ", line0, line1 );
			return
		}
		type		= line0.findPos('::').trim();
		header		= line0.trim();
		
		size		= line1.findPos('::').trim()
		contentType	= line1.findPos('::').trim()
		info		= line1.trim();
		
		print("@@ 웹소켓 메시지처리 시작 $type ==>  $header", data.size(), size, contentType )
		if( size > data.size() ) {
			config.recvRemainSize = size - data.size();
			config.recvData=data;
			config.with(type, header, contentType)
			print("@@ 소켓 요청 크기오류 [$type : $header]", type, header, contentType, size, config.recvRemainSize);
			return;
		}
		
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
		print("@@ 웹소켓 메시지처리 정보", type, requestType, fc )	
		if(typeof(fc,'func')) {
			arr = args().reuse()
			arr.add(client, param, requestType, header, resultInfo, resultContentType, data )
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
	req_pageActive(client, param, requestType, &header, resultInfo, resultContentType, data) {
		param.inject(pageId, logId)
		if( pageId) {
			page = page(pageId)
			if( typeof(page,'widget')) {
				not(logId ) logId='webview-in'
				not(page.is('active')) logWriter(logId).write("##>activePage:")
				this.sendOk(requestType, resultContentType, data)
			}
		}
	}
	req_pageMove(client, param, requestType, &header, resultInfo, resultContentType, data) {
		page=global('webviewParent').pageNode()
		if(page) {
			if( param.move) {
				not(page.moveTick) return;
				d=System.tick()-page.moveTick;
				if(d>25000) return;
				page.geo().inject(x,y)
				dx=page.px - param.sx;
				dy=page.py - param.sy;
				print("==>$x, $y ",dx, dy, x, y)
				x-=dx;
				y-=dx;
				page.px = param.sx;
				page.py = param.sy;
			}
			if( param.start ) {
				page.moveTick = System.tick()
				page.px = param.sx;
				page.py = param.sy;
			} else {
				page.px = 0;
				page.py = 0;
				page.moveTick = 0;
			} 
		}
	}
	req_chunkFileUpload(client, param, requestType, &header, resultInfo, resultContentType, data ) {
		type='res_imageSend'
		code = 200
		path=conf('temp.path')
		header.split('|').inject(fieldId, name, size, currentIndex, currentChunk, totalChunks)
		fo=Baro.file('upload')
		fileName = "$path/upload/$fieldId--$name"
		not(fo.open(fileName, "append")) {
			code = 400
			responseData = "$fileName 파일 열기 오류");
			print("파일열기오류 (파일명: $fileName)")
		}
		if( code==200 ) {
			saveData=data.decode('base64')
			last = totalChunks - 1	
			end = currentIndex.eq(last)
			if(end) end =  size.eq(saveData.size() )
			fo.append(saveData)
			fo.close()
			responseData= fileName
		}
		size=responseData.size();
		resultContentType='text'
		client.sendWs("@${type}::${code}\r\n${size}::${requestType}::${resultContentType}::${resultInfo}\r\n\r\n${responseData}")
	}
	
	req_login(client, param, requestType, &header, resultInfo, resultContentType, data) {
		type='res_login'
		resultContentType='text'
		responseData='login ok'
		size=responseData.size();
		client.sendWs("@${type}::${errorCode}\r\n${size}::${requestType}::${resultContentType}::${resultInfo}\r\n\r\n${responseData}")		
	}
	req_api(client, param, requestType, &header, resultInfo, resultContentType, data) {
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
	
	apiService(service, &uri, param, buffer) {
		return was().service(service, &uri, param, buffer)
	}
</script>