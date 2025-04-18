<func>
	@json.data(node, fields, listCode, skip) {
		rst='';
		not( listCode ) listCode='children';
		if( fields ) {
			if( typeof(fields,'array') ) {
				fa=fields;
			} else {
				fa=fields.split(',');
			}
		}
		if( skip ) {
			rst.add("[");
		} else {
			rst.add("{");
			num=0;
			while( key, node.keys() ) {
				val=node.get(key);
				if(typeof(val,'null','func')) continue;
				if( num ) rst.add(',') 
				v=cur.get(key)
				if(typeof(val,'node')) {
					val=@json.nodeStr(v)
				} else if(typeof(val,'array')) {
					val=@json.arrayStr(v)
				} else if(typeof(v,num) || v.eq('true','false','null')) {
					val=v
				} else {
					val=Cf.jsValue(v)
				}
				rst.add(Cf.jsValue(key),':',val)
				num++;
			}
			if( typeof(fa,'array') ) {
				if( num ) rst.add(',');
				rst.add('"fields":[')
				while( key, fa, idx, 0 ) {
					if( idx ) rst.add(',');
					rst.add( Cf.jsValue(key) );
				}
				rst.add("]");
				num++;
			} 
			not( listCode ) {
				rst.add('}')
				return rst;
			} 
			if(node.childCount()) {
				if(num) rst.add(',');
				rst.add(' "',listCode,'":[');
			}
		}
		while( cur, node, row ) {
			if( row ) rst.add(','); 
			rst.add("{");
			col=0;
			while( key, cur.keys() ) {
				if( col ) rst.add(",")
				v=cur.get(key)
				if(typeof(val,'node')) {
					val=@json.nodeStr(v)
				} else if(typeof(val,'array')) {
					val=@json.arrayStr(v)
				} else if(typeof(v,num) || v.eq('true','false','null')) {
					val=v
				} else {
					val=Cf.jsValue(v)
				}
				rst.add(Cf.jsValue(key),':',val)
				col++;
			} 
			if( cur.childCount() ) { 
				if( col ) rst.add(",");
				data=@json.data(cur, fa, listCode, true)
				rst.add(' "',listCode,'":', data);
			}
			rst.add("}");
		}
		if(skip) {
			rst.add("]");		
		} else {
			rst.add("]}");
		}
		return rst;
	}
	parseFolderList(fo, path, param, depth, pathLen ) {
		not(depth) depth=1
		if(depth.gt(3)) return;
		chkRoot=false
		not(pathLen ) {
			pathLen=path.size()
			chkRoot=true
		}
		fo.list(path,func(info) {
			while(info.next()) {
				info.inject(type, name, fullPath, createDt)
				if(type.eq('file')) continue;
				if(name.eq('.','..')) continue
				if(chkRoot) {
					if(name.start('Program Files')) continue;
					if(name.eq('Windows')) continue;
				}
				relative=fullPath.value(pathLen)
				childFolderCount=getFolderCount(fullPath)
				cur=param.addNode().with(relative, name, childFolderCount, createDt)
				parseFolderList(fo, fullPath, cur, depth+1, pathLen)
			}
		})
		return param;
	}
	getFolderCount(path, type) {
		not(isFolder(path)) return 0
		fo=Baro.file("fs");
		fo.var(filter,'folders')
		cnt=0;
		fo.list(path,func(info) {
			while(info.next()) {
				info.inject(type, name)
				if(name.eq('.','..')) continue
				cnt.incr()
			}
		})
		return cnt;
	}
</func>
<api>
	funcList(req,param,&uri) {
		map=object('@inc.userFunc')
		useFunc=param.addNode().with(name:'사용자함수')
		while(name, map.keys()) {
			path=map.get(name)
			param.addNode().with(name, name)
		}
		return param;
	}
	folderList(req,param,&uri) {
		depth=uri.findPos('/').trim()
		drive=uri.findPos('/').trim()
		relative=uri.trim()
		path="$drive:/$relative";
		fo=Baro.file('folders')
		fo.var(filter,'folders')
		fo.var(sort, 'name')
		parseFolderList(fo, path, param)
		return @json.data(param);
	}
	currentFolder(req,param,&uri) {
		drive=uri.findPos('/').trim()
		fo=Baro.file('folders')
		fo.var(filter,'folders')
		fo.var(sort, 'name')
		path="$drive:"
		pathLen=page.size()
		parseFolderList(fo, path, param, 0, pathLen)
		p=param;
		print("start $path $uri $param", param.child(0))
		while(c,p) print(">>$c");
		while(uri.valid()) {
			name=uri.findPos('/').trim()
			sub=findField(p,'name',name)
			print("current folder",name, p, sub)
			not(sub) {
				path="${drive}:${p.relative}/${name}"
				print("=====> $path 시작")
				parseFolderList(fo, path, p, 0, pathLen)
				sub=findField(p,'name',name)
				not(sub) return print("$path 경로가 없습니다");
			}
			p=sub
			p.expand=true
		}
		return param;
	}
	funcSrc(req,param,&uri) {
		err=""
		fnm=uri.findPos('/').trim()
		path=object('@inc.userFunc').get(fnm)
		not(path) err="$fnm 함수 미정의"
		if(err) {
			param.error=err
		} else {
			param.source=parse(fileRead(path))
		}
		return param;
		
		parse=func(&s) {
			while(s.valid()) {
				s.findPos(fnm)
				c=s.ch()
				not(c) break;
				if(c.eq('(')) {
					s.match()
					if(s.ch('{')) {
						return s.match(1)
					}
				}
			}
			return;
		};
	}
</api>