<api>
	version(req, param) {
		return '1.0.0'
	}
	drives(req, param, &uri) {
		while(path, System.driveList() ) {
			name = path.ch()
			param.addNode().with(name,path)
		}
		data=json(param,'data')
		return req.send(data);
	}
	cmdTest(req, param, &uri) {
		param.type='api'
		param.reqObject = req
		param.command='ipconfig'
		param.endCheck=false
		param.startTick = System.tick()
		addCmdWorker('test', param, func(result,param) {
			req=param.reqObject
			if(req) {
				req.send(result)
			}
			param.endCheck=true
		})
		while(notValid(param.endCheck)) {
			dist=System.tick()-param.startTick;
			if(dist>10000) {
				print("@@ API cmdTest timeout ($dist ms)")
				break;
			}
			System.sleep(100)
		}
		return;
	}
	folders(req, params, &uri) {
		path = params.path
		root= params.addNode()		
		root.id = 'K0'
		root.parent = '#'
		root.path = path
		root.text = path
		root.type = 'folder'
		print("root==>$root", param)
		params.set('idx',1)
		return @api.folderList(path, params, root.id)
	}
	fetchTreeChild(req, params, &uri, &data) {
		params.parseJson(data)
		print('@@ fetchTreeChild', params, data)
		result = _node()
		while(node, params.children, n) {			
			node.inject(id, pid, fullPath)
			cur=result.addNode().with(pid, fullPath)
			@api.fetchTreeChild(fullPath, pid, params, cur)			
		}
		return result;
	}
</api>

<func>
	@api.folderTree(path, root, depth) {
		not(root) root = _node('listFolder').removeAll();
		not(depth) depth = 0;
		fo=Baro.file()
		fo.var(sort,'name, case')
		depth++;
		fo.list(path, func(info) {
			while(info.next()) {
				info.inject(type,name, fullPath)
				if(type.eq('file')) continue;
				if(name.eq('Windows', 'Program Files','Program Files (x86)')) continue;
				cur=root.addNode().with(type,name,fullPath)
				if(depth<3) {
					@api.listFolder(fullPath, cur, depth)
				}
			}
		})
		return root;
	}
	@api.folderList(path, root, parentId, depth, idxNum) {
		not(parentId) return;
		not(depth) depth = 0
		not(idxNum) idxNum = 0
		fo=Baro.file()
		fo.var(sort,'name, case')
		depth++;
		idxNum++;
		fo.list(path, func(info) {
			while(info.next()) {
				info.inject(type,name, fullPath)
				if(type.eq('file')) continue;
				if(name.eq('Windows','Users','Program Files','Program Files (x86)')) continue; 
				cur=root.addNode()
				num = root.incrNum('idx')
				cur.id="K$num"
				cur.text = name
				cur.type = type
				cur.parent = parentId
				cur.fullPath = fullPath
				cur.depth = depth
				if( idxNum<3 ) {
					cur.checkChild=true
					@api.folderList(fullPath, root, cur.id, depth, idxNum)
				}
			}
		});
		return root;
	} 
	@api.fetchTreeChild(path, pid, root, cur) {
		fo=Baro.file()
		fo.var(sort,'name, case')
		fo.list(path, func(info) {
			while(info.next()) {
				info.inject(type,name,fullPath)
				if(type.eq('file')) continue;
				if(name.eq('Windows', 'Program Files','Program Files (x86)')) continue; 
				sub = cur.addNode()
				num = root.incrNum('idx')
				sub.id="K$num"
				sub.text = name
				sub.type = type
				sub.parent = parentId
				sub.fullPath = fullPath
				sub.depth = depth
			}
		});
		return root;
	} 
</func>	