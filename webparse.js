<script>
	
	@web.parseTemplate(&s, fn, param) {
		not(s.find('#{')) return s;
		checkStart = false
	 	not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
	 	not(param) {
			checkStart = true
			param=fn.get('param')
		}
	 	not(typeof(param,'node')) param = null
	 	ln ="\r\n", ss=''
	 	while(s.valid()) {
	 		left = s.findPos('#{')
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		src=s.match() if(typeof(src,'bool')) continue;
	 		@web.parseTemplateVar(src,fn,param,left)
	 	}
	 	return ss; 
	}
	@web.parseTemplateVar(&s,fn,param,depth) {
		not(depth) depth=0
		result=''
		while(s.valid()) {
			c=s.ch() not(c) break;
			if(c.eq('<')) {
				sp=s.cur()
				if( s.start('<>')) {
					ss=s.match('<>','<>')
					if(typeof(ss,'bool')) ss=s.findPos('<>')
					prop=''
				} else {
					c=s.incr().next().ch()
					if(c.eq('.','-')) c=s.incr().next().ch()
					tag = s.trim(sp+1,s.cur())
					s.pos(sp)
					ss=s.match("<$tag","</$tag>")
					if(typeof(ss,'bool')) ss=s.findPos("</$tag>")
					prop=ss.findPos('>')
				}
				if(ss.ch('<')) {
					result.add(@web.parseTemplateVar(ss,fn,param,depth+1) )
				}
			} else {
				if(@web.isScriptPageVar(s)) {
					arr=[]
					a = s.findPos('[',0,1)
					arr=@web.splitParam(a,[])
					fc=call("websrc.sp_$type")
					if(typeof(fc,'func')) {
						not(param.var(funcNode)) param.var(funcNode,fn)
						call(fc,param,arr)
					}
				}
				print(">>",a,s)
			}
		}
		return result;
	}
	@web.splitParam(&s,arr) {
		arr.reuse()
		while(s.valid()) {
			c=s.ch()
			not(c) break;
			if(c.eq()) {
				v=s.match().trim()
				c=s.ch() if(c.eq(',')) s.incr()
			} else {
				v=s.findPos(',').trim()
			}
			arr.add(v)
		}
		return arr;
	}
	@websrc.sp_effect(s) {
		arr=args()
		fn=this.var(funcNode)
		
	}
	@web.isScriptPageVar(&s) {
		c=s.next().ch()
		return c.eq('[')
	};
	@web.isTemplateVar(&s) {
		c=s.next().ch()
		return c.eq('[')
	}
	@web.parseTemplateVar(&s,fn,param,left) {
		not(Cf.funcNode('parent').isset('ss')) ss=''
		not(isTemplateVar(s)) return ss.add(left,"#{$s}");
		type=s.findPos('[',0,1).trim()
		a=s.match(1).trim() 
		switch(type) {
		case var: // #{var[a]b}
			not(a) a='json'
			if(a.eq('json')) param.parseJson(s)
		case set:
			cur = param.addNode('@setMap')
			cur.set(a,@web.parseTemplate(s,fn,param))
		case html:
			cur = param.addNode('@htmlMap')
			cur.set(a,@web.parseTemplateHtml(s,fn,param))
			
		}
		return ss;
	}
	
	@web.parseConfValue(&s,fn,param) {
		not(s.find('#{')) return;
		node = param.addNode('@confValue')
		jsinfo = param.addNode('@jsInfo')
	 	while(s.valid()) {
	 		left = s.findPos('#{')
			if(left.ch()) jsinfo.appendText('init', left)
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		ss=s.match() 
			if(typeof(ss,'bool')) continue;
			k=ss.findPos('[',0,1)
			v=ss.match()
			if(typeof(v,'bool')) continue;
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


s=#[<><div class="chat-messages">
		effect[chat-list] 
		while(item) item.messageType=='recv' ? echo[chat_recv] : echo[chat_send]	
	</div>
	<>
]
parse(s)

~~
<func>
	parse(&s) {
		c=s.ch('<')
		if(c.eq('<')) {
			sp=s.cur()
			if( s.start('<>')) {
				ss=s.match('<>','<>')
				print("=======>",s,ss)
				prop=''
			} else {
				c=s.incr().next().ch()
				if(c.eq('.','-')) c=s.incr().next().ch()
				tag = s.trim(sp+1,s.cur())
				s.pos(sp)
				ss=s.match("<$tag","</$tag>")
				print("xxxx", tag, c,ss)
				prop=ss.findPos('>')
			}
			print("ss==$ss")
			parse(ss)
		} else {
			a=isPageVar(s)
			print(">>",a,s)
		}
		isPageVar = func(&s) {
			c=s.next().ch()
			return c.eq('[')
		}
	}
</func>
