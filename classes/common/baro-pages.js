@baro.initPages(projectFolder, mode, infoSource) {
	pages = object("baro.pages")
	not(projectFolder) {
		pages.inject(projectFolder, mode)
		not(projectFolder) return print("백앤드 소스생성오류 프로젝트폴더 미정의")
	}
	not(mode) mode='sql' // mode=[sql,mongo,drizzle]	
	fo = Baro.file()
	not(isFolder(projectFolder)) {
		fo.mkdir(projectFolder, true)
	}
	envPath=pathJoin(projectFolder,'.env')
	@baro.filePathInfo(projectFolder).inject(basePath,projectName)
	pages.removeAll(true)
	pages.with(projectFolder, mode, basePath, projectName)
	pages.backendInitTick = System.tick()
	if(mode.eq('sql','drizzle')) {
		pages.set('useNeon', true)
	} else if(mode.eq('mongo')) {
		pages.set('useMango', true)
	}
	if(isFile(envPath)) {
		pages.set('envs', fileRead(envPath) )
	}
	infoFile = pathJoin(projectFolder,"${projectName}.info.js")
	if(infoSource) { 
		fileWrite(infoFile, @baro.parsePages(pages, pages, infoSource))
	}
	Cf.error(true)
	not(isFile(infoFile)) {
		@baro.backend_makeInfoFile(projectFolder, infoFile)
		if(Cf.error()) return print("@baro.backend_makeInfoFile 오류:", Cf.error())
	}
	// src = stripJsComment(fileRead(infoFile))
	@baro.loadPages(pages, fileRead(infoFile), true)
	@baro.copyPagesSource(pages)
}
@baro.loadPages(pages, &s, reset) {
	not(pages) pages=object('baro.pages')
	print(">> load pages start reset==$reset", )
	_isProps = func(s) {
		c=s.ch()
		return when(c.eq('{'),true);
	};
	
	type='', base=null, cur=null
	while(s.valid()) {
		left = s.findPos('##>')
		if(type) { 
			name = cur.get('@name')
			confCode="baro.${type}:${name}#modify"
			if(conf(confCode)==left) {
				not(reset) continue;
			} else {
				pages.set("@${type}_modify", true)				
				cur.set('@modify',true)
				conf(confCode,left,true)
			}
			print(">> parse pages type=====$type $confCode start")
			if(type.eq('config')) {
				@baro.parseConfig(pages, cur, left)
			} else if(type.eq('route')) {
				@baro.parseRoute(pages, cur, left)
			} else if(type.eq('sql')) {
				@baro.sqlFuncVal(pages, cur, left)
			} else if(type.eq('table')) {
				@baro.tableFuncVal(pages, cur, left)
			} else {
				@baro.parseConfig(pages, cur, left)
			}
			print("parse pages type=====$type end")
		}
		not(s.ch()) break
		type=s.move().lower()
		if(type.eq('routes')) type='route'
		else if(type.eq('tables')) type='table'
		base=pages.addNode("@$type")
		name=''
		if(_isProps(s)) {
			node={}
			s.ch()
			src=s.match(1)
			@baro.parseConfig(pages, node, src)
			if(node.isset('@keyArray')) {
				while(k,node.get('@keyArray')) {
					if(k.eq('name')) continue;
					v=node.get(k)
					if(v) cur.set(k,v)
				}
			}
			print(">> prop node==$node")
			name=node.name
		}
		not(name) name='default'
		cur=base.addNode(name)
		if(reset) {
			cur.removeAll(true)
		}
		cur.set('@name', name)
	}	
}
@baro.copyPagesSource(pages) {
	not(pages) pages = object('baro.pages')
	pages.inject(projectFolder)
	not(isFolder(projectFolder)) return print("백앤드 소스복사 오류 프로젝트 경로 미정의", pages)
	template = @baro.conf('backendTemplatePath')
	packageJson = pathJoin(template,'package.json')
	base=pages
	Cf.error(true)	
	save = func(path) {
		srcPath = pathJoin(template,path)
		destPath = pathJoin(projectFolder,path)
		src=@baro.parsePages(pages,base,fileRead()) if(Cf.error()) return;
		not(src) return print()
		fileWrite(destPath, src)
	};
	table=pages.get('@table')
	route=pages.get('@route')
	if(pages.get('@modify')) {
		not(save('.env')) return;
		not(save('package.json')) return;
		not(save('src/config/env.js')) return;
		pages.set('@modify',false)
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
@baro.backend_infoVars(pages) {
	not(pages) pages = object('baro.pages')
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
		nodeAppendText(pages,'vars',k,',')
	}
}
@baro.backend_makeInfoSave(infoFile) {
	nl=conf('cf.newline')
	pages = object("baro.pages")
	@baro.backend_infoVars(pages)
	
	templateInfo = pathJoin(@baro.conf('backendTemplatePath'),'info.js')
	src=@baro.parsePages(pages,pages,fileRead(templateInfo)) 
	not(src) return print("$infoFile backend_makeInfoSave 소스오류")
	fileWrite(infoFile,src)
}
isSpace(&s) {
	c=s.ch()
	return when(c,false,true)
}
@baro.configKeyValue(pages, node, &s, type, value) {
	ss='', cur=null
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq('.')) {
			s.incr()
			if(@baro.isFunc(s)) {
				continue;
			}
			name=s.move()
			if(typeof(cur,'node')) {
				if(cur.isset("^$name")) {
					v=cur.ref("^$name")
					cur=v.match(1)
				} else {
					cur=cur.get(name)
				}
			} else {
				print("@@ configKeyValue 하위키를 찾을수 없습니다 name:$name", cur)
				cur=null
			}
		} else if(s.start('&&')) {
			not(cur) return;
		} else if(s.start('||')) {
			if(cur) return cur
		}
		if(@baro.isFunc(s)) {
			fnm=s.findPos('(',0,1).trim()
			fparam=s.match(1)
			cur=@baro.backendFuncVal(pages,node,fnm,fparam)
		} else {
			root=null
			name=s.move()
			if(node.isset("^$name")) {
				cur=node.get("^$name")
				continue
			}
			c=s.ch()
			if(pages.isset("@$name")) {
				root=pages.get("@$name")
				if(typeof(root,'node')) {					
					if(c.eq('.')) {
						s.incr()
						name=s.move()
						cur=root.get(name)
						c=s.ch()
						if(c.eq('.')) {
							not(typeof(cur,'node')) cur=root.get('default')
						}
					}
				}
			} else {
				if(node.isset(name)) {
					cur=node.get(name)
				} else if(pages.isset(name)) {
					cur=pages.get(name)
				} else {
					root=pages.get('@config')
					cur=root.get(name) 
					if(c.eq('.')) {
						not(typeof(cur,'node')) cur=root.get('default')
					}
				}
			}
			if(c.eq('.')) {
				s.incr()
				name=s.move()
				if(typeof(cur,'node')) {
					cur=cur.get(name)
				}
			} 
			if( cur ) continue;
			if( root) {
				while(sub,root) {
					if(sub.isset(name)) {
						cur=sub
						break;
					}
				}
			}
			if( type.eq('set')) {
				not(cur) {
					root=pages.get('@config')
					if(root) {
						cur=root.get('default')
					}
					not(cur) cur=pages
				}
				cur.set(name,@baro.backendValue(pages,node,value))
			}
		}
	}
	if( type.eq('value','string') ) {		
		return @baro.backendValue(pages,node,cur);
	}
	return cur;
}

@baro.parsePages(pages, node, &s, addCheck) {
	Cf.error(true)
	nl=conf('cf.newline')
	prevSize=-1;
	ss='', skip=false, skipCnt=0,val='';
	while(s.valid(),idx) {
		if(Cf.error()) return;
		left=s.findPos('@[')
		if(isSpace(left)) {
			if(prevSize!=ss.size()) {				
				if( left.find("\n")) ss.add(nl)
			}
		} else {
			ss.add(left)
		}
		not(s.ch()) break;
		prevSize=ss.size()
		indent=''
		if(left.find("\n")) {
			indent=@baro.lastIndent(left)
		}
		k=s.findPos(']'), def=''
		if(lineCheck(k,'||')) {
			v=@baro.configKeyValue(pages, node, k.findPos('||'),'value') not(v) v=k.trim()
			if(v) ss.add(@baro.backendValue(pages,node,v))
			continue;
		}
		ok=false
		if(lineCheck(k,'?')) {
			ok=@baro.configKeyValue(pages, node, k.findPos('?'))
			c=k.ch()
			if(c.eq('(')) {
				val=k.match(1)
				if(k.ch(':')) k.incr()
			} else {
				val=k.findPos(':')
			}
			if(ok) {
				ss.add(@baro.backendValue(pages,node,val))
			} else if(k.valid()) {
				ss.add(@baro.backendValue(pages,node,k))
			}
			continue;
		}
		sp=s.cur()
		c=s.ch()
		if(c.eq('=')) {
			s.incr()
			val=_checkValue()
			not(skip) {
				@baro.backendValue(pages,node,k,'set',val)
			}
			continue
		}
		 
		v=@baro.configKeyValue(pages,node, k, indent)
		if(v) ok=true
		if(c.eq('?')) {
			s.incr()
			v=_checkValue()
			if(ok) {
				not(skip) ss.add(@baro.backendValue(pages,node,v,indent))
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
					not(skip) ss.add(@baro.backendValue(pages,node,v,indent))
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
				ss.add(@baro.backendValue(pages,node,v,indent))
			}
			continue;
		}
		if(skip) continue;
		if(ok) {
			ss.add(@baro.backendValue(pages,node,v))
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
		if(typeof(body,'bool')) return print("@@ parsePages $tag 태그 매칭오류")
		return body;
	};
	_isElse = func(&s) {
		c=s.ch()
		if(s.start('else')) return true;
		return false;
	};
}
@baro.backendFuncVal(pages,node,fnm,fparam) {
	print(">> backendFuncVal $fnm")
	if(fnm.eq('conf')) {
		code = fparam.trim()
		return @baro.conf(code)
	}
	if( fnm.eq('fields','binds','pk','tableinfo')) {
		node = @baro.configKeyValue(pages,node,"table.$fparam")
		return json().nodeStr(node)
	} 
	if( fnm.eq('eq')) {
		a=fparam.findPos(',').trim()
		b=fparam.findPos(',').trim()
		if(pages.isset(a)) {
			return pages.cmp(a,b)
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
@baro.parseRoute(pages, node, &s) {
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
			@baro.parseConfig(pages, cur, props)
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

@baro.routeFuncVal(pages,node,&s) {
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
			sql = @baro.configKeyValue(pages, node, "sql.$code")
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
@baro.makeRoute(pages) {
	not(pages) pages = object('baro.pages')
	base=pages.get('@route')
	not(base) return print("routes 기준노드 미정의")
	while(cur, base ) {
		print(">>",cur)
		while(sub,cur) {			
			not(sub.isset('@funcs')) continue;
			src=@baro.parsePages(pages, sub, sub.ref('@funcs'))
			print(">> make route cur src==$src", sub.uri)
			@baro.routeFuncVal(pages, sub, src)
		}
	}
}

@baro.tableFuncVal(pages,node,&s) {
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
			table.set('@value', @baro.makeCreateQuery(pages,table,src))
		}
	}
}
@baro.makeCreateQuery(pages,table,&s) {
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
			cur.with(name,type,size)
		} else {
			type=info.move().lower()
			cur.with(name,type)
		}
		if(comment) cur.set('comment', comment)
		cur.set('fieldName', @baro.dbFieldName(name).upper())
		while(info.valid()) {
			c=info.ch() not(c) break;
			if(c.eq(',',';')) {
				info.incr()
				continue;
			}
			if(@baro.isFunc(info)) {
				fnm = info.move().lower(), fparam=''
				c=info.ch()
				if(c.eq('(')) {
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
				} else if(fnm.eq('fk')) {
					cur.set('fk',true)
					cur.set('fkValue',fparam)
				} else if(fnm.eq('in')) {
					fieldName=cur.set('fieldName')
					ss=''
					if(fieldName && ss ) {
						while(fparam.valid()) {
							a=fparam.findPos(',').trim()							
							if(typeof(a,'num') ) {
								ss.add(a)
							} else {
								c=a.ch()
								if(c.eq()) ss.add(a) else ss.add("'",a,"'")
							}
						}
						nodeAppendText('fieldCheck', "CHECK ($fieldName IN ($ss))", ' ')
					}
				} else if(fnm.eq('check')) {
					nodeAppendText('fieldCheck', "CHECK ($fparam)", ' ')					
				} else {
					print("@@ $fnm 테이블 필드속성 미정의")
				}
			} else {
				name = info.move()
				if(name.eq('unique')) name='uniq'
				if(name.eq('pk','fk','notnull','def','uniq')) {
					cur.set(name,true)
				} else {
					nodeAppendText('fieldInfo', name, ' ')
				}
			}			
		}
		print(">> 테이블필드정보 : $cur")
	}
	nl=conf('cf.newline')
	ss=''
	fkCnt=0
	while(cur, table) {
		if( cur.isset('fkValue') ) fkCnt++;
	}
	while(cur, table, idx) {
		cur.inject(fieldName, name, def, defValue, fk, pk, uniq, type, size, notnull, fieldCheck, fieldInfo)
		if(idx) ss.add(',', nl)
		not(type) type='text'
		if(type.eq('flot')) {
			type='decimal'
			not(size) size='10, 2'
		} else if(type.eq('datetime','dt')) {
			type='timestamp'
		} else if(type.eq('num')) {
			if(size) type='decimal' else type='integer'
		} else if(type.eq('vc')) {
			type='varchar'
			not(size) size='255'
		} else if(type.eq('uuid')) {
			def=true
			defValue='gen_random_uuid()'
			// defValue='uuid_generate_v4()'
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
				else if(type.eq('timestamp')) defValue='CURRENT_TIMESTAMP'
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
		if(fk) {
			s=cur.ref('fkValue')
			aa=s.move(), bb=''
			c=s.ch()
			if(c.eq('.')) {
				bb=s.incr().move()
			}
			if(aa) {
				if(fieldInfo) fieldInfo.add(' ')
				fieldInfo.add(aa) if(bb)fieldInfo.add(" ($bb)")
			}			
		}
		if(uniq) { 
			nodeAppendText(table, '@tableUniques', fieldName, ',')
		}		
		if(fieldCheck) {
			ss.add(" $fieldCheck")
			// ALTER TABLE ${table} ADD CHECK (${fieldName} >= 30000)
		}
		if(fieldInfo) {
			ss.add(" $fieldInfo")
		}
	}
	fields=table.get('@tableUniques')
	if(fields) {
		ss.add(',',nl,'UNIQUE(',fields,')')
	}
	return ss;
}

@baro.sqlFuncVal(pages,node,&s) {
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

@baro.backendValue(pages,node,&s,indent) {
	not(s) return;
	checkNull=func(&s) {
		a=s.move().lower()
		if( a.eq('null') && ~(s.ch()) ) return true;
		return false;
	};
	isTagValue=func(s) {
		c=s.ch()
		if(c.eq('&')) {
			
		}
		return false;
	};
	if(typeof(s,'string')) {
		if(checkNull(s)) return;
		if(isTagValue(s)) {
			tag = s.incr().move()
			s.incr()
			props=s.findPos('=>')
			return s;
		} 
		if(s.find('@[')) {
			return @baro.parsePages(pages,node,s)
		}
		return s;
	}
	if(typeof(s,'node')) {
		cur=s
		if(cur.isset('@value')) return cur.get('@value');
	}
	return "$s";
	
}
@baro.parseConfigArray(parent, node, arr, &s ) {
	not(typeof(arr,'array')) return print('@baro.parseConfigArray 배열 미정의');
	if(typeof(s,'bool')) return print('@baro.parseConfigArray 매치오류');
	arr.reuse()
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq()) { 
			arr.add(s.match())
			continue;
		}
		if(c.eq('{')) {
			cur= node.addNode()
			@baro.parseConfig(parent,node,s.match())
			arr.add(cur)
		} else if(c.eq('[')) {
			a = node.addArray()
			@baro.parseConfigArray(parent,node,a,s.match())
			arr.add(a)
		} else if(lineCheck(s,',')) {
			v=s.findPos(',').trim()
			arr.add(v)
		} else {
			v=s.findPos("\n").trim()
			arr.add(v)
		}
	}
	return arr;
}
trimLine(&s) {
	nl=conf('cf.newline')
	ss=''
	while(s.valid(),n) {
		not(s.ch()) break;
		line=s.findPos("\n").trim() not(line)continue;
		if(n) ss.add(nl)
		ss.add(line)
	}
	return ss;
}
@baro.parseConfig(parent, node, &s, overwrite) {
	if(typeof(s,'bool')) return print('@baro.parseConfig 매치오류');
	not(typeof(node,'node')) return print('@baro.parseConfig 타겟노드오류');
	arr = node.addArray('@keyArray').reuse()
	addProp = func(k,v) {
		if(arr.find(k)) {
			not(overwrite) {
				print("@@ parseConfig $k 가 중복등록되었습니다", node)
				return;
			}
			arr.remove(k)
			arr.add(k)
		} else {
			arr.add(k)
		}
		node.set(k,v)
	};
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}		
		if(c.eq()) {
			k=s.match()
		} 
		else {
			sp=s.cur()
			if(c.eq('@')) {
				s.incr()
			}
			c=s.next().ch()
			while(c.eq('-')) {
				c=s.incr().next().ch()
			}
			k=s.trim(sp,s.cur(),true)
		}
		bprop=false
		if(c.eq(':','=')) {			
			bprop=true
			c=s.incr().ch()
		}
		if(c.eq()) {
			addProp(k,s.match())
			continue
		} 
		if(c.eq('(')) {
			v=trimLine(s.match(1))
			addProp(k,v)
			continue
		}
		if(@baro.isFunc(s)) {
			sp=s.cur()
			while(@baro.isFunc(s)) {
				s.findPos('(',0,1)
				s.match()
				c=s.ch() 
				if(c.eq(',',';')) s.incr().ch()
			}
			v = s.trim(sp,s.cur(),true)
			addProp(k,v)
			continue
		}
		if(c.eq('<')) {
			if(s.start('<>')) {
				addProp(k, s.match('<>','</>'))
			} else {
				sp=s.cur()
				c=s.incr().next().ch()
				while(c.eq('-')) {
					c=s.incr().next().ch()
				}
				tag = s.trim(sp+1,s.cur(),true)
				s.pos(sp)
				src=s.match("<$tag","</$tag>",8) 
				if(typeof(src,'bool')) {
					print("@@ $tag 매칭오류")
					s.pos(sp)
					v=s.findPos("\n").trim()
					addProp(k,v)
				} else {
					prop=src.findPos('>')				
					addProp("@k","${tag}:${prop}")
					addProp(k,src)
				}
			}
			continue
		}
		if(c.eq('{','[')) {
			sub=s.match(1)
			if(c.eq('{')) {
				cur = node.addNode(k)
				@baro.parseConfig(parent,cur,sub)
			} else {
				arr = node.addArray(k)
				@baro.parseConfigArray(parent,node,arr,sub)
			}
			continue
		}
		
		if( bprop) {
			if(lineCheck(s,',')) {
				v=s.findPos(',').trim()
			} else {
				v=s.findPos("\n").trim()
			}
		} else {
			v=true
		}
		addProp(k,v)
	}
	return node;
}
@baro.setNodeVersion() {
	@baro.cmdRun(c,'node -v',func(&s) {
		pages=object('baro.pages')
		s.findPos("\n")
		line=s.findPos("\n")
		pages.set('@nodeVersion', line)
	})
}