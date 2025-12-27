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
		this.var(page, page)
	}
	tray = @eval {
		src=cv('layout')
		a=cv('actions')
		node={}
		node.parseJson(cv('actions'))
		page=this.var(page)
		tray = page.tray()
		tray.contextMenu(node.trayActions)
		tray.icon('vicon:application_lightning')
		tray.show()
		print(">> node", page, node.trayActions, tray)
		this.var(tray, tray)
	}
	grid-add: @eval {
		this.inject(@page, @tray) 
		page.inject(@baseCode)
		splitSep(baseCode,':').inject(base, id)
		// 그리드 생성
		grid=object("grid.${base}:gridTest")
		grid.id='gridTest'
		grid.tag='grid'
		Cf.createWidget(grid)
		// 레이아웃에 추가
		vbox=page.child(0)
		vbox.addChild(grid)
		this.with(@grid)
	}
	grid-model = @eval {
		this.inject(@grid)
		grid.var(baseCode, "${base}:gridTest")
		grid.fields('name,addr,phone,email') 
		grid.update() 
	}
	node-test= @eval {
		this.inject(@grid)
		node={}
		node.parseJson(cv('data.actions'))
		this.set('actions', node)
		print(">> ", grid, this.actions )
	}
	
	@eval {
		o=cv('actions.trayActions')
		print("o->$o")
	}
	
##> data 	
	actions {
		trayActions: [
			{id:menu.apiForm, text:API관리, icon:'vicon:application_lightning', type:page}
			{id:menu.htmlTemplateForm, text:HTML템플릿관리, menuText:HTML템플릿, icon:'vicon:html_go', type:page}
			{id:menu.dbTableForm, text:DB테이블관리, icon:'vicon:database_gear', type:page}
			{id:menu.dbConnectMng, text:DB연결관리, icon:'vicon:database_connect', type:page}
			{id:menu.codeMng, text:공통코드관리, icon:'vicon:table_gear', type:page}
			{id:menu.appMenuMng, text:앱메뉴관리, icon:'vicon:application_view_tile', type:page}
			{id:menu.langMngPage, text:다국어관리, icon:'vicon:keyboard_magnify', type:page}
			{id:menu.close, text:프로그램 종료, icon:'icons:close'}
			{id:sourceEditor, text:실행툴열기, icon:'vicon:script_code'}
			{id:locknlock, text:락앤락툴열기, icon:'ficon:blog-tumblr'}
		]
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
