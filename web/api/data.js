<api>
	query(req, param, &uri) {
		dsn=param.dsn not(dsn) dsn='config'
		sql=param.sql.escape()
		db=Baro.db(dsn)
		db.fetchAll(sql,param, true)
		ss=''
		print("param=>", param) 
		while(field, param.var(fields), idx ) {
			if(idx) ss.add("\t")
			ss.add(field);
		}
		while(cur, param, idx ) {
			ss.add("\n")
			while(field, param.var(fields), idx ) {
				if(idx) ss.add("\t")
				ss.add(cur.get(field));
			}
		}
		return ss;
	}
	saveNote(req, param, &uri) {
		src=param.src.escape()
		name=param.name;
		not(name) param.error="노트명을 입력하세요"
		conf("note#xx.$name",src, true)
		param.src="note#xx.$name";
		return param;
	}
	confValue(req, param, &uri) { 
		return @util.confValue(uri.trim());
	}
	testSrcValue(req, param, &uri) { 
		code=uri.trim()
		return @util.confValue("test#src.$code");
	}
	testSrcList(req, param, &uri) { 
		Baro.db('config').fetchAll("select cd as name, tm, regdt from conf_info where grp='test#src' and data!=''", param)
		return param;
	}
	srcList(req, param, &uri) { 
		Baro.db('config').fetchAll("select cd as name, tm, regdt from conf_info where grp like 'src#%' and data!=''", param)
		return param;
	}
	templateList(req, param, &uri) {
		db=Baro.db('config')
		db.fetchAll("select cd from conf_info where grp like 'xtemplate%'", param)
		return param;
	}
	treedata(req,param,&uri) {
		name=uri.trim();
		src=conf("treedata.$name")
		@data.makeTree(param, src)
		param.type='root';
		return param;
	} 
	fileChunkStart(req, param, &uri, data) {
		param.parseJson(data)
		param.inject(tag, fid, filePath, savePath, totalCount)
		not(typeof(totalCount,'num')) return print("file chunk start error [totalCount not defined]");
		path=System.path()
		map=object("map.filechunk")
		infoPath="${path}/data/filechunkInfo/${fid}.inf"
		fileWrite(infoPath, data)
		hash=Baro.file().fileHash(infoPath)
		node=map.addNode(hash).with(infoPath, tag, fid, filePath, savePath, totalCount)
		while(idx=0, idx<totalCount, idx++) {
			node.addNode().with(idx)
		}
		node.startTick=System.tick()
		param.fileHash=hash
		return param
	}
	fileChunk(req, param, &uri, data) {
		idx=uri.findPos('/').trim() not(typeof(idx,'num')) return print("file chunk index error");
		hash=uri.trim()
		map=object("map.filechunk")
		not(hash) return print("file chunk hash error");
		hashNode=map.getNode(hash) not(hashNode) return print("file chunk hash node error [$hash]");
		cur=hashNode.child(idx)
		not(cur) return print("file chunk child node error [$hash $idx]");
		cur.uploadFinished=true;
		// 모두 업로드가 되었다면 chunk 정보를 저장한다
		saveCheck=true;
		while(node, hsahNode) {
			not(node.uploadFinished) {
				saveCheck=false;
				break;
			}
		}
		if(saveCheck ) {
			savePath=hashNode.savePath
			not(savePath) return print("file chunk save path error [$hash $idx]");
			not( @data.makeChunkFile(hashNode) ) return;
		}
		param.result="ok"
		return param;
	}
</api>

<func note="공통함수">
	_ts(o) { return toString(o, true) }
	_name(&s, flag) {
		a=s.findLast('/');
		b=when(a, a.right(), s);
		if(typeof(flag,'bool') && flag) return b;
		return b.findPos('.').trim();
	} 
	_json(id) {
		node=_node().parseJson(this.result)
		return when(id,node.get(id),node)
	}
</func>


<func> 
	@data.fileChunkUploadStart(fid, filePath, savePath) {
		not(fid) return print("파일청크 업로드 생성 CHUNK 식별자 미정의");
		not(isFile(filePath)) return print("파일청크 업로드 생성 파일명 오류 (경로:$filePath)");
		node=object("map.chunkUpload_${fid}")
		if(node.status=='upload') return print("파일청크 업로드 $fid 진행중");
		node.removeAll();
		node.status='start';
		not(savePath) {
			path=System.path()
			name=_name(filePath,true)
			savePath="$path/data/upload/$name"
		}
		not( @data.fileChunk(filePath, node, chunkSize) ) return print("파일청크 업로드 생성오류 (경로:$filePath)");
		tag='chunk-root'
		chunkSize=1048576
		totalCount=node.childCount();
		node.with(tag, fid, filePath, savePath, totalCount)
		web=Baro.web("${fid}_start")
		web.target=node;
		web.result='';
		web.data=@json.nodeStr(node)
		header=web.addNode('@header')
		header.set('Content-Type', 'application/data')
		web.call("http://localhost/api/data/fileChunkStart", "POST", func(type, data) {
			switch(type) {
			case read: this.appendText('result', data)
			case finish:
				target=this.target
				target.fileHash=_json("fileHash")
				print("[finish] fileChunkStart target==$target");
			case error: this.target.error=data
			}
		});
		print("파일청크 업로드 생성완료")
		// @data.fileChunkUpload(node);
		return true;
	}
	@data.fileChunkUpload(node) { 
		not(typeof(param,'node')) return print("파일청크 업로드 root node error");
		node.inject(tag, fid, totalCount, fileHash, status )
		if(status=='finished') return;
		not(fileHash) return print("파일청크 업로드 파일해시오류");
		total=node.childCount();
		if(tag != 'chunk-root' ) return print("파일청크 업로드 root node tag 오류 [node:${node}]");
		if(total != totalCount ) return print("파일청크 업로드 total count error [${total} == ${node.totalCount}]");
		uploadNode=null; 
		while(cur, node) {
			if(cur.error) return print("파일청크 업로드 오청오류 [idx:$idx]");
			if(cur.checkFinished) continue;
			uploadNode=cur;
			break;
		}
		if( uploadNode ) {
			node.status='upload';
			web=Baro.web("${fid}_upload") 
			web.startTick=System.tick()
			web.data=uploadNode.data
			web.target=uploadNode
			web.result=''
			header=web.addNode('@header')
			header.set('Content-Type', 'application/data')
			idx=uploadNode.index();
			web.call("http://localhost/api/data/fileChunk/$idx/$fileHash", "POST", func(type, data) {
				switch(type) {
				case read: this.appendText('result', data)
				case finish:
					target=this.target;
					target.checkFinished=true;
					print("[finish] file chunk", this.url, target.parentNode() ) 
					@data.fileChunkUpload(target.parentNode());
				case error: this.target.error=data
				}
			});
		} else {
			node.status='finish';
		}
		return node;
	}
	@data.fileChunk(path, node, chunkSize) {
		// 파일을 chunk 단위로 나눠서 노드에 저장한다
		not(node) node=_node()
		not(chunkSize) chunkSize=1048576
		fo=Baro.file('api')
		not(fo.open(path,'read')) return print("file chunk error [path:$path]")
		not(node.tag) node.tag='chunk-root'
		not(node.filePath) node.filePath=path;
		fsize=fo.size()
		last=fsize/chunkSize
		cnt=last+1
		while(n=0, n<cnt, n++) {
			cur=node.addNode() 
			if(n==last) {
				cur.data=fo.read(fsize)
			}	else {
				cur.data=fo.read(chunkSize)
			}
			cur.sendFlag=false;
			fsize-=chunkSize
		}
		fo.close()
		return node
	}
	@data.makeChunkFile(path, node) {
		fo=Baro.file('api')
		not(typeof(node,'node')) return print("file make chunk node error [path:$path]")
		not(fo.open(path,'append')) return print("file make chunk error [path:$path]")
		while(c, node) {
			fo.append(c.data)
		}
		fo.close();
		return true;
	}
	
</api>
