<script>
	@web.addUrl(url, fc, skip) {
		if(typeof(fc,'string')) {
			if(fc.find('.')) {
				fc=call(fc)
			} else {
				fnm = Cf.val('webpage.',fc)
				fc=call(fnm)
			}
		}
		not(typeof(fc,'function')) {
			if(skip) return;
			return print("URL 등록오류 : $url 실행 함수 미정의");
		}
		map = Baro.was().uriMap()
		prev = map.get(url)
		if(typeof(prev,'function')) {
			if(skip) return;
			return print("이미등록된 URL 입니다 (경로:$url)");
		}
		map.set(url, fc)
	} 

	@web.mskeCssSvg(path) {
		not(path) path='C:\WORK\baro\data\solid'
		Baro.file().list(path, func(info) {
			while(info.next()) {
				info.inject(type, name, ext, fullPath)
				nm = left(name,'.')
				src = @web.parseSvg(fileRead(fullPath) )
				if(src) {
					print(">> name=>svg.$nm")
					conf("svg.$nm", src, true)
				}
			}
		})
	}
	@web.parseSvg(&s) {
		ss=s.match('<svg','</svg>')
		if(typeof(ss,'bool')) return;
		rst = ss.findPos('>').trim()
		while(ss.valid()) {
			ss.findPos('<path')
			not(ss.ch()) break;
			ss.findPos('d=')
			rst.add("\r\n",ss.match().trim() )
		}
		return rst;
	}
	@web.cssImgDownload(cssFileName, savePath, checkReplace) {
		not(cssFileName) return print("css 파일 미정의")
		if(cssFileName.find('\')) cssFileName = cssFileName.replace('\','/')
		not(isFile(cssFileName)) return print("$cssFileName 파일이 없습니다");
		path = cssFileName.findLast('/').value()
		not(path) return print("css 이미지 저장경로 미정의")
		relativePath='', wd=null
		if(savePath) {
			webPath = conf('web.rootPath')
			if(savePath.start(webPath)) { 
				relativePath = savePath.value(webPath.size())
			}
		} else {
			relativePath = 'icons'
			savePath = "${path}/icons"
			not(isFolder(savePath)) Baro.file().mkdir(savePath,true)
		}
		if(relativePath) {
			wd = webDownload('css', savePath, 3)
		} else {
			relativePath='icons'
		}
		s=fileRead(cssFileName) s.ref()
		print("cssImgDownload web download => $cssFileName", s.size())
		ss='', cnt=0
		while(s.valid()) {
			left = s.findPos('background-image:')
			ss.add(left)
			not(s.ch()) break;
			ss.add('background-image:')
			if( s.start('url(',true)) {
				aa = s.findPos(')')
				c=aa.ch()
				if(c.eq()) url = aa.match().trim() else url = aa.trim();
				name = right(url,'/')
				if(wd ) wd.downloadAdd(url,name)
				ss.add('url(',Cf.jsValue("$relativePath/$name"),')')
				cnt++;
			}
		}
		if(cnt) {
			if(wd) wd.downloadStart()
			if(checkReplace) fileWrite(cssFileName,ss)
		}
		return cnt;
	}
	@web.makeMetaData() {
		// todo ....
		sp = System.tick()
		fo=Baro.file()
		sa='', sb=''
		ma='', mb=''
		fo.list("$path/css/icons", func(info) {
			while(info.next()) {
				info.inject(name, fullPath, ext)
				if(ext.eq('svg')) {
					ps=sa.size()
					sa.add(fileRead(fullPath))
					pe=sa.size()
					size = ep-sa;
					ma.add("[$name,$ps,$size]")
				}
				if(ext.eq('png')) {
					ps=sb.size()
					sb.add(fileRead(fullPath))
					size = sb.size() - ps;
					mb.add("[$name,$ps,$size]")
				}
			}
		})
		fileWrite("$path/css/emoji-svg.data",sa)
		fileWrite("$path/css/emoji-png.data",sb)
		fileWrite("$path/css/emoji-svg.meta",ma)
		fileWrite("$path/css/emoji-png.meta",mb)
		d=System.tick() - sp;
		print("x d==$d xx",sa.size(), sb.size())
	}
	@web.makeMetaMap(metaName, path) {
		not(path) path=Cf.val(conf('web.rootPath'),"/css")
		fileName = "$path/${metaName}.meta"
		not(isFile(fileName)) return print("$fileName 파일 미정의");
		s=fileRead(fileName)
		s.ref()
		map = @web.metaMap(metaName)
		if(map) return map;
		map = _node('metaNodes').addNode(metaName)
		map.set("dataPath", "$path/${metaName}.data")
		map.set("dataSource", fileRead("$path/${metaName}.data"))
		while(s.valid()) {
			ss=s.match() if(typeof(ss,'bool')) break;
			ss.split(',').inject(name,offset,size)
			map.addNode(name).with(name,offset,size)
		}
		print("@@ make metaMap >> ", metaName, map.childCount()) 
		return map
	}
	@web.metaMap(metaName) { return _node('metaNodes').get(metaName) }
	
	@web.makeMetaBuffer(metaName, fileName) {
		root = Cf.rootNode()
		metas = root.addNode('_node.metaNodes')
		map = metas.get(metaName) not(typeof(map,'node')) return print("$metaName 메타정보 미설정")
		cur = map.get(fileName) not(typeof(cur,'node')) return print("$metaName $fileName 메타 파일정보 찾기오류")
		buf = map.ref('dataSource')
		cur.inject(offset, size)
		end = offset + size;
		return buf.value(offset,end);
	}	
	@web.init() {
		map = Baro.was().uriMap()
		map.set('/test', @web.test)
	}
	@web.test(req, param) {
		title = '테스트 페이지 입니다'
		style=@web.cssUiIcon('address-book')
		style.add( @web.cssUiIcon('address-card') )
		body = #[
			<div>
				<i class="ui_icon address-book"></i>
				<i class="ui_icon address-card"></i>				
			</div>
		]
		src = fileRead("$path/template/default.html")
		req.send( @web.parseTemplate(src)) 
	} 

	@web.parseTemplate(&s, fn, param) {
		not(s.find('#{')) return s;
		checkStart = false
	 	not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
	 	not(param) {
			checkStart = true
			param=fn.get('param')
		}
	 	not(typeof(param,'node')) param = null
	 	ss=''
	 	while(s.valid()) {
	 		left = s.findPos('#{')
	 		ss.add(left)
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		src=s.match() if(typeof(src,'bool')) continue;
	 		ss.add(@web.parseVar(src,fn,param) )
	 	}
		cf = param.val('@confValue')
		print("xxxxxxx $param, $cf", cf.keys())
		if(checkStart) {
			data=param.val('script')
			if(param.isVar('@jsInfo')) {
				node = param.val('@jsInfo')
				while(key, node.keys()) {
					src = node.val(key)
					if(ss.find("{__script-${key}__}")) {
						ss=_replace(ss,"{__script-${key}__}",@web.parseTemplate(src, fn, param))
					} else {
						data.add(src)
					}
				}
			}
			if(data && ss.find("{__script__}") ) {
				ss=_replace(ss,"{__script__}",@web.parseTemplate(data, fn, param))
			}
			data=param.val('css')
			if(param.isVar('@cssInfo')) {
				node = param.val('@cssInfo')
				print("node=>$node")
				while(key, node.keys()) {
					src = node.val(key)
					if(ss.find("{__css-${key}__}")) {
						ss=_replace(ss,"{__css-${key}__}",@web.parseTemplate(src, fn, param))
					} else {
						data.add(src)
					}
				}
			}
			if(data && ss.find("{__css__}") ) {
				ss=_replace(ss,"{__css__}",@web.parseTemplate(data, fn, param))
			}
		}
	 	return ss;
	 	
	 	_replace = func(&s,a,b) {
	 		ss=''
	 		left = s.findPos(a)
	 		ss.add(left,b,s)
	 		return ss;
	 	};
	}
	@web.parseVar(&s,fn,param) { 
		not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
		val = ''
		if( @src.isVar(s) ) {
			ss=s.findPos(':')
			if(ss.start('script-',true)) {
				code=ss.trim()
				return "{__script-${code}__}";
			} 
			if(ss.start('css-',true)) {
				code=ss.trim()
				return "{__css-${code}__}";
			}
			code = ss.trim()
			if(code.eq('script')) {
				return "{__script__}";
			} 
			if(code.eq('css')) {
				return "{__css__}";
			}
			if(code.find('.')) {
				cur=null
				name = left(code,'.')
				if(fn.isset(name)) {
					cur = fn.get(name)
				} else if(param.isVar(name)) {
					cur = param.get(name)
				} 
				if(typeof(cur,'node')) {
					name=right(code,'.') not(cur.isVar(name)) return print("변수값오류 객체에 $name 설정되지 않았습니다")
					val = cur.get(name)
				} else {
					val = @web.parseTemplate(conf(code),fn,param)
				}
			} else {
				if(param.isVar(code)) {
					val=param.get(code)
				}
				not(val) val = fn.get(code)
			}
			not(val) {
				if(s.ch()) {
					val=@web.parseTemplate(s,fn,param)
				}
			}
		}
		else if( @src.isPrint(s) ) val = @web.parsePrint(s,fn,param)
		else if( @src.isControl(s) ) val = @web.parseControl(s,fn,param)
		else if( @src.isCase(s) ) val = @web.parseCase(s,fn,param)
		else if( @src.isFunc(s) ) val = eval(s);
		return val;
	}
	@web.parseConfValue(&s,fn,param) {
		not(s.find('#{')) return;
		node = param.addNode('@confValue')
	 	while(s.valid()) {
	 		left = s.findPos('#{')
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		ss=s.match() if(typeof(ss,'bool')) continue;
			k=ss.findPos('[',0,1)
			v=ss.match()
			if(typeof(v,'bool')) break;
			if(k.start('css-',true)) {
				code= k.trim()
				cur = param.addNode('@cssInfo')
				cur.appendText(code, v)
				continue;
			} 
			if(k.start('script-',true)) {
				code= k.trim()
				cur = param.addNode('@jsInfo')
				cur.appendText(code, v)
				continue;
			} 
			key = k.trim()
			if(key.eq('script')) {
				param.appendText('script', v)
			} else if(key.eq('css')) {
				param.appendText('css', v)
			} else {
				node.val(key, v)
			}
	 	}
	}
	@web.parsePrint(&s,fn,param) {
		result = '' 
		line=s.findPos('[',0,1) not(line.ch()) return;
		src=s.match() if(typeof(src,'bool')) return;
		if(line.start('css-',true)) {
			code= line.trim()
			cur = param.addNode('@cssInfo')
			cur.appendText(code, src)
			return;
		}
		if(line.start('script-',true)) {
			code= line.trim()
			cur = param.addNode('@jsInfo')
			cur.appendText(code, src)
			return;
		}
		if(line.find('.')) {
			if(line.ch('@')) {
				line.incr()
				code = line.trim()
				conf(code, src, true)
			} else {					
				code = line.findPos('.').trim()
				if(code.eq('conf')) {
					node = param.val('@confValue', true)
					code = line.trim()
					node.val(code, src)
				}
			}
		} else {
			type = line.trim()
			if(type.eq('func','function')) {
				@src.addFuncSource(src)
			} else if(type.eq('eval')) {
				eval(src, fn, param)
			} else if(type.eq('set')) {
				param.parseJson(src)
			} else if(type.eq('script')) {
				param.appendText('script', src)
			} else if(type.eq('css')) {
				param.appendText('css', src)
			} else if(type.eq('include')) {
				path = param.val('webpageFileName').findLast('/').trim()
				while(src.valid()) {
					line = src.findPos("\n").trim()
					not(line) continue;
					@web.parseConfValue(fileRead("$path/$line"),fn,param)
				}
			} else if(type.eq('print')) {
				result.add(@web.parseTemplate(src,fn,param))
			} else {
				param.set(type, @web.parseTemplate(src,fn,param))
			}
		} 
		return result;
	}
	 
	 
	@web.parseControl(&s,fn,param) {
		ftype= s.findPos('(',1,1)
		fparam = s.match()
		not(s.ch('<')) return print("$ftype 시작문자 오류", s)
		sp= s.cur()
		c=s.incr().ch()
		if(c.eq('>')) {
			s.pos(sp)
			src = s.match('<>','<>')
			if(typeof(src,'bool')) return print("$s 시작태그 오류")
		} else {
			tag = s.move()
			s.pos(sp)
			match = s.match("<$tag","</$tag>")
			if(typeof(match,'bool')) return print("$tag 태그매치 오류")
			src = s.value(sp, s.cur())
		}
		result = ''
		if(ftype.eq('each')) {
			fparam.split(',').inject(a,b)
			node = fn.get(b) not(node) node=param.get(b)
			if(typeof(node,'node')) {
				while(cur, node) {
					fn.set(a,cur)
					result.add(@web.parseTemplate(src,fn,param))
				}
			} else {
				result = print("each( $a, $b ) 오브젝트 설정오류")
			}
		}
		return result;
	}
	 
	@web.parsePrint(&s,fn,param) {
		result = ''
		while(s.valid()) {
			line=s.findPos('[',1,1)
			src=s.match()
			if(typeof(src,'bool')) {
				s.findPos(']')
				continue;
			}
			not(line.ch()) break;
			if(line.start('css-',true)) {
				code= line.trim()
				css = param.val('@cssInfo',true)
				css.val(code, src)
			}
			if(line.find('.')) {
				if(line.ch('@')) {
					line.incr()
					code = line.trim()
					conf(code, src, true)
				} else {					
					code = line.findPos('.').trim()
					if(code.eq('conf')) {
						node = param.val('@confValue', true)
						code = line.trim()
						node.val(code, src)
					}
				}
			} else {
				type = line.trim()
				if(type.eq('func','function')) {
					@src.addFuncSource(src)
				} else if(type.eq('eval')) {
					eval(src, fn, param)
				} else if(type.eq('set')) {
					param.parseJson(src)
				} else if(type.eq('script')) {
					param.appendText('script', @web.parseTemplate(src,fn,param))
				} else if(type.eq('include')) {
					path = param.val('webpageFileName').findLast('/').trim()
					while(src.valid()) {
						line = src.findPos("\n").trim()
						not(line) continue;
						@web.parseConfValue(fileRead("$path/$line"),fn,param)
					}
				} else if(type.eq('print')) {
					result.add(@web.parseTemplate(src,fn,param))
				} else {
					param.set(type, @web.parseTemplate(src,fn,param))
				}
			}
			c=s.ch()
			not(c) break;
			if(c.eq(',',';')) s.incr()
		}
		return result;
	}
	@web.parseCase(&s,fn,param,skipCheck){
		if(skipCheck) {
			c=s.ch()
		} else { 
			c=s.ch()
			sp = s.cur()
			if(c.eq('@')) c=s.incr()
			c=s.next().ch()
			if(c.eq('.')) c=s.incr().next().ch()
			ok=false
			if(c.eq('(')) {
				s.match()
				src = s.trim(sp, s.cur())
				if( eval(src) ) ok=true
			} else {
				name = s.trim(sp, s.cur())
				if(fn.isset(name)) {
					if(fn.get(name)) ok=true;	
				} else if(param.isVar(name)) {
					if(param.get(name)) ok=true;
				}
			}
			c=s.ch()
			not(c.eq('?')) return print("parseCase case 매칭오류", s.size())
			c=s.incr().ch()
		}
		result=''
		if(c.eq('<')) {
			sp=s.cur()
			if(s.start('<>')) {
				result = s.match('<>','<>')
			} else {
				tag=s.incr().move()
				s.pos(sp)
				print("s===$s")
				if( @src.isSingleTag(s) ) {
					s.findPos("/>")
				} else {
					match=s.match("<$tag","</$tag>")
					if(typeof(match,'bool')) return print("parseCase $tag 매칭오류 ")
				}
				ep = s.cur()
				result = s.value(sp,ep,true)
			}
		} else if(c.eq('(','[')) {
			result = s.match()
		} else if(c.eq()) {
			result = s.match()
		} else if(s.start('#{')) {
			s.incr()
			src=s.match()
			result = @web.parseVar(src,fn,param)
		} else {
			result = s.findPos(':',1,1).trim()
		}
		if(ok) {
			return @web.parseTemplate(result,fn,param);
		}
		c=s.ch()
		if(c.eq(':')) {
			s.incr()
			skipCheck = ~(@src.isCase(s))
			result = @web.parseCase(s,fn,param,skipCheck)
		}
		return result;
	}
	@src.isControl(&s) {
		c=s.next().ch()
		if(c.eq('(')) {
			s.match()
			c=s.ch()
			if(c.eq('<')) return true;
		}
		return false;
	}
	@src.isSingleTag(&s) {
		left= s.findPos('>')
		c=left.ch(-1)
		return c.eq('/')
	} 
	@src.isFunc(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()
		if(c.eq('.')) c=s.incr().next().ch()
		if(c.eq('(')) {
			s.match()
			not( s.ch() ) return true;
		}
		return false;
	}
	@src.isCase(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()	
		if(c.eq('.')) c=s.incr().next().ch()
		if(c.eq('(')) {
			s.match()
			c=s.ch()
		}
		return c.eq('?');
	}
	
	@src.isVar(&s) {
		c=s.next().ch()
		if(c.eq('#')) c=s.incr().next().ch()
		if(c.eq('.','-')) {
			c=s.incr().next().ch()
			if(c.eq('#')) c=s.incr().next().ch()
		}
		if(c.eq(':')) c=''
		not(c) return true;
		return false;
	}
	@src.isPrint(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()
		if(c.eq('#','-')) c=s.incr().next().ch()
		if(c.eq('.')) {
			c=s.incr().next().ch()
			if(c.eq('#','-')) c=s.incr().ch()
		}
		if(c.eq('[') ) return true;
		return false;
	}
	@src.addFuncSource(&s) {
		ss=''
		while(s.valid()) {
			sp = s.cur()
			fnm = s.findPos('(',1,1).trim()
			fc=call(fnm)
			ok=true if(typeof(fc,'function')) ok=false;
			s.match()
			not(s.ch('{')) break;
			a=s.match(1) if(typeof(a,'bool')) return print("$fnm 함수 매칭오류");
			if(ok) {
				print("$fnm 웹사용 함수등록")
				fsrc=s.value(sp,s.cur())
				ss.add(fsrc)
			}
			c=s.ch() not(c) break;
			if(c.eq(',',';')) s.incr()
		}
		if(ss) call(ss)
	}
</script>

<script>
	@web.cssUiIcon(name, w, h) {
		not(w) w=24
		not(h) h=w
		src = conf("svg.$name")
		not(src) print("svg $name 로드오류 ")
		src.ref()
		prop = src.findPos("\n").trim()
		ss=''
		while(src.valid()) {
			d = src.findPos("\n").trim()
			ss.add( fv('<path d="#{d}"/>') )
		}
		// background-size: contain;
		return 
#[
	.ui_icon.${name} {
		background-image: url('data:image/svg+xml,<svg ${prop}>${ss}</svg>');
		width: ${w}px;
		height: ${h}px;
		background-position: center;
	}
]
	}
</script>