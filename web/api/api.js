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
		id=nvl(params.id, 0)
		root.set('id',when(id.eq(0),'#',id))
		root.set('idx', id+1)
		return @api.folderList(params.path, params)
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
	folderList(path, root, parentId, depth) {
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
				cur=root.addNode()
				// { 'id': 't1', 'parent': '#', 'text': 'New Folder 1', 'type': 'folder' },
				cur.id=root.incrNum('idx')
				cur.text = name
				cur.type = type
				cur.parent = parentId
				cur.fullPath = fullPath
				cur.depth = depth
				if(depth<3) {
					@api.folderList(fullPath, root, cur.id, depth)
				}
			}
		});
		return root;
	} 
</func>	