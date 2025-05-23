<script>
	apiController(req, param, service, uri) {
		was=Cf.rootNode("_node.was.object")
		param.set('@uri', uri)
		bound=req.getValue('boundary');
		buffer=null;
		if(bound) {
			was.parseReqParam( req, param, bound );
		} else {
			buffer=req.readBuffer();
			if(buffer) {
				type=req.getValue('Content-Type');
				if(type.eq('application/data')) {
					print("type == application/data")
				} else if(type.eq('application/xml')) {
					parseXml(buffer);
				} else {
					param.parseJson(buffer);
				}
			}
		}
		data=''
		uri=param.ref('@uri')
		if( uri.find('/') ) {
			name=uri.findPos('/').trim();
			if(was.isValid("@api/$service/$name")) {
				objectId = "$service/$name"
				fileName = "api/$service/$name" 
				name = uri.findPos('/').trim()
			} else {
				objectId = service
				fileName = "api/$service"
			}
			data=uri.trim()
		} else {
			if(was.isValid("@api/$service")) {
				objectId = service
				fileName = "api/$service"
			} else {
				objectId = 'api'
				fileName = 'api/api'
			}
			name=uri.trim()
		}
		serviceNode=Cf.getObject("api", objectId, true)
		not( serviceNode.isValid('lastModifyTm') ) { 
			path = conf('web.rootPath')
			fullPath="${path}/${fileName}.js"
			print("service:$service [serviceNode=$serviceNode fullPath == $fullPath]")
			modifyTm = Baro.file('api').modifyDate(fullPath)
			serviceNode.set('lastModifyTm', modifyTm)
			was.addServiceFunc(serviceNode, fileRead(fullPath))
		}
		Cf.error(true)
		fc=serviceNode.val(name)
		if(typeof(fc,'func')) {
			result = fc(req, param, data, buffer)
			not(result) result = param
		} else {
			result = "$objectId 서비스에 $name 함수미정의"
		}
		if( param.var(sendCheck)) return;
		if( typeof(result,'node')) {
			if(result.isValid('@treeMode')) {
				req.send(json().listData(result) )
			} else {
				req.send(jsonData(result) )
			}
		} else {
			req.send(result)
		} 
	}
	webFileFilter(req, page) {
		param=req.param();
		if(page) {
			req.sendPage(page, true);
		} else {
			req.send("webFileFilter page:${this}\nREQUEST: ${req.get()}\nPARAM: ${param}");
		}
		return true;
	}
	jsonData(node, fields, listCode) {
		fa=null
		not(listCode ) listCode='children'
		not(fields) fields=node.var(fields)
		if(fields) {
			if( typeof(fields,'array') ) {
				fa=fields;
			} else {
				fa=fields.split(',');
			}
		}
		not(typeof(fa,'array')) {
			cur = node.child(0)
			if(cur) {
				fa=_arr()
				while(key, cur.keys()) {
					if(key.ch('@')) continue;
					fa.add(key)
				}
			}
		}
		num=0, rst='{'
		while( key, node.keys() ) {
			if( key.ch('@') ) continue;
			val=node.get(key);
			if(typeof(val,'node','array')) continue;
			if( typeof(val,'num') || val.eq('true','false') ) {
				rst.add(Cf.jsValue(key), ':', val )
			} else {
				rst.add(Cf.jsValue(key), ':', Cf.jsValue(val) )
			}
			num++;
		}
		if(num) rst.add(',')
		rst.add(' "',listCode,'":[')
		while( cur, node, idx ) {
			if( idx ) rst.add(',')
			rst.add("{")
			while( key, fa, col ) {
				if( col ) rst.add(",")
				val = cur.get(key)
				if( typeof(val,'num') || val.eq('true','false') ) {
					rst.add(Cf.jsValue(key), ':', val )
				} else {
					rst.add(Cf.jsValue(key), ':', Cf.jsValue(val) )
				}
			}
			rst.add("}")
		}
		rst.add("]}")
		return rst;
	}
	
</script>

<script module="@was">	
	init(mode) {
		@was=Baro.was()
		@devMode = mode
		@status = 'stay'
		@watcher = null
	}
	start(port, path) {
		not(port) port=80;
		not(path) path=Cf.val(System.path(),"/web");
		not(conf('web.rootPath')) conf('web.rootPath',path,true)
		was.start(port,path);
		@status = 'start'
		return was;
	}
	startRouter() {
		this.makeRouterMap();
		if( this.member(devMode) ) {
			this.watcherStart()
		}
	}
	close() { 
		@status = 'close'
		was.shutdown()
	} 
	webRootPath() {
		return was.get("rootPath");
	}
	clientArray() {
		return was.clientArray(true);
	}
	serviceNode(objectId, src) {
		not(objectId) objectId = 'api'
		node = Cf.getObject("api", objectId)
		if(src && node ) {
			node[$src]
		}
		return node;
	}
	service(service, &uri, param, buffer) {
		path=conf('web.rootPath')
		fo=Baro.file('api');
		vars=null;
		if(uri.find('/') ) {
			objectId=service;
			name=uri.findPos('/').trim();
			fileName="api/${service}/${name}"
			if(fo.isFile("${path}/${fileName}.js")) {
				objectId="${service}/${name}"
				name=uri.findPos('/').trim()
			} else {
				fileName="api/${service}"
			}
			vars=uri.trim()
		} else {
			fileName="api/${service}"
			if(fo.isFile("${path}/${fileName}.js")) {
				objectId = service
			} else {
				objectId='api'
				fileName='api/api'
			}
			name=uri.trim()
		} 
		fullPath="${path}/${fileName}.js"
		serviceNode=Cf.getObject("api", objectId, true);
		modifyTm=fo.modifyDate(fullPath);
		not(modifyTm.eq(serviceNode.lastModifyTm)) {
			was = was()
			not(serviceNode.var(tag)) serviceNode.var(tag, objectId)
			was.addServiceFunc(serviceNode, fileRead(fullPath));
			serviceNode.lastModifyTm=modifyTm;
		}
		fc=serviceNode.get(name)
		if(typeof(fc,'func')) {
			return fc(req, param, vars, buffer)
		} else {
			// param.error="$fullPath 파일에 $name 함수미정의"
			print("$fullPath 파일에 $name 함수미정의")
		}
		return param;
	}
	addServiceFunc(serviceNode, &s) {
		ss='', src=''
		while(s.valid() ) {
			left=s.findPos('<api')
			ss.add(left)
			not(s.ch()) break;
			aa=s.findPos('</api>')
			aa.findPos('>');
			src.add(aa)
		}
		if(src) serviceNode[$src]
		if(ss.trim()) {
			tag = serviceNode.var(tag)
			if( tag ) {
				Cf.rootNode('@funcInfo').set('refName', tag)
				Cf.sourceApply(ss)
				Cf.rootNode('@funcInfo').set('refName', '')
			}
		}
	} 
	parseReqParam( req, param, bound ) {
		buf=req.readBuffer();
		contentLength=req.getValue("Content-Length");
		not(bound) bound=req.getValue('boundary');
		boundCheck=Cf.val('--',bound);
		print(">> multipartParse boundCheck: $boundCheck", contentLength, buf.size() );
		buf.findPos(boundCheck);
		while( buf.valid(), n, 0 ) {
			left=buf.findPos(boundCheck);
			header=left.findPos("\r\n\r\n");
			not(header.ch()) break; 
			node=this.multipartHeader(header);
			node.inject(name, filename);
			not( name ) continue;
			contentType=node.get("Content-Type");
			print("this.parseReqParam node=>$node");
			if(filename) {
				param[$name]=filename;
				data=left.findLast("\r\n");
				uploadPath=Cf.val(System.path(), "/data/temp/$filename");
				print(">> upload $name=$filename PATH:$uploadPath", contentType, data.size() );
				writeFile(uploadPath, data);
				param.var(uploadFilePath, uploadPath);
			} else {
				param[$name]=left.trim();
			} 
		}
		return param;
	}

	multipartHeader(&s) {
		node=_node();
		while( s.valid(), n, 0 ) {
			not( s.ch() ) break;
			line=s.findPos("\n");
			type=line.findPos(":").trim();
			not( type ) break;
			kind=line.findPos(";").trim();
			node.set(type, kind );
			not( line.ch() ) continue;
			while( line.valid() ) {
				key=line.findPos("=").trim();
				c=line.ch();
				if( c.eq() ) {
					val=line.match();
				} else {
					val=line.findPos(";").trim();
				}
				node.set(key, val);
				if( line.ch(';') ) line.incr();
			}
		}
		return node;
	}
	uploadSave(data) {
		use(param) not(param) return print("upload save error : param not define");
		param.inject(fnm, fkey, uploadPath);
		num=lpad(param.fidx), size=data.size();
		not( uploadPath ) {
			uploadPath="data/upload";
		}
		path=wasRoot();
		fileName=Cf.val(path,'/',uploadPath,'/', fkey,'#',num,'-',fnm);
		Baro.file("upload").writeAll(fileName, data );
		print(">> upload idx:$num  size:$size", fileName);
	}
	uploadFileCopy(uploadKey, savePath, fileName) {
		fo=Baro.file('upload');
		not(fo.isFolder(savePath)) fo.mkdir(savePath, true);
		destFullName="$savePath/$fileName";
		if(fo.isFile(destFullName)) fo.delete(destFullName);
		foDest=Baro.file("dest");
		not(foDest.open(destFullName, "append")) return print("업로드 파일복사 오류 키:$uploadKey 파일명:$destFullName");
		uploadPath=Cf.val(System.path(), "/data/temp/$filename");
		fo.var(sort,'type,name')
		fo.var(nameFilter, "${uploadKey}-*.*");
		fo.list(uploadPath, function(info) {
			while(info.next()) {
				info.inject(type, name, fullPath);
				foDest.append( fullPath, true );
			}
		})
		foDest.close();	
	}
	
	makeRouterMap(path, pathLen) {
		fo=Baro.file()
		map=Baro.was().urlMap()
		not(path ) {
			path=Cf.val(webRoot(),'/router')
		}
		not(pathLen) {
			global("routerPath",path)
			pathLen=path.size()
		}
		self=this
		print("route path==$path")
		fo.list(path, function(info) {
			while(info.next()) {
				info.inject(type, name, fullPath, modifyDt) 
				if( type=='folder') {
					if(name.ch('.')) continue;
					if(name.eq('assets','images','css','js','test','temp') ) continue;
					self.makeRouterMap(fullPath, pathLen)
					continue;
				}
				fnm=fullPath.value(pathLen)
				relative=fnm.findLast('.').trim()
				not(self.modifyCheck(fullPath, relative, modifyDt)) {
					continue
				}
				if(relative=='/main') {
					uri="/"
				} else {
					uri=relative
				}
			
				name=relative.value(1)
				if(name.find('/')) {
					name=name.replace('/','#')
				}
				chk=call(#[
					@RoutePage.${name}(req,param,&uri) {
						fullPath='${fullPath}'
						req.send(fileRead(fullPath));
					}
				]);
				if(chk ) {
					map[$uri]=call("@RoutePage.${name}")
				}
			}
		})
	}
	
	modifyCheck(fullPath, relative, modifyDt) {
		not(fullPath) return false;
		not(relative) relative=fullPath
		map = object('@comm.fileCheckMap')
		cur=map.get(relative)
		not(cur) {
			cur=map.addNode(relative);
			cur.with(fullPath, relative, modifyDt)
			print("modify check new item => ", cur)
			return true;
		}
		if(modifyDt.eq(cur.modifyDt)) return false
		return true;
	}
	watcherStart() {
		if( this.member(watcher) ) {
			print("웹라우팅 폴더감시가 이미 실행중입니다")
			return;
		}
		path=conf("was.routerPath")
		not(path) {
			path = this.webRootPath()
			path.add('/router')
			conf("was.routerPath", path, true)
		}
		@watcher=System.watcherFile(path, this, this.watcherProc)
	}
	watcherProc(act,name) { 
		n=watcher.get(name)
		if(n>0) {
			dist=System.tick()-n
			if(dist<500) return;
		}
		watcher.set(name, System.tick());
		if(act==1) {
			path=global("routerPath")
			print("$name 파일 생성 웹페이지맵 새로고침")
			this.makeRouterMap(path);
		} else {
			print("파일변경 $act $name");
		}
	}
</script>
 
<text id="web.accessControl">
	Access-Control-Allow-Origin: http://localhost:8080
	Access-Control-Allow-Credentials: true
	Access-Control-Max-Age: 86400
	Access-Control-Allow-Headers: Accept, Content-Type, X-PINGOTHER
	Access-Control-Allow-Method: GET, POST, PUT, OPTIONS
</text>
 


