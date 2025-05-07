<api>
	emoji(req, param, &data) { 		
		name = data.findPos('/').trim()
		ext = name.findLast('.').right().lower()
		metaName = "emoji-$ext"
		root = Cf.rootNode()
		metaNodes = root.val('_node.metaNodes')
		mapTypes = root.val('@contentType')
		map=metaNodes.val(metaName)
		not(map) return print("$metaName 메타정보 미정의 (name:$name, $ext)", metaNodes, param);
		type=mapTypes.val(ext)
		if(type) {
			req.setValue('@checkBlob', true)
			req.setValue('@contentType', type)
		}
		cur = map.val(name) not(typeof(cur,'node')) return print("$metaName $name 메타 파일정보 찾기오류")
		cur.inject(offset, size)
		end = offset + size;
		buf = map.ref('dataSource')
		if(ext.eq('svg')) {
			data='<?xml version="1.0" encoding="UTF-8"?>'
		} else {
			req.send(buf.value(offset,end))
		}
		param.set("@sendCheck", true)
		return;
	}
	iconList(req, param, &data) {
		filter = ''
		offset = data.findPos('/').trim()
		if(offset) size = data.findPos('/').trim()
		if(size) filter = "limit $offset,$size"
			
		db=Baro.db('config')
		node=db.fetchAll("select cd, data from conf_info where grp='svg' $filter", param)
		node.var(fields,"name,props,svg")
		while(cur, node) {
			data = cur.ref('data')
			cur.name = cur.cd
			cur.props = data.findPos("\n")
			cur.svg = data
		}
		return node;
	}
	
	emojiNames(req, param) {
		print("emoijNames => $param")
		map = _node('emoji.names') if(map.childCount()) return map;		
		path = conf('web.rootPath')
		css = "$path/css/emoji-api.css"
		s=fileRead(css) s.ref()
		while(s.valid()) {
			left = s.findPos('{',0,1)
			not(s.ch()) break;
			s.match()
			if(left.find(',')) continue;
			if(left.find('svg')) continue;
			c=left.ch()
			while(c.eq('/')) {
				c=left.ch(1)
				if(c.eq('/')) left.findPos("\n")
				else left.match()
				c=left.ch()
			}
			if(c.eq('.')) {
				left.incr()
				if(left.find('.')) continue;
				name = left.trim()
				map.addNode().with(name)
			}
		}
		return map;
	}
</api>