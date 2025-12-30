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
	addCmd(req, param, &uri) {
		addCmdWorker('api','cd')
	}
	appCommand(req, param, &uri) {
		print("uri==$uri")
		type=uri.findPos('/').trim()
		line=uri.findPos('/').trim()
		
		addGlobalAppCommand("@@>$type:$line")
	}
	cmdTest(req, param, &uri) {
		command=uri.findPos('/').trim()
		not(command) command='cd'
		return @api.sendCmdResult(req,param,command)
	}
	cmdArray(req, param, &uri) {
		a=param.addArray('commandList')
		a.add('cd')
		a.add('dir')
		a.add('ipconfig')
		a.add('npm list')
		print(">> cmdTest command==$command ", param, args())
		return @api.sendCmdResult(req,param)
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
	@api.sendCmdResult(req, param, command, maxTick) {
		not(maxTick) maxTick=60000 // 60초
		param.type='cmdResult'
		param.reqObject = req		
		param.endCheck=false
		param.startTick = System.tick()
		if(command) {
			param.command=command
		}
		param.logCallback = func(result,param) {
			req=param.reqObject
			param.appendText('result', result)
			if( isValid(param.commandList) ) {
				print("수행할 명령이 남아 있습니다($param.commandList) 수행결과:$result")
				return;
			}
			if(req && req.isConnect() ) {
				req.send(param.result)
			}
			param.endCheck=true
		};
		cmd=addCmdWorker('aa', param)		
		print(">> @api.sendCmdResult param=$param", cmd)
		if(param.error) {
			return param;
		}
		while(checkTick(param.startTick,maxTick)) {
			not(req.isConnect()) {
				break;
			}
			if( isValid(param.endCheck) ) {
				break;
			}
			System.sleep(250)
		}
	}
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