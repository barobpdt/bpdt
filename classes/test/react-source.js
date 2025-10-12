<func>
	pushArray(a,b) {
		if(a.size()) a.insert(0,b) else a.add(b)
		return b;
	}
	log(&a) {
		line = a.findPos("\n")
	}
	@ws.pushIncVars(param, code) {
		param.inject(@incStack, @incMap)
		if( incMap.isVar(code) ) {
			return print("$code 컨포넌트가 이미 등록되었습니다");
		}
		cur = incMap.addNode(code)
		not( incStack.size() ) {
			while(key, param.keys()) {
				cur.set(key, param.get(key))
			}
		}
		parent = incStack.get(0) not(parent) parent=incMap
		cur.set('@parentIncNode', parent)
		cur.set('@incCode', code)
		cur.set('@incIndex', iidx)
		cur.set('@incFuncName', @ws.getFnm())
		cur.set('@incFuncSrc','')
		print("@@ push inc vars cur ==> $cur")
		return pushArray(incStack, cur);
	}
	@ws.page(req, param) {
		end=";\n"
		ln="\n"
		param.set('@funcNode', Cf.funcNode())
		param.addArray('@srcStack')
		param.addArray('@incStack')
		param.addNode('@incMap')
		param.set('@js', '')
		param.set('@jsRady', '')
		
		srcPath = @ws.pagePath(req.getValue('url'))
		log("web page path => $srcPath")
		src=fileRead(srcPath)
		param.set('@srcPath', srcPath)
		src = @ws.parseTemplate(src, param)
		param.inject(@incStack, @srcStack)
		cur=incStack.pop()
		cur.inject(@parentIncNode, @incFuncName, @incFuncSrc)
		fsrc = srcStack.pop()
		param.appendText('@jsScript', "const ${incFuncName} = ()=>{$fsrc}",ln)
		req.send(src) 
	} 
	@ws.parseText(param, &s, parentEl) {
		ss='', sp=0
		while(s.valid()) {
			c=s.ch()
			not(c) break;
			while( c.is('oper') ) {
				c=s.incr().ch()
				not(c) break
			}
			not(c) break;
			not(@ws.isWebTag(s) ) { 
				s.next()
				continue;
			}
			Cf.error(true)
			ep=s.cur()
			left = s.value(sp,ep)
			ss.add(left)
			sp = @ws.makeWebTag(param,s,parentEl) not(typeof(sp,'number')) return;
			if(Cf.error()) {
				ss.add(Cf.error() )
				break	
			}
			if( sp<ep) {
				log("parse template end point error ($sp, $ep) ")
				break;
			}
			s.pos(sp)
		}
		ep=s.cur()
		if(sp<ep) ss.add(s.value(sp,ep))
		return ss;
	}
	@ws.parseTag(param, &s, parentEl, depth) {
		use(end)
		not(depth) depth = 0
		param.inject(@srcStack, @incStack)
		cur = incStack.get(0)
		not(parentEl) {
			return print("parseTag 오류 부모요소 미정의 inc=>",cur)
		}
		while(s.valid() ) {
			c=s.ch()
			if( @ws.isWebTag(s) ) {
				sp = @ws.makeWebTag(param,s) not(typeof(sp,'number')) return;
				if(Cf.error()) {
					ss.add(Cf.error() )
					break	
				}
				if( sp<ep) {
					log("parse template end point error ($sp, $ep) ")
					break;
				}
				s.pos(sp)
				continue;
			}
			if( c.eq('<')) {
				sp=s.cur()
				c=s.incr().next().ch()
				while(c.eq('-')) c=ss.incr().next().ch()
				tag = s.trim(sp+1, s.cur())
				print("$depth>>$tag")
				s.pos(sp)
				ss=s.match("<$tag", "</$tag>",8) if(typeof(ss,'bool')) return print("$tag match error", s)
				idx = cur.incrNum('elIndex')
				props=parseProp()
				srcStack.append(0,"childEl[e$idx]=createEl('$tag',$props)", end)
				@ws.parseTag(param,ss,"childEl[e$idx]",depth+1)
				srcStack.append(0,"${parentEl}.append(childEl[e$idx])", end)
			} else {
				@ws.parseText(param,ss,parentEl)
			}
		}
		parseProp = func() {
			rst='{'
			while(ss.valid()) {
				c=ss.ch()
				not(c) break;
				if(c.eq('>')) {
					ss.incr()
					rst.add('}')
					return rst;
				}
				sp=ss.cur()
				c=ss.next().ch()
				while(c.eq('-')) c=ss.incr().next().ch()
				key=ss.trim(sp,ss.cur())
				
				not(ss.ch('=')) {
					print("\t>> not match prop start [key:$key] c==$c")
					ss.findPos('>')
					break;
				}
				c=ss.incr().ch()
				if(c.eq()) {
					val=ss.match()
				} else if(c.eq('{')) {
					val=ss.match(1)
				} else {
					val=ss.findPos(" \t\n>",4)
				}
				print("\t>> $key = $val")
			}
			return '[]';
		};
	}

	pushArray(a,b) {
		if(a.size()) a.insert(0,b) else a.add(b)
		return b;
	}
	log(&s) {
		not(s.ch()) return;
		line=s.findPos("\n")
		while(a, args(), n) {
			if(n) {
				aa="$a"
				line.add("\n\t[$n] ", aa.findPos("\n"))
			}
		}
		print(line)
	}
	sfc() {
		ss=''
		while(a,args(),n) {
			if(n.eq(0)) {
				ss.add(a,'(')
				continue;
			} 
			if(n.gt(1)) ss.add(',')
			ss.add(a)
		}
		ss.add(')')
		return ss;
	} 
	@ws.pagePath(&s) {
		home = conf('web.rootPath')
		a=s.findLast('/')
		if(a) {
			b=s.trim()
			if(b.find('.')) {
				name = b.findLast('.').trim()
			} else {
				name = b
			}
		}
	} 
	@ws.addUrl(url, fc) {
		prev = map.get(url)
		if(typeof(prev,'function')) {
			not(fc) return;
			return print("이미등록된 URL 입니다 (경로:$url)");
		}
		not(fc) fc=@ws.page
		map.set(url, fc)
	}
	
	@ws.pagePath(&s) {
		home = conf('web.rootPath')
		log("s==>$s")
		if( s.find('/index.html') ) {
			a=s.findPos('/index.html')
			return Cf.val(home, a,".htm")
		}
		return Cf.val(home, s,".htm");
	}	
	
	@ws.addUrl(url, fc) {
		map = Baro.was().uriMap()
		prev = map.get(url)
		if(typeof(prev,'function')) {
			not(fc) return;
			return print("이미등록된 URL 입니다 (경로:$url)");
		}
		not(fc) fc=@ws.page
		map.set(url, fc)
		log("map -> $map")
	}
	@ws.setLastTagStr(param, &s) {
		s.findLast('<',1)
		val = s.trim()
		if( val) {			
			param.set('@lastTagStr', "<$val")
		}
		print("@@ setLastTagStr => $val")
	}
	@ws.getFnm(param, prefix) {
		idx = param.incrNum("@incIndex")
		not(prefix) prefix='incfc'
		return "${prefix}_${idx}";
	}
	
	@ws.parseTemplate(&s, param) {
		@ws.pushIncVars(param,"index")
		ss='', sp=0
		while(s.valid()) {
			c=s.ch()
			not(c) break;
			while( c.is('oper') ) {
				c=s.incr().ch()
				not(c) break
			}
			not(c) break;
			not(@ws.isWebTag(s) ) { 
				s.next()
				continue;
			}
			Cf.error(true)
			ep=s.cur()
			left = s.value(sp,ep)
			ss.add(left)
			@ws.setLastTagStr(param, left)
			sp = @ws.makeWebTag(param,s) not(typeof(sp,'number')) return;
			if(Cf.error()) {
				ss.add(Cf.error() )
				break	
			}
			if( sp<ep) {
				log("parse template end point error ($sp, $ep) ")
				break;
			}
			s.pos(sp)
		}
		ep=s.cur()
		if(sp<ep) ss.add(s.value(sp,ep))
		return ss;
	}
	@ws.getEl(&ref) {
		id=propVal(ref,'id')
		if(id) {
			el = "getEl('#$id')"
		} else {
			clsVal=''
			cls=propVal(ref,'class')
			cls.ref()
			while(cls.valid()) {
				a=cls.findPos(" \t",4).trim()
				not(a) break;
				clsVal.add(".$a")
			}
			if( clsVal) {
				el = "getEl('$clsVal')"
			} else {
				el = "getEl()"
			}
		}
		return el;
	}
	@ws.isWebTag(&s) {
		name = s.move()
		not( name.eq('$','inc', 'get','set','switch','if','case','default', 'while', 'effect', 'vars') ) return false;
		c=s.ch()
		not(c.eq('[') ) return false;
		return true;
	}
	@ws.makeWebTag(param,&s,parentEl) {
		not(s.ch()) return print("@@ make page var start error", s)
		name=s.move() if(name.eq('$')) name = 'get'
		c=s.ch() not(c.eq('[')) return print("@@ make page var Param error", s)
		p=s.match()
		c=s.ch()
		fc = call("ws.fc_$name")
	
		log("xxxxxxxxxx make page var xxxxxxxxx", name, fc, p)
		param.inject(@incStack)
		isRoot=false;
		not(parentEl) {
			if(incStack.size()==1 ) {
				isRoot=true;
				parentEl=@ws.getEl(param.ref('@lastTagStr'))
			}
		}
		not(parentEl) {
			return print("makeWebTag 오류 부모요소 미정의 name:$name")
		}
		if( typeof(fc,'func')) {
			ep = fc(param,s,p,parentEl)
		} else {
			ep = s.cur()
		}
		use(end)
		if(isRoot) {
			cur=incStack.get(0)
			param.appendText('@funcReady', cur.get('@incFuncName'), "($parentEl)", end)
		}
		log("xxxxxxxxxx $name xxxxxxxxx ep==$ep")
		return ep;
	}
	@ws.fc_inc(param,s,&p,parentEl) {
		param.inject(@incStack, @incMap, @srcStack)
		code = ''
		if( p.find(',') ) {
			code = p.findPos(",").trim()
		} 
		path = @ws.incPath(param,p)
		not(isFile(path)) {
			return print("inc 오류 : $path 파일 찾기오류")
		}
		not(code) {
			code = @ws.getFileName(path)
		}
		cur=@ws.pushIncVars(param, code)
		cur.set('@parentEl', parentEl)
		cur.set('@srcPath', path)
		pushArray(srcStack, '')
		srcStack.append(0,'const childEl={}', end)
		@ws.parseTag(param, fileRead(path), parentEl)
		src=srcStack.pop()
		fnm=@ws.getFnm(param,'fcinc')
		return s.cur()
	}
	@ws.fc_vars(param,s,p,parentEl) {
		param.inject(@funcNode, @incStack, inlineMode)
		cur = incStack.get(0)
		if(inlineMode) {
			cur.parseJson(p)
		} else {
			val = @json.nodeStr(cur)
			funcNode.append('ss', "addVars($val)\n")
		}
		log("@@ vars => ", val)
		return s.cur()
	}
	@ws.appendScript(param) {
		param.inject(@incStack)
		cur = incStack.get(0)
		ss=''
		while(a, args(1)) ss.add(a); 
		cur.apptenText('@incFuncSrc',  ss)
	}
	@ws.parseCtrl(param, &s, arr) {
		ss='';
		print("parse ctrl s===============$s")
		while(s.valid()) {
			c=s.ch()
			not(c) break;
			if(c.eq('(')) {
				a=s.match()
				ss.add('(',@ws.parseCtrl(param,a,arr),')')
				continue;
			}			
			if(c.eq()) {
				v=s.match()
				print("v==$v")
				ss.add(Cf.jsValue(v))
				continue;
			}
			if( c.is('oper')) {				
				s.incr()
				ss.add(c)
				continue;
			}
			sp=s.cur()
			c=s.next().ch()
			while(c.eq('.',':')) c=s.incr().next().ch();
			v=s.trim(sp,s.cur())
			if(arr) arr.add(v)
			ss.add('getVal("',v,'")')
		}
		return ss;
	}
	@ws.fc_if(param,s,p,parentEl) {
		param.inject(@funcNode, @incStack, @srcStack)
		ssize = incStack.size()
		sp = s.cur()
		if( asize==1 ) {
			ep = @ws.sub(param,s,parentEl)
			if( sp>ep ) return 0;
			val = s.value(sp,ep)
			if( @ws.checkTrue(param,p)) {				
				funcNode.append('ss', val)
			}
			return ep;
		}
		cur=incStack.get(0)
		not(typeof(cur,'node')) {
			print("@@ 컴포넌트 스택오류", s)
			return 0;
		}
		arr=_arr()
		pushArray(srcStack,'')
		srcStack.append(0, 'if(', @ws.parseCtrl(param,p,arr),') {' )
		srcStack.append(0, '}')
		src = srcStack.get(0)
		cur.appentText('@incFuncSrc', src)
		srcStack.pop()
		return ep;
	}
	@ws.fc_case(param,s,p,parentEl,type,pp) {
		log("call case => ", p, param, type, pp)
		return @ws.sub(param,s,parentEl)
	}		
	@ws.fc_switch(param,s,p,parentEl) {
		log("switch == $p start")
		use(ln,end)
		param.inject(@incStack, @srcStack)
		pushArray(srcStack,'')
		srcStack.append(0,"switch(getVal('$p')) {',end)
		while(s.valid()) {
			not(@ws.isPageVar(s)) break;
			sp = s.cur()
			name = s.move()
			if( name.eq('case','default')) {
				pp=s.match()
				if( name.eq('case')) {
					srcStack.append(0,"case $name:",ln)
				} else {
					srcStack.append(0,"default:",ln)
				}
				ep=@ws.case(param,s,pp,parentEl,name,p)
				srcStack.append(0,"break",end)
				log("switch case ep == $ep")
				not(ep) return 0;
				s.pos(ep)
			} else {
				return print("@@ switch end error $s")
				s.pos(sp)
				break;
			}
		}
		srcStack.append(0,"}",end)
		src=srcStack.pop()
		fnm=@ws.getFnm('switch')
		funcSrc="const ${fnm} = (parentEl) => {$src}"
		param.appendText("@jsReady", "setEffect(getVal('$p'),$parentEl,$fnm)", end)
		not(srcStack.size()) {
			param.appendText("@jsReady", "${fnm}($parentEl)", end)
		} else {
			cur.appendText("@incFuncSrc", funcSrc, ln)
		}
		return s.cur();
	} 
	@ws.getFileName(&s) {
		name = right(s,'/')
		return name.findPos('.').trim()
	}
	@ws.incPath(param,&s) {
		if( s.ch('/') ) {
			fullPath = Cf.val(conf('web.rootPath'), s)			
		} else {			
			s.start('./', true)
			sa = param.ref('@srcPath')
			sa=sa.findLast('/')
			while(s.valid()) {
				if(s.start('../', true) ) {
					if(sa) {
						sa=sa.findLast('/')
					}
				}
			}
			fullPath = Cf.val(sa, '/', p)
		}
		name = @ws.getFileName(fullPath)
		print("fullPath == $fullPath",  name)
		return fullPath
	}

	@ws.isSingleTag(tag, &s) {
		if(tag.eq('link','img','br') ) return true;
		a=s.findPos('>')
		c=a.ch(-1)
		if(c.eq('/')) return true;
		return false;
	}
	
	@ws.sub(param,&s,parentEl) {
		param.inject(@funcNode, @incStack, @srcStack)
		ssize = incStack.size()
		if( @ws.isWebTag(s)) {
			ep = @ws.makeWebTag(param,s,parentEl)
		} else {
			c=s.ch()
			log("make sub c==$c")
			if( c.eq() ) {
				s.match()
			} else if( c.eq('<')) {
				sp=s.cur()
				c=s.incr().next().ch()
				if(c.eq('-')) s.incr().next()
				ep=s.cur()
				tag = s.trim(sp+1,ep)
				log("tag == $tag")
				if(@ws.isSingleTag(tag,s)) {
					s.findPos('>',1)
				} else {
					s.pos(sp)
					a=s.match("<$tag", "</$tag>",8)
					if(typeof(a,'bool')) return print("@@ $tag match error ", s)
				} 
			} else {
				s.next()
			}
			ep=s.cur()
		}
		return ep;
	}
 
	@ws.paramVal(param) {
		c=s.ch(), v=null
		if(c.eq()) {
			v = s.match()	
		} else {
			sp = s.cur()
			c=s.next().ch()
			if(c.eq('.', ':')) {
				s.next().ch()
			}
			a = s.trim(sp,s.cur())
			v=@ws.varVal(a, param)
		}
		return v;
	}
	@ws.varVal(&s, param) {
		param.inject(@vars, @incStack, @incMap )
		root = incStack.get(0) not(root) root=param
		name = s.move()
		log("@@ var val ==> $name", name, vars)
		c=s.ch()
		if(c) {
			if(name.eq('global')) {
				root=vars
			} else if(c.eq(':','.')) {
				root = incMap.get(name)
			}
			name = s.incr().move()
		}
		not(typeof(root,'node')) return print("@ws.varVal node error", name, incStack)
		if(root.isVar(name)) {
			v=root.get(name)
		} else {
			v=vars.get(name)
		}
		return v;
	}
	
	@ws.checkOper(a,oper,b) {
		log("xxxxxxxxx check oper xxxxxxxx",a,oper,b)
		switch(oper) {
		case 1: return a.ne(b);
		case 2: return a.eq(b);
		case 3: return a.lt(b);
		case 5: return a.gt(b);
		case 4: return a.le(b);
		case 6: return a.ge(b)l
		case 7: if(a&&b) return true;		
		case 8: if(a||b) return true;
		default:
		}
		return false;
	}
	@ws.checkTrue(param, &s) {
		not(s.ch()) return false;
		a='',b='',c=s.ch()
		a=@ws.paramVal(param)
		oper=0
		c=s.ch(), cc=s.incr().ch()
		if(c.eq('!') ) {
			if(cc.eq('=')) oper=1
		} else if(c.eq('=')) {
			if(cc.eq('=')) oper=2			
		} else if(c.eq('<','>')) {
			if(cc.eq('=')) oper=4 else oper=3
			if(c.eq('>')) oper+=2
		} else if(c.eq('&','|')) {
			if(c.eq(cc)) {
				if(c.eq('&')) oper=7 else oper=8
			}
		}
		not(oper) return false;
		not(oper.eq(3,5) ) {
			s.incr()
		}
		b=@ws.paramVal(param)
		return @ws.checkOper(a,oper,b)
	}
	
</func>
