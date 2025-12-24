##> config
	test = @eval {
		not(typeof(@apps.loadPage,'func')) {
			print("apps.loadPage 함수 미정의", cv('layout') )
			return;
		}
		src=cv('layout')
		page = @apps.loadPage(src,'ConfigEditPage','main')
		print(".......... page ..........", page, src)
	}

	test1 = @eval {
		Cf.debug('clear')
		src=cv('widget.layout')
		page('test02:main').close()		
		page = @apps.loadPage(src,'test02','main')

		print(".......... page ..........", page, src )
	}
	@eval {
		src=cv('layout')
		print("src==$src")
	}

##> widget
layout: <>
	<page id="main" title="로그출력 페이지" module="ConfigEditPage">
		<splitter type="vbox">
	</page>

	// ##소스편집 
	<page id="ConfigEdit" margin="0,8,0,0" module="ConfigEditPanel">
		<hbox>
			<button id="searchFunc" text="함수찾기">
			<button id="configInfo" text="설정정보">
			<label id="editorTitle" stretch=1>
			<label text="찾기: "><input id="searchInput">
		</hbox>
		<editor id="editor" module={EditorSource}>
		<hbox>
			<button id="btnRun" text="실행" onClick() { page().runScript() }>
			<label id="editorInfo" stretch=1>
		</hbox>
	</page>
	
	// ##로그출력
	<page id="ConfigLog" margin="0,8,0,0" module="ConfigLogPanel">
		<hbox>
			<label id="logTitle" stretch=1>
			<label text="찾기: "><input id="searchInput" onEnter() { page().searchText(this)}>
		</hbox>
		<editor id="editor">
		<hbox>
			<button id="btnClear" text="지우기" onClick() { page().clearLog() }>
			<label id="logInfo" stretch=1>
		</hbox>
	</page> 
</>


##> module {name=ConfigEditPage}
	init() {
		@editPanel  = page("ConfigEdit")
		@logPanel   = page("ConfigLog")
		@splitter   = widget("splitter")
		splitter.addPage(editPanel)
		splitter.addPage(logPanel)
		this.positionLoad()
		this.timer(200, this.setSplitterSize)
	}	
	onActivationChange() {
		if( this.is('active') ) {
			editPanel.editorFocus();
		}
	}
	onClose() {
		this.positionSave();
	}
	onTimer() {
		str=logReader('sourceRun').timeout();
		if(str) logPanel.appendLog(str)
	}
	editorFocus() {
		editPanel.editorFocus()
	}
	setSplitterSize() {
		total=splitter.sizes().sum();
		if(total>600) {
			arr=recalc(total, "550,*");
		} else {
			arr=recalc(total, "7,3")
		}
		splitter.sizes(arr);
		print("set splitter sizes==>", arr );
	} 

##> module {name=ConfigEditPanel}
	initPage() {
		@editor=widget('editor')
		@editorTitle=widget('editorTitle')
		@editorInfo=widget('editorInfo')
		@searchInput=widget('searchInput')		
		
		setEvent(searchInput,'onEnter',this.searchText, this)
		this.setEditorSyntax() 
		this.setKeyMap()
		editorTitle.value("소스테스트 실행창")
	}
	editorFocus() {
		this.timer(250, func() { editor.focus() })
	}
	searchFocus() {
		page().active()
		this.timer(50, func() { searchInput.focus() })
	}
	runScript(key) {
		sel=editor.is("select");
		not(key) key=when(sel,KEY.R, KEY.B);
		if( key.eq(KEY.Return, KEY.Enter) ) {
			if(sel) {
				line=editor.text("select");
				editor.movePos("selectEnd");
			} else {
				line=editor.sp('lineStart').spText("lineEnd");
				editor.movePos("lineEnd").insert("\r\n");
			}
			return Cf.sourceCall(line, true);
		} 
	}
	setEditorSyntax() {
		node=Cf.getObject("syntax","default");
		not(node) {
			node=Cf.getObject("syntax","default",true);
			node.parseJson(conf("editor.syntax"));
		}
		editor.syntax(node);
	}
	searchText() { 
		str=searchInput.value();
		if(str) global().set("prevSearchValue",str);
		this.editorFocus()
		editor.editorSearch(0);	
	}

##> module {name=ConfigLogPanel}
	initPage() {
			@editor=widget('editor')
			@logTitle=widget('logTitle')	
			@logInfo=widget('logInfo')
		}
		appendLog(str) {
			editor.append(str, true);
		}
		clearLog() {
			editor.clear();
			Cf.debug('clear')
			page('editPanel').editorFocus()
		}
		apiResult(socket, uri, &data, param) {
			this.appendLog("$uri => $data")
		}
