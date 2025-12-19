_page(pcode,src) {
	p = Cf.getObject('page', pcode)
	s=stripJsComment(src)
	s.ref()
	c=s.ch()
	if(p) {
		not(c.eq('<')) p[$s]
		return p;
	}
	not(c.eq('<')) return print("$pcode 페이지 생성 오류 (페이지 소스가 없습니다)");
	if(pcode.find(':')) {
		pcode.split(':').inject(base, name)
	} else {
		base='test'
		name=pcode
	}
	
	sp=s.cur()
	tag=s.incr().move()
	s.pos(sp)
	ss=s.match("<$tag", "</$tag>")
	if(typeof(ss,'bool')) return print("$pcode 페이지 태그 매칭오류")
	prop=ss.findPos('>')
	src=_s('<widgets base="$base"><$tag id="$name" $prop>$ss</$tag></widgets>')
	Cf.sourceApply(src)
	p = Cf.getObject('page', "$base:$name")
	not(typeof(p,'widget')) return print("$pcode 페이지 생성 오류 (페이지 소스가 없습니다)");
	if(s.ch()) {
		p[$s]
	}
	if(p.init ) {
		p.init()
	}
	return p;
}

## 파이션 브라우저 열기
```javascript
@baro.main() {
	p = _page('app:main', V[
		<page margin="0">
			<div id="container">
		</page>
		init() {
			@container= this.get('container')
			this.size(800,600)
			this.open()
		}
		addStack(page) {
			container.addPage(page, true)
		}
	])
	p.addStack( @baro.pageChatbot() )
	return p;
}
@baro.pageChatbot() {
	p =_page('app:chatbot', V[
		<page>
			<canvas id="c">
			<hbox>
				<button id="ok" text="ok">
				<space>
			</hbox>
		</page>
		
		init() {
			@cmd = cmd('python')
			@in = logWriter('webvew-in')
			@out = logReader('webview-out')
			@python = _s('${@python.path}/python')
			@srcPath = _s('${@sample.path}/apps')
			@canvas = this.get('c')
			@btnOk = this.get('ok')
			print("## init line==>",python,srcPath)	
			this.setPage()
			this.setPageEvent()
			this.setWebview()
			this.timer(50)
		}
		onTimer() {
			log = out.timeout()
			if( log ) {
				print("@@ timer log == $log")
				this.parseWebLog(log)
			}
		}
		onResize() {
			canvas.geo().inject(x,y,w,h)
			line = _s('##> geo:$:x,$:y,$:w,$:h,0')
			print("xx resize xx", line, x, y)
			in.append(line)
		}
		onClose() {
			this.killTimer()
			in.append('##> quit:')
		}
		setPage() {
			box=this.child(0)
			box.margin(0)
			hbox=box.child(1)
			hbox.margin(4,2,2,2)
		}
		setPageEvent() {
			_event(canvas,'onDraw', this.drawCanvas, this)
			_event(btnOk, 'onClick', this.clickBtnOk, this)
		}
		setWebview() {
			line=_s('$python "$srcPath/webpage.py" --log "${in.logFileName}" --out "${out.logFileName}"')
			cmd.cmdAdd(this, line, this.webviewStarted )
		}
		drawCanvas(dc,rc) {
			dc.fill('#344')
		}
		clickBtnOk() {
			alert('clickBtnOk')
		}
		webviewStarted(&s) {
			print("@@ web view start => $s")
		}
		parseWebLog(&s) {
			if(s.find('##> start:')) {
				winId = canvas.winId()
				in.append("##>setParent:${winId}")
			}
		}
	])
	return p;
}
```
