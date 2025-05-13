<func note="interface function">
	proxyController(req, param, &url, proxy ) {
		not(url.start('/proxy/',true)) return @proxy.sendError(req, param, "proxy client start error [URL:$url]");
		uid=url.findPos('/').trim();
		client=proxy.get(uid) not(client) return @proxy.sendError(req, param, "proxy client connect error [ID:$uid URL:$url]");
		post=req.postData();
		size=post.size();
		packet="@@>apiCall|$size|$url\n"
		packet.add(post);
		data=client.sendProxy(req, packet);

	}
	startProxyServer() {
		server=Baro.was("proxy")
		server.startServer(8093, @proxy.serverAccept, "proxy", 2500);
	}
</func>

<func note="proxy util function">
	@proxy.sendError(req, param, msg) {
		param.error=msg;
		req.send(@json.listData(param));
	}
</func>

<func note="proxy server function">

	@proxy.serverAccept(client, proxy) {
		data=client.recvData();
		print("proxy accept data==$data", client, proxy);
		data.ref();
		data.findPos('@@>');
		line=data.findPos("\n");
		type=line.findPos('|').trim();
		size=line.findPos('|').trim();
		print("$type => size:$size");
		if(type=='login') {
			uid=data.trim();
			if(proxy.get(uid)) {
				prev=proxy.get(uid);
				print("prev==", prev.getValue());
				if(prev==client) {

				} else {
					proxy.set(uid, client);
				}
			} else {
				print("login ok UID:$uid");
				client.setValue('@uid', uid);
				proxy.set(uid, client);
			}
		}
		msg='ok';
		size=msg.size();
		packet="@@>$type|$size\n$msg";
		client.sendData(packet);
	}
</func>

<func note="proxy client function">
	@proxy.clientStart(deviceId, ip, port, timeout) {
		not(deviceId) return print("장치아이디를 등록하세요");
		not(ip) ip='58.230.162.173';
		not(port) port=8093;
		not(timeout) timeout=2500;
		socket=Baro.socket('proxy');
		worker=Baro.worker('proxy');
		worker.with(ip,port,timeout,socket, deviceId);
		worker.start(@proxy.clientProc, true, 100)
		return worker;
	}
	@proxy.clientProc() {
		this.inject(socket);
		print("worker dispatch socket==$socket");
		not(socket) return System.sleep(1000);
		not(socket.isConnect()) {
			this.inject(ip,port,timeout,deviceId);
			if(socket.connect(ip,port,timeout)) {
				tm=System.localtime();
				size=deviceId.size();
				if( socket.sendData("@@>login|$size|$tm\n$deviceId") ) {
					result=socket.recvData();
					print("connect result == $result");
				}
			}
			not(socket.isConnect()) return;
		}
		if(socket.isRead(500) ) {
			data=socket.recvData();
			if(typeof(data,'bool')) {
				print("proxy r`ead data error", this);
				return;
			}
			@proxy.clientParse(socket, data);
		}
	}
	@proxy.clientParse(socket, &data) {
		data.findPos('@@>');
		line=data.findPos("\n");
		type=line.findPos('|').trim();
		size=line.findPos('|').trim();
		param=this.addNode("param").removeAll(true);
		result=null;
		if(type=='apiCall') {
			uri=line;
			if(uri.ch('/')) uri.incr();
			service=uri.findPos('/').trim();
			name=null, vars=null;
			if(uri.find('/') ) {
				name=uri.findPos('/').trim();
				fileName="pages/web/api/${service}.${name}.js";
				if(isFile(fileName)) {
					name=uri.findPos('/').trim();
				} else {
					fileName="pages/web/api/${service}.js";
				}
				vars=uri.trim();
			} else {
				name=uri.trim();
				fileName="pages/web/api/${service}.js";
			}
			ch=data.ch();
			if(ch.eq('[','{')) {
				param.paseJson(data);
			}
			service=object("api.${service}");
			fc=service.get(name);
			if(typeof(fc,'function')) {
				result=fc(socket, param, vars);
			}
			not(result) {
				result=param;
				result.error="$service $name API호출 오류(URI:$line)";
			}
		} else {
			result=param;
			result.error="PROXY 타입오류 (타입:$type)";
		}
		if( typeof(result,'node')) {
			if(result.var(checkSend)) return;
			socket.send(@json.listData(result) );
		} else {
			socket.send(result);
		}
	}
</func>
