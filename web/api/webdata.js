<func>
	kill_netstatePort(c) {
		not(c) c=cmd('python')
		closePort = func(param) {
			ss=this.ref('cmdResult')
			while(ss.valid()) {
				line=ss.findPos("\n")
				not(line.ch()) continue;
				not(line.start('TCP',true)) continue;
				line.findPos('LISTENING')
				if(line.ch()) {
					pid = line.trim()
					print("netstate kill port pid=$pid")
					if(pid) c.run("taskkill /f /pid $pid")
					return true;
				}
			}
		};
		setEvent(c, 'onResult', closePort)
		c.run("netstat -ano | findstr 8000")
	}
	pythonRun(pyFileNm, pid) {
		not(pid) pid='python'
		c=cmd(pid)
		pythonResult = func(s) {
			print("파이션 실행결과: $s")
		};
		workPath=conf('python.workPath') not(workPath) workPath='c:/bpdt/sample'
		pythonPath=conf('python.path') not(pythonPath) return print("파이션 설치경로 미정의")
		not(isFolder(workPath)) return print("파이션 $workPath 작업경로 폴더 오류")
		not(isFile("$workPath/$pyFileNm")) return print("파이션 실행파일 오류 (파일경로: $workPath/$pyFileNm)")
		py="$pythonPath/python"
		command = fv('#{py} "#{workPath}/#{pyFileNm}')
		print("python status ########### ", c.status)
		switch(c.status) {
		case first:			
			setEvent(c,'onResult',true)
			setEvent(c,'onResult',pythonResult)
			c.cmdAdd('c:')
			c.cmdAdd("cd ${workPath}")
			c.cmdAdd(command)
		case start:
			return print("파이션 프로그램이 시작중입니다", c.cmdResult)
		case stay:
			c.run(command)
		default:
		}		
	}
	pipInstall(name, mode) {
		not(mode) mode='install'
		c=cmd('pipInstall')
		c.startTime = System.localtime()
		pythonResult = func(s) {
			print("파이션 PIP ${mode} 결과: $s")
		};
		pythonPath=conf('python.path') not(pythonPath) return print("파이션 설치경로 미정의")
		py="$pythonPath/python"
		if(name=='upgrade') {
			name='--upgrade pip'
		}
		if(name=='list') {
			command = fv('#{py} -m pip list')
		} else {
			command = fv('#{py} -m pip #{mode} #{name}')
		}
		switch(c.status) {
		case first:
			setEvent(c,'onResult',true)
			setEvent(c,'onResult',pythonResult)
			c.cmdAdd('c:')
			c.cmdAdd("cd ${workPath}")
			c.cmdAdd(command)
		case start:
			return print("파이션 설치가 시작중입니다", c.cmdResult)
		case stay:
			c.run(command)
			cnt = c.get('cmd.list').size()
			if(cnt) {
				print("파이션 설치 대기수 :$cnt")
			}
		default:
		}
	}
	
</func>

wikidocs(req, param, url) {
	req.send("wikidocs start ")
}

emoji(req, param, &uri) { 
	conf('webdata.emoji',#[
표정
	😀,😃,😄,😁,😅,😂,🤣,😊,😇,🙂,🙃,😉,😌,😍,🥰
동물
	🐶,🐱,🐭,🐹,🐰,🦊,🐻,🐼,🐨,🐯,🦁,🐮,🐷,🐸,🐵
음식
	🍎,🍐,🍊,🍋,🍌,🍉,🍇,🍓,🍈,🍒,🍑,🥭,🍍,🥥,🥝	
])

	s = conf('webdata.emoji')
	s.ref()
	sp=-1;
	cur=null;
	while(s.valid()) {
		line = s.findPos("\n")
		indent = indentCount(line)
		not(line.ch()) continue;
		if(sp.eq(-1)) sp = indent
		dist = indent - sp;
		if(dist==0 ) {
			name = line.trim()
			cur = param.addNode().with(name)
		} else if(dist==1) {
			while(line.valid()) {
				not(line.ch()) break;
				emoji = line.findPos(',').trim()
				cur.addNode().with(emoji)
			}
		}
	}
	param.val('@treeMode', true)
	return param;
}

parseDdl(req,param,&uri,data) {
	s=stripSqlComment(fileRead('c:/temp/jkj.sql'))
	s=parseTable(s)
	fileWrite('c:/temp/jkj_table.txt', s)
	result = pythonSampleRun('path_test.py')
	not(result ) result = '실행결과가 없습니다'
	req.send(result)
	
	stripSqlComment = func(&s) {
		ss=''
		while(s.valid()) {
			ss.add(s.findPos('--'))
			not(s.ch()) break;
			s.findPos("\n")
		}
		return ss;
	};
	parseTable = func(&s) {
		ss = ''
		while(s.valid()) {
			a=s.move().lower()
			not(a.eq('create')) break;
			sp=skip(s)
			if(sp) s.pos(sp)
			a=s.findPos('(',1,1)
			c=a.ch()
			if(c.eq('`')) {
				table = a.match('`','`')
			} else {
				table = a.trim()
			}
			body = s.match(1)
			line = s.findPos(';')
			line.findPos('COMMENT=')
			c=line.ch()
			if(c.eq()) tableDesc=line.match() else tableDesc=''
			ss.add("$table<sep>$body<sep>$tableDesc<end>\r\n")
		}
		return ss;
	}
	skip = func(&s,type) {
		not(s.ch()) return 0;
		sp=0
		while(s.valid()) {
			a=s.move().lower()
			if(a.eq('table','if','not','exists')) {
				sp=s.cur()
				n++
				continue;
			}
			break;
		}
		return sp;
	}
}

<func>
	pythonSampleRun(runFile) {
		pythonPath=conf('python.path')
		bpdtPath='c:/bpdt'
		c=cmd('python')
		while(n=0,10) {
			if(c.status=='stay') break;
			System.sleep(500)
		}
		setEvent(c,'onResult', func(s) {
			fn=Cf.funcNode('parent')
			print("python result == $s")
			print("fn=>", fn)
			this.result = s
		})
		cmd=fv('#{pythonPath}/python #{bpdtPath}/sample/#{runFile}')
		print("python cmd=>$cmd")
		c.run(cmd)
		while(n=0,10) {
			if(c.result) break;
			System.sleep(500)
		}
		return getObjectResult(c)
	}
	getObjectResult(obj) {
		s=obj.result
		obj.result = ''
		return s;
	}
</func>

<func>
	/*
	s='<h3><a href="index.html">SQLAlchemy ORM</a> <spna>test</span></h3>'
	title=@wc.parseTagTitle(s,'h3',' ')
	*/
	@wc.callUrl(id, url, callback) {
		not(typeof(callback,'function')) return print("callUrl 웹데이터수집 오류 (콜백함수 미정의)")
		not(id) id='callUrl'
		worker = Baro.worker(id)
		map=worker.addNode("@docsMap")
		map.removeAll(true)
		map.webCallback = callback
		@wc.setCallback(worker, @wc.callWorkerProc)
		web = @wc.getWebProxy(worker, map.webCallback)
		web.call(url)
		return map;
	}
	@wc.wikidocs(url) {
		not(url) url='https://wikidocs.net/book/14285'
		id = right(url,'/')
		return callUrl(id, url, @wc.wikidocsParse)
	}
	@wc.callWorkerProc(node) {
		not(node) return;
		print("callWorkerProc >> $node")
		map=this.get('@docMap')
		not(typeof(map.webCallback,'function')) {
			return print("callUrl worker proc 웹데이터수집 오류 (콜백함수 미정의)")
		}
		node.inject(type, mode, url, saveFileName)
		if( url ) {
			web = @wc.getWebProxy(worker, map.webCallback)
			web.call(url)
		}
	}
	@wc.setCallback(obj, func) {
		fn=obj.get('@callback')
		if(typeof(fn,'func')) return;
		obj.set('@callback', Cf.funcNode(func,obj))
		return obj;
	}
	@wc.webProxyProc(type, data) {
		switch(type) {
		case read: this.appendText('@webResult',data)
		case error: this.set('@error', data)
		case finish:
			fn = this.onResult
			not(typeof(fn,'func')) {
				print("@@ web result callback not define")
				return;
			}
			data = this.ref('@webResult')
			fn.callFuncParams(data)
			fn.callFuncSrc()
		default:
		}
	}
	@wc.getWebProxy(target, callback) {
		proxys = _arr('user.webProxys')
		addWeb = func(idx) {
			not(idx) idx= proxys.size()+1;
			web=proxys.add(Baro.web("webProxy$idx"))
			return @wc.setCallback(web, @wc.webProxyProc)
		};
		not(proxys.size()) {
			while(n=1,5) addWeb(n)
		}
		web=null
		while(cur, proxys) {
			if(cur.is('run')) continue;
			web=cur
			break;
		}
		not(web) {
			web=addWeb()
		}
		if( web.onResult ) {
			setEvent(web, 'onResult', true)
		}
		if( callback) {
			setEvent(web,'onResult',callback)
		}
		web.set('@target', target)
		web.set('@webResult','')
		return web;
	}
	
	@wc.sqlalchemyParse(s) {
		// @wc.callUrl('sqlalchemy','https://docs.sqlalchemy.org/en/20/orm/index.html',@wc.sqlalchemyParse)
		not(worker) worker=this.get('@target')
		map = worker.get('@docsMap')
		s.findPos('<div id="docs-sidebar-inner">')
		s.findPos('<h3>')
		docTitle = s.findPos('</h3>')
		sp=s.cur()
		ep=@wc.makeUl(map,s)
		if(sp>=ep) break;
		s.findPos('<section id="declarative-mapping-styles"',0,1)
		content = s.match('<section','</section>')
		prop = content.findPos('>')		
	}
	
	 
	@wc.wikidocsParse(s, worker) {
		not(worker) worker = this.get("@target")
		map = worker.get('@wikidocsMap')
		print("@@ wikidocs parse start ", s.size(), map)
		s.findPos('<div class="list-group',0,1)
		ss=s.match('<div','</div>',8)
		ss.findPos('>')
		while(ss.valid()) {
			ss.findPos('<a')
			not(ss.ch()) break;
			sa=ss.match('<a','</a>',8) if(typeof(sa,'bool')) break;
			prop=sa.findPos('>')			
			prop.findPos('href=')
			href=prop.match()
			if(href.start('javascript:page',true)) {
				href=href.match()
			}
			not(sa.find('<span')) continue;
			padding='', title=''
			sa.findPos('<span',0,1)
			sss=sa.match('<span','</span>',8) if(typeof(sss,'bool')) break;
			if(sss.find('<span')) {
				prop=sss.findPos('>')
				prop.findPos('style=')
				if(prop.ch()) pading=prop.match()
			} else if(sss.find('<strong')) {
				sss.findPos('<strong')
				sss.findPos('>')
				title = sss.findPos('</strong>')
				padding='strong'
			}
			print("@@ title=$title, href=$href")
			if(href && title) {
				not(map.isVar(href)) {
					type='wikidocs'
					cur=map.addNode(href).with(type, href, title, padding)
					worker.push(cur)
				}
			}
		}
		s.findPos('<div class="clearfix',0,1)
		ss=s.match('<div','</div>',8)
		ss.findPos('<ol')
		ss.findPos('>')
		sss=ss.findPos('</ol>')
		docNav = @wc.parseTagTitle(sss,'li','/')

		s.findPos('<h1 class="page-subject">')
		docTitle = s.findPos('</h1>').trim()
		s.findPos('<div class="page-content',0,1)
		ss=s.match('<div','</div>',8)
		ss.findPos('>')
		
		docBody = @wc.htmlDownloadImage(ss,worker)
		cur.with(docNav, docTitle, docBody)	
	}
	@wc.htmlDownloadImage(&s, worker) {
		not(worker) worker=this.get('@target')
		map = worker.get('@wikidocsMap')
		while(s.valid()) {
			left = s.findPos('<img')
			prop = s.findPos('>')
			if( prop.find('src=') ) {
				prop.findPos('src=')
				isrc = prop.match()
				not(map.isVar(isrc)) {
					iidx=job.incrNum('docImageIndex')
					cur=map.addNode(isrc)
					ext=right(isrc,'.')
					cur.type='image'
					cur.saveFileName="images/doc_img_${iidx}.${ext}"
					worker.push(cur)
				}
			}
		}
	}
	@wc.parseTagTitle(&s,tag,sep) {
		rst=''
		while(s.valid()) {
			s.findPos("<$tag",0,1)			
			ss=s.match("<$tag","</$tag>",8) if(typeof(ss,'bool')) return rst
			ss.findPos('>') 
			a=@wc.stripTag(ss)
			not(a) break;
			if(rst) rst.add(sep)
			rst.add(a)
		}
		return rst;
	}
	@wc.stripTag(&s) {
		rst=''
		while(s.valid()) {
			left=s.findPos('<')
			if(s.start('!--')) {
				s.match('<!--','-->')
				continue;
			}
			tag=s.move()
			not(tag.ch('/')) {				
				if(tag.eq('script','style')) {
					s.findPos("</$tag>")
					continue;
				}
			}
			if(left.ch()) rst.add(left)
			not(s.ch()) break;
			s.findPos('>')
		}
		return rst;
	}
	@wc.makeUl(node, &s, depth) {
		not(depth) depth=0
		type='li'
		s.findPos('<ul>',0,1)
		ul=s.findPos('<ul','</ul>') 
		while(ul.valid()) {
			ul.findPos('<li>',0,1)
			li=ul.match('<li','</li>', 8) if(typeof(li,'bool')) break;
			prop=li.findPos('>')
			body = li.findPos('<ul',0,1)
			cur=node.addNode().with(type, prop, body, depth)
			if(li.ch()) {
				@wc.makeUl(cur, li, depth+1)
			}
		}
		return s.cur();
	}	
	@wc.clearTagProp(&s) {
		ss=''
		while(s.valid()) {
			left=s.findPos('<')
			ss.add(left)
			not(s.ch()) break;
			tag=s.move()
			s.findPos('>') not(s.ch()) break;
			ss.add("<$tag>")
		}
		return ss;
	}
</func>