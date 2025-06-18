~~
include('classes/common/json')
@ws.addUrl('/pages/app', @ws.page)
 
~~
<func>
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
	@ws.page(req, param) {
		srcPath = @ws.pagePath(req.getValue('url'))
		log("web page path => $srcPath")
		src=fileRead(srcPath)
		param.set('@srcPath', srcPath)
		req.send(@ws.parseTemplate(src, param)) 
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
	 
	@ws.setIncVars(cur, code, param) {
		fidx = param.incrNum("@funcIndex")
		cur.set('@incCode', code)
		cur.set('@incFuncName',"${code}_$fidx")
		cur.set('@incFuncSrc','')
		cur.set('@incFuncScript','')
	}
	@ws.parseTemplate(&s, param) {
		stack=param.addArray('@incStack')
		map = param.addNode('@incMap')
		cur = map.addNode('index')
		@ws.setIncVars(cur,"index",param)
		pushArray(stack,cur)
		param.set('@funcNode', Cf.funcNode())
		
		ss='', sp=0
		while(s.valid()) {
			c=s.ch()
			not(c) break;
			while( c.is('oper') ) {
				c=s.incr().ch()
				not(c) break
			}
			not(c) break;
			not(@ws.isPageVar(s) ) { 
				s.next()
				continue;
			}
			ep=s.cur()
			ss.add(s.value(sp,ep))
			Cf.error(true)
			sp = @ws.makePageVar(s, param) not(typeof(sp,'number')) return;
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
	@ws.isPageVar(&s) {
		name = s.move()
		not( name.eq('$','inc', 'get','set','switch','if','case','default', 'while', 'effect', 'vars') ) return false;
		c=s.ch()
		not(c.eq('[') ) return false;
		return true;
	}
	@ws.makePageVar(s, param) {
		not(s.ch()) return print("@@ make page var start error", s)
		name=s.move() if(name.eq('$')) name = 'get'
		c=s.ch() not(c.eq('[')) return print("@@ make page var Param error", s)
		p=s.match()
		c=s.ch()
		fc = call("ws.$name")
	
		log("xxxxxxxxxx make page var xxxxxxxxx", name, fc, p)
		if( typeof(fc,'func')) {
			ep = fc(s, p,param)
		} else {
			ep = s.cur()
		}
		log("xxxxxxxxxx $name xxxxxxxxx ep==$ep")
		return ep;
	}
	@ws.vars(s,p,param) {
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
	@ws.switch(s,p,param) {
		log("switch == $p start")
		while(s.valid()) {
			not(@ws.isPageVar(s)) break;
			sp = s.cur()
			name = s.move()
			if( name.eq('case','default')) {
				pp=s.match()
				ep=@ws.case(s, pp,param, name, p)
				log("switch case ep == $ep")
				not(ep) return 0;
				s.pos(ep)
			} else {
				return print("@@ switch end error $s")
				s.pos(sp)
				break;
			}
		}
		return s.cur();
	}
	@ws.inc(s,p,param) {
		log("ws inc[$p] ")
		return s.cur()
	}
	@ws.case(s,p,param,type,pp) {
		log("call case => ", p, param, type, pp)
		return @ws.sub(s,param)
	}	
	@ws.isSingleTag(tag, &s) {
		if(tag.eq('link','img','br') ) return true;
		a=s.findPos('>')
		c=a.ch(-1)
		if(c.eq('/')) return true;
		return false;
	}
	
	@ws.sub(s,param) {
		if( @ws.isPageVar(s)) {
			ep = @ws.makePageVar(s,param)
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
					a=s.match("<$tag", "</$tag>")
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
	@ws.checkTrue(&s, param) {
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