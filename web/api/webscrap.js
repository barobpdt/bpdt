test(req, param, uri) {
	a=uri.findPos('/')
	param.set('@workType','webscrap')
	param.set('@workUrl','https://wikidocs.net/book/14452')
	param.set('@timerFunc','@wc.wikidocs')
	
	param.set('@apiResult','')
	globalTimeout(param)
	while(n=0,20) {
		if(param.get('@apiResult')) break;
		System.sleep(500)
	}
	result = param.get('@apiResult')
	not(result) result='호출결과가 없습니다'
	req.send(result)
}
<func>
	/*
	s='<h3><a href="index.html">SQLAlchemy ORM</a> <spna>test</span></h3>'
	title=@wc.parseTagTitle(s,'h3',' ')
	*/
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
		return @wc.callUrl(id, url, @wc.wikidocsParse)
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
			tempSavePath = conf('ws.savePath')
			not(tempSavePath) { 
				tempSavePath="c:/temp/web_${System.tick()}.html"
			}
			fileWrite(tempSavePath, data)
			fn.callFuncParams(data)
			fn.callFuncSrc()
		default:
		}
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
		s.pos(ep)
		s.findPos('<section id="declarative-mapping-styles"',0,1) not(s.ch()) return print("본문시작 오류");
		content = s.match('<section','</section>') if(typeof(content,'bool')) return print("본문시작 section 매칭 오류");
		prop = content.findPos('>')		
	}
	
	 
	@wc.wikidocsParse(s, worker) {
		not(worker) worker = this.get("@target")
		map = worker.get('@docsMap')
		not(typeof(map,'node')) return print("@@ wikidocs parse error [map노드 미정의]", this);
		print("@@ wikidocs parse start ", s.size(), map)
		s.findPos('<div class="list-group',0,1)
		ss=s.match('<div','</div>',8)
		ss.findPos('>')
		while(ss.valid()) {
			ss.findPos('<a',0,1)
			not(ss.ch()) break;
			sa=ss.match('<a','</a>',8) if(typeof(sa,'bool')) break;
			prop=sa.findPos('>')			
			prop.findPos('href=')
			href=prop.match() 
			if(typeof(href,'bool')) {
				print("prop=$prop", sa);
				continue;
			}
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
		ul=s.match('<ul','</ul>')
		print("@@ make ul >> ", ul.size())
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