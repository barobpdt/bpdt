<api>
	drives(req, params, &uri) {
		print("drives >> ", param, uri) 
		while(path, System.driveList() ) {
			name = path.ch()
			params.addNode().with(name,path)
		}
		return params;
	}
	folders(req, params, &uri) {
		path = params.path
		root= params.addNode()		
		root.id = 'K0'
		root.parent = '#'
		root.path = path
		root.text = path
		root.type = 'folder'
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
	folderTree(path, root, depth) {
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
	folderList(path, root, parentId, depth, idxNum) {
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
	fetchTreeChild(path, pid, root, cur) {
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