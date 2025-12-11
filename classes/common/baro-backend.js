@baro.initBackend(projectFolder, mode, infoSource) {
	backend = object("baro.backend")
	not(projectFolder) {
		backend.inject(projectFolder, mode)
		not(projectFolder) return print("백앤드 소스생성오류 프로젝트폴더 미정의")
	}
	not(mode) mode='sql' // mode=[sql,mongo,drizzle]	
	fo = Baro.file()
	not(isFolder(projectFolder)) {
		fo.mkdir(projectFolder, true)
	}
	envPath=pathJoin(projectFolder,'.env')
	@baro.filePathInfo(projectFolder).inject(basePath,projectName)
	backend.removeAll(true)
	backend.with(projectFolder, mode, basePath, projectName)
	backend.backendInitTick = System.tick()
	if(mode.eq('sql','drizzle')) {
		backend.set('useNeon', true)
	} else if(mode.eq('mongo')) {
		backend.set('useMango', true)
	}
	if(isFile(envPath)) {
		backend.set('envs', fileRead(envPath) )
	}
	infoFile = pathJoin(projectFolder,"${projectName}.info.js")
	if(infoSource) { 
		fileWrite(infoFile, @baro.parseBackend(backend, backend, infoSource))
	}
	Cf.error(true)
	not(isFile(infoFile)) {
		@baro.backend_makeInfoFile(projectFolder, infoFile)
		if(Cf.error()) return print("@baro.backend_makeInfoFile 오류:", Cf.error())
	}
	// src = stripJsComment(fileRead(infoFile))
	@baro.loadBackend(backend, fileRead(infoFile), true)
	@baro.copyBackendSource(backend)
}
@baro.loadBackend(backend, &s, reset) {
	print(">> load backend $backend, info=$s")
	_isProps = func(s) {
		c=s.ch()
		return when(c.eq('{'),true);
	};
	type='', base=null, cur=null
	while(s.valid()) {
		left = s.findPos('##>')
		if(type) {
			src=left // @baro.parseBackend(backend, cur, left)
			print(">> parse backend type=====$type start")
			if(type.eq('config')) {
				@baro.configFuncVal(backend, cur, src)
			} else if(type.eq('route')) {
				@baro.parseRoute(backend, cur, src)
			} else if(type.eq('sql')) {
				@baro.sqlFuncVal(backend, cur, src)
			} else if(type.eq('table')) {
				@baro.tableFuncVal(backend, cur, src)
			}
			print("parse backend type=====$type end")
		}
		not(s.ch()) break
		type=s.move().lower()
		if(type.eq('routes')) type='route'
		else if(type.eq('tables')) type='table'
		base=backend.addNode("@$type")
		if(reset) base.removeAll(true)
		if(_isProps(s)) {
			s.ch()
			src=s.match(1)
			node=_node()			
			@baro.parseProps(backend, base, node, src,true)
			name=node.name not(name) name='default'
			cur=base.addNode(name)			
			if(node.isset('@keyArray')) {
				while(k,node.get('@keyArray')) {
					cur.set(k,node.get(k))
				}
			}
			not(cur.name) cur.name = name
		}
	}	
}
@baro.copyBackendSource(backend) {
	not(backend) backend = object('baro.backend')
	backend.inject(projectFolder)
	not(isFolder(projectFolder)) return print("백앤드 소스복사 오류 프로젝트 경로 미정의", backend)
	template = @baro.conf('backendTemplatePath')
	packageJson = pathJoin(template,'package.json')
	base=backend
	Cf.error(true)	
	save = func(path) {
		srcPath = pathJoin(template,path)
		destPath = pathJoin(projectFolder,path)
		src=@baro.parseBackend(backend,base,fileRead()) if(Cf.error()) return;
		not(src) return print()
		fileWrite(destPath, src)
	};
	table=backend.get('@table')
	route=backend.get('@route')
	if(backend.get('@modify')) {
		not(save('.env')) return;
		not(save('package.json')) return;
		not(save('src/config/env.js')) return;
		backend.set('@modify',false)
	}
	if(table.get('@modify')) {
		not(save('src/config/db.js')) return;
		table.set('@modify',false)
	}
	if(route.get('@modify')) {
		while(base, route) {
			while(sub, base) {
				not(sub.get('@modify')) return;
				sub.set('@modify',false)
			}
		}
		route.set('@modify',false)
	}
}
@baro.backend_makeInfoFile(projectFolder, infoFile) {
	fo=Baro.file()	
	srcPath = pathJoin(projectFolder,'src')
	not(isFolder(srcPath)) {
		fo.mkdir(srcPath)
		fo.mkdir(pathJoin(srcPath,'config'))
		fo.mkdir(pathJoin(srcPath,'controllers'))
		fo.mkdir(pathJoin(srcPath,'routes'))
	}
	not(isFile(infoFile)) {
		@baro.backend_makeInfoSave(infoFile)
	}
}
@baro.backend_infoVars(backend) {
	not(backend) backend = object('baro.backend')
	map=_node()
	parse = func(&s) {
		while(s.valid()) {
			s.findPos('@[')
			k=s.findPos(']').trim()
			if(typeof(k,'num')) continue;
			not(map.isset(k)) map.set(k,true)
		}
	};	
	path = @baro.conf('backendTemplatePath')
	search(path)
	search = func(path) {
		fo=Baro.file()
		fo.list(path, func(info) {
			while(info.next()) {
				info.inject(name, type, fullPath)
				if(type=='file') {
					parse(fileRead(fullPath))
				} else {
					search(fullPath)
				}
			}
		});
	};
	while(k,map.keys()) {
		nodeAppendText(backend,'vars',k,',')
	}
}
@baro.backend_makeInfoSave(infoFile) {
	nl=conf('cf.newline')
	backend = object("baro.backend")
	@baro.backend_infoVars(backend)
	
	templateInfo = pathJoin(@baro.conf('backendTemplatePath'),'info.js')
	src=@baro.parseBackend(backend,backend,fileRead(templateInfo)) 
	not(src) return print("$infoFile backend_makeInfoSave 소스오류")
	fileWrite(infoFile,src)
}
isSpace(&s) {
	c=s.ch()
	return when(c,false,true)
}
@baro.backendKeyValue(backend, node, &k, indent) {
	not(k.find('.')) {
		return when(backend.isset(k), backend.get(k), node.get(k));
	}
	nm=k.move()
	cur=backend.get("@$nm")
	not(cur) return print("$nm 타입 미정의 [백엔드 분석오류]")
	c=k.ch()
	if(c.eq('.')) {
		k.incr()
		sp=k.cur()
		nm0=k.move()
		c=k.ch()
		if(c.eq('.')) {
			k.incr()
			sub=cur.get(nm0)
			nm1=k.trim()
			not(sub) print("$nm 항목에 $nm0 을 찾을수없습니다")
			print(">> ", nm, nm0, nm1, sub)
			if( nm1 ) v=sub.get(nm1) else v=sub;			
		} else if(cur.isset(nm0)) {
			v=cur.get(nm0)
		} else {
			k.pos(sp)
			nm0=k.trim()
			while(sub, cur) {
				if(sub.isset(nm0)) {
					v=sub.get(nm0)
					break;
				}
			}
		}
	}
	return v;
}
@baro.parseBackend(backend, node, &s, addCheck) {
	Cf.error(true)
	nl=conf('cf.newline')
	ss='', skip=false, skipCnt=0, prevSize=0
	while(s.valid(),idx) {
		if(Cf.error()) return;
		left=s.findPos('@[')
		if(idx) {
			if( prevSize.ne(ss.size()) ) ss.add(nl)
		}
		prevSize=ss.size()
		not(isSpace(left)) {
			ss.add(left)
		}
		not(s.ch()) break;
		indent=''
		if(left.find("\n")) {
			indent=@baro.lastIndent(left)
		} else {
			
		}
		ok=false
		k=s.findPos(']'), def=''
		if(lineCheck(k,'||')) {
			key=k.findPos(':').trim()
			def=k.trim()
			k=key
		}
		else if(@baro.isFunc(k)) {
			v=@baro.backendFuncVal(backend, node, k, indent)
			if(v) ok=true
		} else if(k.find('.')) {
			v=@baro.backendKeyValue(backend,node, k, indent)
		} else {			
			if(node.isset(k)) {
				v=node.get(k)
				if(v) ok=true
			} else if(backend.isset(k)) {
				v=backend.get(k)
				if(v) ok=true
			}
		}
		c=s.ch()
		if(c.eq('?')) {
			s.incr()
			v=_checkValue()
			if(ok) {
				not(skip) ss.add(@baro.backendValue(backend,node,v,indent))
			}  
			if(_isElse(s)) {
				c=s.ch()
				s.start('else', true)
				c=s.ch()				
				if(s.start('@[')) {
					if(ok) {
						skipCnt++;
						skip=true
					}
					continue;
				}
				v=_checkValue()
				not(ok) {
					not(skip) ss.add(@baro.backendValue(backend,node,v,indent))
				}
			}
			if(skip) {
				skipCnt--;
				if(skipCnt==0) {
					skip=false
				}
			}
			continue;
		} 
		if(s.start('||',true)) {
			not(ok) v=_checkValue()
			if(skip) continue;
			if(v) { 
				if(addCheck) node.set(k,v)
				ss.add(@baro.backendValue(backend,node,v,indent))
			}
			continue;
		}
		if(skip) continue;
		if(ok) {
			ss.add(v)
		} else if(def) {
			ss.add(def)
		}
	}
	return ss;
	
	_checkNext = func(&s) {
		c=s.ch()
		return c.eq('<','(','{','[');
	};
	_checkFunc = func(&s) {
		c=s.ch() not(c) return;
		s.start('export',true)
		s.start('async',true)
		if( s.start('function',true) ) {
			c=s.ch()
			not(c.eq('(')) c=s.next().ch()
			if(c.eq('(')) {
				s.match()
				c=s.ch()
				if(c.eq('{')) return true;
			}
		} else if(lineCheck(s,'=')) {
			s.findPos('=')
			c=s.ch()
			if(s.start('async',true)) c=s.ch()
			if(c.eq('(')) s.match() else s.next()
			c=s.ch()
			if(s.start('=>',true)) {
				return true;
			}
		}
		return;
	};
	_checkValue = func() {
		c=s.ch() not(c) return;
		if(_checkNext(s)) {
			if(c.eq('<')) {
				v=_tagValue()
			} else {
				src=s.match()
				if(c.eq('{','[')) {
					if(c.eq('{')) v="{$src}" else v="[$src]"
				} else {
					v=src
				}
			}
		} else if(_checkFunc(s)) {
			sp=s.cur()
			s.start('export',true)
			s.start('async',true)
			if( s.start('function',true) ) {
				c=s.ch()
				if(c.eq('(')) s.match() else s.next()
				c=s.ch()
				if(c.eq('{')) s.match(1)				
			} else {
				s.findPos('=>')
				c=s.ch()
				if(c.eq('{')) s.match(1) else s.findPos("\n")
			}
			ep=s.cur()
			c=s.ch()
			if(c.eq(',',';')) {
				s.incr()
				ep=s.cur()
			}
			if(sp<ep) v=s.trim(sp,ep,true)
		} else {
			if(lineBlankCheck(s) ) {
				s.findPos("\n")
				v=s.findPos("\n")
			} else {
				s.ch()
				v=s.findPos("\n")
			}
		}
		return v;
	};
	_tagValue = func() {
		c=s.ch() not(c.eq('<')) return;
		if(s.start('<>')) {
			tag='value'
			props=''
			body=s.match('<>','</>')
		} else {
			sp=s.cur()
			tag=s.incr().move()
			s.pos(sp)
			body=s.match("<$tag","</$tag>")
			props=body.findPos('>')
		}
		if(typeof(body,'bool')) return print("@@ parseBackend $tag 태그 매칭오류")
		return body;
	};
	_isElse = func(&s) {
		c=s.ch()
		if(s.start('else')) return true;
		return false;
	};
}
@baro.backendFuncVal(backend,node,&s) {
	fnm = s.findPos('(',0,1).lower()	
	fparam = s.match()
	print(">> backendFuncVal $fnm")
	if(fnm.eq('conf')) {
		code = fparam.trim()
		return @baro.conf(code)
	}
	if( fnm.eq('fields','binds','pk','tableinfo')) {
		node = @baro.backendKeyValue(backend,node,"table.$fparam")
		return json().nodeStr(node)
	} 
	if( fnm.eq('eq')) {
		a=fparam.findPos(',').trim()
		b=fparam.findPos(',').trim()
		if(backend.isset(a)) {
			return backend.cmp(a,b)
		} else if(node.isset(a)) {
			return node.cmp(a,b)
		}
		print(">> not eq $a, $b")
		return
	} 
	if( fnm.eq('query')) {
		
	} else {
		print("@@ backendFuncVal $fnm 함수가 정의되지 않았습니다")
	}
	return;
}
@baro.parseRoute(backend, node, &s) {
	node.set('@comment', '')
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('*')) comment=s.match() else comment=s.findPos("\n");
			nodeAppendText(node,'@comment',comment,conf('cf.newline'))
			continue;
		}
		not(c.eq('[')) return print("route 시작오류 ", node, s)
		sp=s.cur()
		method = s.match().trim().lower()
		s.ch()		
		if(lineCheck(s,'{')) {
			uri = s.findPos('{',0,1).trim()
		} else {
			uri = s.findPos("\n").trim()
		}
		cur=node.addNode(uri)
		cur.with(method, uri)
		if(s.ch('{')) {
			props=s.match(1)
			@baro.parseProps(backend, node, cur, props,true)
		}
		confCode = "baro.route:${node.name}#${uri}"
		ep=funcsAppend(s)
		if(sp<ep) {
			s.pos(ep)
			if(cur.get('@funcs')!=conf(confCode) ) {
				node.set('@modify',true)
				conf(confCode, cur.get('@funcs'),true)
			}
		} else {
			break;
		}
	}
	funcsAppend = func(&s) {
		cur.set('@funcs','')
		while(s.valid()) {
			c=s.ch() not(c) return s.cur();
			if(c.eq('/')) {
				c=s.ch(1)
				if(c.eq('*')) s.match(1) else s.findPos("\n");
				continue;
			}
			not(@baro.isFunc(s)) break;
			fnm = s.findPos('(',0,1).lower()
			fparam = s.match(1)
			cur.appendText('@funcs',"${fnm}(${fparam})")
			if(c.eq(',',';')) s.incr()
		}
		return s.cur();
	};
}

@baro.routeFuncVal(backend,node,&s) {
	funcVal = func(fnm, &fparam) {
		if(fnm.eq('if','not')) {
			node.set('newline','')
			if(fnm.eq('if')) return "if($fparam) ";
			chk=''
			while(fparam.valid(),n) {
				a=fparam.move()
				chk.add("isEmpty(!$a)")
				c=fparam.ch() not(c) break;
				if(c.eq(',')) {
					chk.add('||')
				}
			}			
			return "if($chk) ";
		}
		ss=''
		node.set('newline',conf('cf.newline'))
		switch(fnm) {
		case js: 
			ss=fparam
		case auth:
			vnm = fparam.trim() not(vnm) vnm='userId'
			node.get('@varNames').add(vnm)
			ss="const {$vnm} = getAuth(req)"
		case sql:
			node.set('@bindTarget', '')
			code=fparam.move(), vnm='', binds=''
			if(fparam.ch('(')) {
				binds=fparam.match()
				if(binds.ch(':')) {
					target=binds.trim(1)
					node.set('@bindTarget', target)
				} else {				
					if(binds.find(':')) {
						left = binds.findPos(':')
						target = binds.trim()
					} else {
						left = binds
						target = 'req.body'
					}
					ss.add("const {$names} = $target", conf('cf.newline'))
				} 
			}
			fparam.ch()
			if(fparam.start('=>',true)) {
				vnm= fparam.trim()
			}
			sql = @baro.backendKeyValue(backend, node, "sql.$code")
			not(sql) {
				sql="쿼리 코드 $code 미정의"
				print("@@ $sql")
			}
			query = @baro.parseQuery(sql, target)
			if(vnm) ss.add(#[const ${vnm}=await sql`${query}`])
			node.set('@async',true)
		case res:
			ss="res.status(200).json($fparam)"
		case error:
			if(isNum(fparam)) {
				num = fparam.findPos(',').trim()				
			} else {
				num = 400
			}
			c=fparam.ch()
			if(c.eq('{')) {
				val=fparam.trim()
			} else {
				if(c.eq()) {
					err=fparam.match()
				} else {
					err=fparm
				}
				val=Cf.val(#[{error:`${err}`}])
			}
			ss="res.status($num).json($val)"
		default: print("@@ 라우트 함수 $fnm 미정의")
		}
		return ss;
	}; 
	isNum = func(s) {
		v=s.findPos(',').trim()
		return when(typeof(v,'num'), true)
	};
	node.set('@funcsVal', '')
	node.addArray('@varNames').reuse()
	while(s.valid()) {
		not(@baro.isFunc(s)) break;
		fnm =s.findPos('(',0,1)
		fparam = s.match(1)
		fsrc=funcVal(fnm, fparam)
		print("####### routeFuncVal == $fnm $fsrc")
		node.appendText('@funcsVal', fsrc)
		if(node.newline) node.appendText('@funcsVal', node.newline)
	}
}
@baro.makeRoute(backend) {
	not(backend) backend = object('baro.backend')
	base=backend.get('@route')
	not(base) return print("routes 기준노드 미정의")
	while(cur, base ) {
		print(">>",cur)
		while(sub,cur) {			
			not(sub.isset('@funcs')) continue;
			src=@baro.parseBackend(backend, sub, sub.ref('@funcs'))
			print(">> make route cur src==$src", sub.uri)
			@baro.routeFuncVal(backend, sub, src)
		}
	}
}

@baro.tableFuncVal(backend,node,&s) {
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cmt=s.findPos("\n") else cmt=s.match(1);
			if(comment) comment.add(conf('cf.newline'))
			comment.add(cmt)
			continue
		}
		a=s.move().lower()
		if(a.eq('create')) {			
			b=s.move().lower()
			if(b.eq('table')) {
				name = s.move().lower()
			} else {
				name = b
			}
		} else {
			name=a
		}
		c=s.ch()
		not(c.eq('(')) return print("table $name 시작오류")
		table=node.addNode(name)
		table.set('@name',name)
		if(comment) {
			table.set('comment', comment)
			comment=''
		}
		confCode = "baro.table#$name"
		src=s.match(1)
		prevSrc = conf(confCode)
		if(src.eq(prevSrc) ) {
			node.set('@modifySource',true)
			table.set('@modify', true)
		}
		table.set('@source', src)
		if(a.eq('create')) {
			table.set('@value', src)
		} else {
			table.set('@useParse', true)
			table.set('@value', @baro.makeCreateQuery(backend,table,src))
		}
	}
}
@baro.makeCreateQuery(backend,table,&s) {
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		c=s.ch()
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(lineCheck(s,'//')) {
			info = s.findPos('//')
			comment=s.findPos("\n").trim()
		} else if( lineCheck(s,'/*')) {
			info = s.findPos('/*',0,1)
			comment=s.match(1)
		} else {
			info = s.findPos("\n")
			comment=''
		}
		name=info.move(), type='text', size=''
		cur=table.addNode(name)
		if(@baro.isFunc(info)) {
			type=info.findPos('(',0,1).trim().lower()
			size=info.match().trim()
			cur.with(name,type,size,comment)
		} else {
			type=info.move().lower()
			cur.with(name,type,comment)
		}
		cur.set('fieldName', @baro.dbFieldName(name).upper())
		while(info.valid()) {
			c=info.ch() not(c) break;
			if(c.eq(',',';')) {
				info.incr()
				continue;
			}
			if(@baro.isFunc(info)) {
				fnm = info.move(), fparam=''
				if(info.ch('(')) {
					fparam=info.match()
				}				
				if(fnm.eq('def','default')) {
					def=''
					if(fparam) {
						c=fparam.ch()
						if(c.eq()) {
							def=fparam.match().trim()
						} else {
							def=fparam.trim()
						}
					}
					cur.set('def',true)
					if(def) cur.set('defValue',def)
				} else {
					print("@@ $fnm 테이블 필드속성 미정의")
				}
			} else {
				name = info.move()
				cur.set(name,true)
			}			
		}
		print(">> 테이블필드정보 : $cur")
	}
	nl=conf('cf.newline')
	ss=''
	while(cur, table, idx) {
		cur.inject(fieldName, name, def, defValue, pk, fk, type, size, notnull)
		if(idx) ss.add(',', nl)
		not(type) type='text'
		if(type.eq('flot')) {
			type='decimal'
			not(size) size='10, 2'
		} else if(type.eq('datetime','dt')) {
			type='timestamp'
		} else if(type.eq('vc')) {
			type='varchar'
			not(size) size='255'
		} else if(type.eq('uuid')) {
			pk=true
			def=true
			defValue='uuid_generate_v4()'
		} else if(type.eq('auto')) {
			type='serial'
		} else if(type.eq('int','num')) {
			type='integer'
		}
		ss.add("$fieldName ", type.upper())
		if(size) {
			ss.add("($size)")
		}
		if(notnull) ss.add(" NOT NULL")
		if(pk) ss.add("PRIMARY KEY")
		if(def) {
			if(defValue) {
				if(defValue.eq('now')) defValue='NOW()'
			}
			else {
				if(type.eq('integer','decimal')) defValue='0'
				else if(type.eq('timestamp')) defValue='NOW()'
				else if(type.eq('date')) defValue='CURRENT_DATE'
				else if(type.eq('time')) defValue='CURRENT_TIME'
				else if(type.eq('char') && size.eq('1')) defValue='Y'
				else if(type.eq('bool')) defValue='true'
				else defValue=''
			}
			if(@baro.isFunc(defValue)|| typeof(defValue,'num') || defValue.eq('CURRENT_DATE','CURRENT_TIME','true','false')) {
				ss.add(" DEFAULT $defValue")
			} else if(defValue) {
				ss.add(" DEFAULT '$defValue'")
			}
		}
	}
	return ss;
}

@baro.sqlFuncVal(backend,node,&s) {
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cmt=s.findPos("\n") else cmt=s.match(1);
			if(comment) comment.add(conf('cf.newline'))
			comment.add(cmt)			
			continue
		} 
		not(@baro.isFunc(s)) return print("sql 설정 시작오류 ", s)
		name = s.findPos('(',0,1).trim() not(name) break;		
		sql=node.addNode(name)
		sql.set('@name',name)
		sql.set('@source',s.match(1))
	}
}
@baro.configFuncVal(backend,node,&s) {
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		not(@baro.isFunc(s)) return print("config 설정 시작오류 ", s)
	}
}
@baro.dbFieldName(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('_',c.lower())
		} else {
			ss.add(c)
		}
	}
	return ss;
}
@baro.backendValue(backend,node,&s,indent) {
	if(_isTagValue(s)) {
		tag = s.incr().move()
		s.incr()
		props=s.findPos('=>')
		return s;
	} 
	if(s.find('@[')) {
		return @baro.parseBackend(backend,node,s)
	}
	return s;
	
	_isTagValue=func(s) {
		c=s.ch()
		if(c.eq('&')) {
			
		}
		return false;
	};
}