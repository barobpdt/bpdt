##> config
	test=@eval {
		// page = @apps.loadPage(cv('widget.layout'), 'test', 'EditorTest')
		page=page('test:EditorTest')
		this.var(page, page)
		print(">>", this, page)
	}
	nextword-test {
		// e.value(removeIndentText(cv('layout')))
		sp=e.pos()
		ep=e.pos('nextWord')
		while(sp.lt(ep), n) {
			s=e.text(sp,ep)
			print(s)
			sp=ep
			ep=e.pos(sp,'nextWord')
			if(n==100) break;
		}
	}
	event-add= @eval {
		p=this.var(page)
		e=p.get('e')
		event(e,'onContentChange', @editor.contentChange)
		event(e,'onCursorChange', @editor.cursorChange)
	}
	@eval {
		p=this.var(page)
		e=p.get('e')
		event(e,'onCursorChange', @editor.cursorChange, null, true)
		log(">> eval ", @editor.cursorChange)
	}
	
##> func 
	@editor.contentChange(pos, add, remove) {
		sp=this.pos(pos,'prevWord','prevChar')
		// b=this.pos(pos,'nextWord')
		aa=this.text(sp,pos+1)
		this.var(changeTick, System.tick())
		if( add==1 ) {
			c=aa.ch()
			if(c.eq('@')) {
				cc=aa.ch(-1)
				print(">> ", c,cc)
			} 
		}
		print("contentChange>> $pos, $add, $remove : [$aa]")
	}
	@editor.cursorChange() {
		print("@@ editor cursor change")
		// if( this.is('select') || this.is('wrapUse')) return;
		if(this.var(changeTick)) {
			dist=System.tick() - this.var(changeTick)
			print(">> dist==$dist")
			if(dist<500) return;
		}
		word=this.text('word')
		print("cursor change : $word")
	}

##> widget
layout<>
	<page id="EditorTest" module="EditorTestPage">
		<label id="top" height=30>
		<editor id="e" module="EditorTest">
		<hbox>
			<button id="run" text="실행" onclick() {page().runClick()}>
			<button id="save" text="저장" onclick() {page().saveClick()}>
			<space>
			<button id="close" text=닫기>
		</hbox>
	</page>
</>

	
##> module {name=EditorTestPage}
	init() {
		@e=widget('e')
	}
	runClick() {
		alert('runClick')
	}
	saveClick() {
		alert('saveClick')
	}
	
##> module {name=EditorTest>
	init() {
		print("init editor test")
	}