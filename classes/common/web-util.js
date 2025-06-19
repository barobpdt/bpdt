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
	 		not(s.ch()) {
				ss.add(left)
				break;
			}
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		src=s.match() if(typeof(src,'bool')) continue;
			@web.parseTemplateVar(src,fn,param,left)
	 	}
		if(checkStart) {
			if(param.isValid('css')) ss=@web.addCss(ss,fn,param)
			if(param.isValid('script')) ss=@web.addScript(ss,fn,param)
		}
	 	return ss;
	}

	@web.isTemplateVar(&s) { 
		not(s.ch()) return;
		c=s.next().ch()
		if(c.eq('-','.','#')) c=s.incr().next().ch()
		if(c.eq('[')) return true;
	}
	@web.parseTemplateVar(&s,fn,param,left) { 
		not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
		val = ''
		not( @web.isTemplateVar(s) ) ss.add(s.trim())
		type = s.findPos('[',0,1).trim()
		addCheck = true
		a=s.match(1)
		switch(type) {
		case set: 
			cur=param.addNode('@set')
			cur.set(a,s)
		case eval:
			eval(a,fn,param)
		case html:
		case innerHtml:
			
		case var: 
			param.parseJson(s)
		case css:
		case script:
		case include:
			path = param.val('webpageFileName').findLast('/').trim()
			while(src.valid()) {
				line = src.findPos("\n").trim()
				not(line) continue;
				@web.parseInclude(fileRead("$path/$line"),fn,param)
			}
		default:
		}
		if(addCheck) ss.add(left)
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
 