<script>
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
	 	not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
	 	not(param) param=fn.get('param')
	 	not(typeof(param,'node')) param = null
	 	ss=''
	 	while(s.valid()) {
	 		left = s.findPos('#{')
	 		ss.add(left)
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		a=s.match() if(typeof(a,'bool')) continue;
	 		ss.add(@web.parseVar(a,fn,param) )
	 	}
	 	return ss;
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
	@web.parseVar(&s,fn,param) { 
		// #{test:default}
		not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
		val = ''
		if( @src.isVar(s) ) {
			code=s.findPos(':').trim()
			if(code.find('.')) {
				cur=null
				name = left(code,'.')
				if(fn.isset(name)) {
					cur=fn.get(name)
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
	@web.parsePrint(&s,fn,param) {
		result = ''
		while(s.valid()) {
			line=s.findPos('[',1,1)
			src=s.match()
			if(line.find('.')) {
				if(line.ch('@')) {
					line.incr()
					code = line.trim()
					conf(code, src, true)
				} else {
					code = line.trim()
					conf(code, src)					
				}
			} else {
				type = line.trim()
				if(type.eq('func','function')) {
					@src.addFuncSource(src)
				} else if(type.eq('eval')) {
					eval(src, fn, param)
				} else if(type.eq('set')) {
					param.parseJson(src)
				} else {
					param.set(type, @web.parseTemplate(src,fn,param))
					result.add(param.get(type))
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
	@src.isPrint(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()
		if(c.eq('#')) c=s.incr().ch()
		if(c.eq('.')) {
			c=s.incr().next().ch()
			if(c.eq('#')) c=s.incr().ch()
		}
		if(c.eq('[') ) return true;
		return false;
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
		if(c.eq('#')) c=s.next().ch()
		if(c.eq('.')) {
			c=s.next().ch()
			if(c.eq('#')) c=s.next().ch()
		}
		if(c.eq(':')) c=''
		not(c) return true;
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