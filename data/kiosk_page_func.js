dev:main.onInit() {
	pi=newClass('common.EditorSrc', this);
	this.size(800,750);
}
dev:main.close.onClick() {
	this.hide();
}
dev:main.run.onClick() {
	s=this.src.value().str();
	if( s.find(">>") ) {
		a=s.findPos(">>");
		var=a.findLast("\n").right().trim();
		src=a.findLast("\n").left().value();
		while( s.valid() ) {
			not(s.ch() ) break;
			fnm=s.move().trim();
			param=s.match(); 
			body=s.match();
			src.add("${var}.function('$fnm', func($param) { $body });\n");
			ch=s.ch();
			if( ch.eq('>') ) {
				s.incr(2);
				break;	
			}
			if( ch.eq(';') ) s.incr();
		}
		if( s.valid() ) src.add(s);
		print("RUN >> $src");
		Cf.call(src);
	} else {
		Cf.call(s);
	}
}
dev:main.src.onKeyDown() {
	if( pi.editorKeyDown(@key, @mode) ) return true;
	if( @mode&KEY.ctrl ) {
		switch(@key) {
		case KEY.D: 	this.hide();
		case KEY.S:		this.fireEvent('run.onClick');
		case KEY.M:	 	this[src].match();
		case KEY.L:		this[src].insert( conf('devShortCut.genEvent'), 	true );
		case KEY.F:		this[src].insert( conf('devShortCut.genFunc'), 		true );
		case KEY.W:	this[src].insert( conf('devShortCut.genWidget'), 	true );
		default: return false;
		}
	}
}
DevFuncEdit:main.setClassFunc(cls, funcNm) {
	pageClass.makeComboData(cls);
	pageClass.showAll();
	classFuncCombo.value(funcNm);
}
DevFuncEdit:main.makePageFuncTab(funcName) {
	not( pageFuncPage ) {
		@pageFuncPage=pageLoad('DevFuncEdit.pageFuncTab');
		this.initSrcAction( pageFuncPage );
	}
	this.addPageFuncPage(funcName); 
}
DevFuncEdit:main.makePageSourceTab() {
	/* 페이지 소스 페이지 처리 */
	not( pageSourcePage ) {
		// impl => dev.PreviewFunctionImpl
		@pageSourcePage=pageLoad('DevFuncEdit.pageSourceTab');
	}
	this.tab.addPage(pageSourcePage, '페이지 소스','vicon.database_table', true);
}
DevFuncEdit:main.onInit() {
	mainPage=this;
	pageCodeNode=null;
	classImplPage=null;
	pageFuncPage=null;
	userFuncPage=null;
	confManagerPage=null;
	coreFuncPage=null;
	pageSourcePage=null;
	db=instance('pages.model');
	help=instance('help.model');
	not( help.open() ) help.open('data/help.db');
	pageSelect=newClass('dev.pageSelectCombo', this);
	projectSelect=newClass('dev.projectSelectCombo', this);
	projectVar=newClass('dev.projectSelectCombo', this);
	pageVar=newClass('dev.pageVarCombo', this, this[pageVarCombo], this[pageVarPlus] );
	pageFunc=newClass('dev.pageFuncCombo', this, this[pageFuncCombo] );
	pageClass=newClass('dev.pageClassCombo',this, this[inheritCombo], this[classVarCombo], this[classFuncCombo]);
 
	this.makePageFuncTab('new');
	this.makeClassImplTab();
	this.pageFuncCombo.findLayout().hideAll();
	this.pageEventCombo.findLayout().hideAll();
	this.pageEventCombo.delegate(true, 24);

 	this.pageGroup.func(tr('event#combo.onKeyDown#showPopup','pageGroup'));
 	this.pageCode.func(tr('event#combo.onKeyDown#showPopup','pageCode'));
 	this.pageFuncCombo.func(tr('event#combo.onKeyDown#showPopup','pageFuncCombo'));
 	this.pageVarCombo.func(tr('event#combo.onKeyDown#showPopup','pageVarCombo'));
 	this.classFuncCombo.func(tr('event#combo.onKeyDown#showPopup','classFuncCombo'));
}
DevFuncEdit:main.imageOpen.onClick() {
	pageLoad("common.image").open(this,'center');
}
DevFuncEdit:main.confManagerOpen.onClick() {
	pageLoad("confManager").open(this,'center');
}
DevFuncEdit:main.devPageOpen.onClick() {
	pageLoad("devTool.pages").open();
}
DevFuncEdit:main.pageEventComboDraw(draw, index, over) {
	node= class('draw').comboDraw(this.pageEventCombo, draw, index, over, this.pageEventVal, 'event_nm');
	not( node ) return;
	rc=draw.rect();
	rcIcon=rc.width(20).center(16,16);
	rc.incrX(20);
	icon=when( node[tag].eq('common'), 'award_star_silver_1','award_star_gold_1');
	draw.icon(rcIcon, "vicon.$icon");
	draw.font(10).text(rc, node[event_nm]);
	if( node[event_param] ) {
		w=draw.textWidth(node[event_param]) + 15;
		draw.font(8).text(rc.move('end',w), "($node[event_param])", 'right');
	}
}
DevFuncEdit:main.pageEventCombo.onDraw() {
	this.pageEventComboDraw(@draw, @index, @over);
}
DevFuncEdit:main.pageEventCombo.onChange() {
	val=@me.value();
	print("xxxxxx $val xxxxxxx");
}
DevFuncEdit:main.addPageFuncPage(funcName) {
	not( funcName ) return;
	this.tab.addPage(pageFuncPage,'페이지 함수','vicon.bricks_defalut', true);
 	node=pageCodeNode.addNode();
	not( node ) node={};
	if( funcName.eq('onInit') ) {
		sort=1;
	} else if( funcName.start('on') ) {
		sort=2;
	} else if( funcName.find('.') ) {
		sort=3;
	} else {
		sort=4;
	} 
	node.varMap(pageCodeNode, 'runtimePage, cmsCode, pageCode: code');
	node.put(funcName, sort);
	not( node[cmsCode] )	node[cmsCode]=pageSelect.pageGroupVal;
	not( node[pageCode] )	node[pageCode]=pageSelect.pageCodeVal;
	print("@@ addPageFuncPage: $funcName, node => $node @@");
	if( funcName.eq('layout', 'new') ) {
		if( funcName.eq('new') ) {
			node.sort=9;
		} else {
			node.sort=0;
			db.fetch("select layout as src from pageLayout where cmsCode=#{cmsCode} and pageCode=#{pageCode}",node);
		}
	} else if( node[cmsCode] && node[pageCode] ) {
		db.fetch("select funcData as src, funcParam, note, type, sort from pageFunc where cmsCode=#{cmsCode} and pageCode=#{pageCode} and funcName=#{funcName} ", node);
	} else {
		return;
	}
	this.addPageFuncEdit(node);
}
DevFuncEdit:main.addPageFuncEdit(node) {
	tab=pageFuncPage.tab;
	luid=tr("[#].[#].[#]", node.cmsCode, node.pageCode, node.funcName);
	while( cur, tab.widget() ) {
		if( cur.luid.eq(luid) ) {
			tab.current(cur);
			return;
		}
	}
	page=this.widget(tr('widget#editor.dev#pageFunc'));
	// impl => dev.EditSrcImpl
	tabBtn=page.widget({tag:toolbutton, 
		onInit() {
			srcNode=null;
		}
		onClick() {
			mainPage[funcTabPage]=srcNode.page;
			str="tab.close, tab.closeOther,-, tab.save, tab.run, -, tab.deleteFunc";
			page.menu(str, 12);
		}
		initButton(node) {
			switch( node[sort] ) {
			case 0: icon="ficon.application-plus-black";
			case 1: icon="ficon.document-code";
			case 2: icon="ficon.document-globe";
			case 3: icon="ficon.document-epub";
			case 4: icon="ficon.document-number";
			case 5: icon="ficon.document-outlook";
			case 9: icon="ficon.application-table";
			}
			this.icon(icon);
			@srcNode=node;
		}
 	});
 	node.page=page;
 	tabBtn.initButton(node);
 	page.initPage(node);
	if( node.sort.eq(9) ) {
		tab.addPage(page, '새페이지', null, true);
		page.luid="newPage";
	} else {
		tab.addPage(page, node[funcName], null, true);
		page.luid=luid;
	}
	tab.tabButton(page, tabBtn, 'left');
	return page;
}
DevFuncEdit:main.makeUserFuncTab() {
	/* 사용자 함수 처리 */
	not( userFuncPage ) {
		// impl => dev.PreviewFunctionImpl
		@userFuncPage=this.widget(tr('page#preview.dev#userFunc'));
	}
	this.tab.addPage(userFuncPage, '사용자 함수','ficon.script-stamp', true);
}
DevFuncEdit:main.pageVarComboChange(val, combo) {
	not( val ) return;
	root=combo.rootNode();
	cur=root.findOne('id', val);
	print("@@ cur===========$cur");
	pageClass.hideAll();
	this.pageEventCombo.findLayout().hideAll();
	if( cur[tag].eq('class') ) {
		page=pageVar.currentPage;
		cls=page[$val];
		pageClass.makeComboData(cls);
		pageClass.showAll();
	} else if( cur[type].eq('widget') ) {
		not( cur[tag] ) return;
		this.pageEventCombo.findLayout().showAll();
		not( eventNode ) eventNode={}; 
		eventNode.removeAll();
		eventNode[tag]=cur[tag];
		help.fetchAll("select tag, event_nm, event_param from core_object_event where tag=#{tag}", eventNode.removeAll() );
		help.fetchAll("select tag, event_nm, event_param from core_object_event where tag='common'", eventNode  );
		this.pageEventCombo.removeAll().addItem(eventNode,'event_nm','=이벤트 선택=');
		maxStr='';
		while( sub, eventNode ) {
			text="$sub[event_nm]\t$sub[event_param]";
			if( maxStr.size()< text.size() ) maxStr=text;
		}
		this.pageEventCombo.addText(maxStr, true);
	} 
}
DevFuncEdit:main.pageFuncComboChange(val, combo) {
	not( val ) return;
	this.addPageFuncPage( val)
}
DevFuncEdit:main.helpOpen.onClick() {
	System.run('http://localhost:8088/@help.coreFunc');
}
DevFuncEdit:main.pageOpen.onClick() {
	/* 페이지 열기 기능 */
	pageSelect.inject( pageGroupVal, pageCodeVal);
	if( pageGroupVal && pageCodeVal ) {
		page=pageLoad("${pageGroupVal}.${pageCodeVal}");
		if( page.layout ) {
			page.open();
			pageSelect.pageCodeChange();
		} else {
			not( this.confirm("페이지 정보가 없습니다 신규 페이지를 생성할까요?") ) {
				return;
			}
		}
	} else {
		this.alert("페이지 코드를 입력하세요");
	}
}
DevFuncEdit:main.pageSelectChange(cur) {
	page=cur.runtimePage;
	not( page ) {
		@pageCodeNode=null;
		return;
	}
	@pageCodeNode=cur;
	pageVar.makeComboData(cur.runtimePage);
	pageFunc.makeComboData(cur.runtimePage);
	this.pageFuncCombo.findLayout().showAll();
	pageClass.hideAll();
	this.pageEventCombo.findLayout().hideAll();
	this.addPageFuncPage('layout');
	this.pageFuncCombo.focus();
}
DevFuncEdit:main.classFuncComboChange( cur) {
	not( cur ) return;
	classImplPage.pageImpl.initPage(cur); 
	this.tab.current( classImplPage );
}
DevFuncEdit:main.makeClassImplTab() {
	not( classImplPage ) {
		// impl => dev.PreviewFuncImpl
		@classImplPage=this.widget(tr('page#preview.dev#classImpl'));
	}
	this.tab.addPage(classImplPage, '페이지 구현 클래스','ficon.application-blog', flagCurrent);
}
DevFuncEdit:main.initSrcAction(page) { 
	page.action([
		{id: 'tab.close',				text: 탭닫기,			icon:ICON.vicon.cancel_defalut },
		{id: 'tab.closeOther',		text: 다른 탭닫기,	icon:ICON.vicon.application_form_delete },
		{id: 'tab.save',				text: 저장,				icon:ICON.vicon.database_save },
		{id: 'tab.run',					text: 실행,				icon:ICON.vicon.monitor_go },
		{id: 'tab.deleteFunc',		text: 함수삭제,		icon:ICON.vicon.brick_delete },
	]); 
	tab=page.tab;
 	page.action('tab.close').trigger(callback() {
		print("tab.close ======> $tab");
 		cur=mainPage[funcTabPage];
		tab.remove(cur);
		not( tab.count() ) {
		}  
	}); 
 	page.action('tab.closeOther').trigger(callback() {
 		cur=mainPage[funcTabPage];
		while( sub, tab.widget() ) {
			if( sub==cur ) continue;
			tab.remove(sub);
		}
	}); 
}
DevFuncEdit:main.pageSourceLoad.onClick() {
	popup=this.pageSourcePopup;
	not( popup ) {
		popup=pageLoad('DevFuncEdit.pageSourceLoadPopup');
		this.pageSourcePopup=popup;
	}
	popup.initPage();
	popup.open(this,'center');
}
DevFuncEdit:main.pageSourceLoadOk(node) {
	not( node.fullPath ) {
		this.alert("페이지 소스 경로가 없습니다");
		return;
	}
	if( node[pageGroup] ) {
		val=this.pageGroup.value();
		not( val.eq(node[pageGroup]) ) {
			class('widget').comboReload(this.pageGroup, node[pageGroup]);
			if( node[pageCode] ) {
				this.pageCode.value(node[pageCode]);
			}
		}
	}
	this.makePageSourceTab();
	pageSourcePage.initPage(node);
}
DevFuncEdit:main.editorUserFunctionClick(funcNm, param ) {
	this.makeUserFuncTab(); 
	userFuncPage.initPage({funcName: $funcNm, funcParam: $param });
}
FuncManager:funcTree.initPage() {
	this.sort_field.addItem( class('code').getCodeNode('funcSortField'),'code,value' );
	this.sort_order.addItem( class('code').getCodeNode('sortOrder'),'code,value' );
	this.func_type.addItem( class('code').getCodeNode('funcSrcType'), 'code,value', '==전체==' );
	tree.update();
}
FuncManager:funcTree.drawTree(d, node, over ) {
	rc=class('draw').treeIcon(tree, d, node, over, node[type].eq('funcSrc') );
	rcIcon = rc.width(18).center(16,16);
	rc.incrX(20);
	if( node.state(NODE.add) ) {
		d.save().pen('#f05050');
		d.text(rc.width(9),'*', 'center');
		rc.incrX(10);
		d.restore();
	}
	switch( node[type] ) {
	case root:
		d.icon( rcIcon, "vicon.script_code_red" );
		d.text( rc,  node[title]);
	case funcGroup:
		d.icon( rcIcon, "vicon.table_gear" );
		d.text(rc,  node[func_grp]);
	case funcSrc:
		d.icon(rc.move('end',24).center(16,16), 'vicon.application_form_edit');
		switch( node[func_type] ) {
		case 'A':	d.icon( rcIcon, "ficon.script-code" );
		case 'C':	d.icon( rcIcon, "ficon.script-globe" );
		case 'S':	d.icon( rcIcon, "ficon.script-block" );
		case 'T':	d.icon( rcIcon, "ficon.script-attribute-t" );
		case 'Z':	d.icon( rcIcon, "ficon.script--exclamation" );
		}
		d.text( rc,  "${node[func_nm]}(${node[func_param]})");
	default:	
	} 
}
FuncManager:funcTree.onInit() {
	db=instance('pages.model');
	tree=this.tree;
	tree.check('treeMode', true);
	tree.check('editTrigger', VIEWEDIT.DoubleClicked);
	tree.check('sortEnable', true);
	tree.model( instance('funcInfo.model'), 'title');
	sortField='func', sortAsc=true;
	
	currentNode=null, currentType=null;
	this.initFuncAction();
	this.initPage();
	this.delay(callback() {
		root=tree.rootNode().child(0);
		tree.expand(root, true, true);				
	});
}
FuncManager:funcTree.initFuncAction( page ) { 
	not( page ) page=this;	
	page.action([
		{id: 'func.add',				text: 함수추가,			icon:ICON.vicon.application_edit },
		{id: 'func.delete',			text: 함수삭제,			icon:ICON.vicon.delete_default },
		{id: 'func.reload',			text: 새로고침,			icon:ICON.vicon.arrow_rotate_clockwise },
	]); 
 	page.action('func.add').trigger(callback() {
		cur = page[contextNode];
		sub=null;
		switch( cur[type] ) {
		case root:
			sub=cur.addNode({type:funcGroup, func_grp: 함수그룹 추가});
		case funcGroup:
			sub=cur.addNode({type:funcSrc, func_nm: 함수 추가});
		case funcSrc:		
			cur=cur.parent();
			sub=cur.addNode({type:funcSrc, func_nm: 함수 추가});
		default:
			not( cur ) {
				cur=tree.rootNode().child(0);
				sub=cur.addNode({type:funcGroup, func_grp: 함수그룹 추가});
			}
		}
		if( sub[type].eq('funcSrc') ) {
			type=page.func_type.value();
			not( type ) type='A';
			sub[func_grp]=cur[func_grp];
			sub[func_type]=type;
		}
		if( sub ) {
			page.addTreeNode(cur, sub);
		}
	});
	page.action('func.delete').trigger(callback() {
		cur = page[contextNode];
		if( cur[type].eq('funcSrc') ) {
			not( page.confirm("$cur[func_nm] 함수를 삭제하시겠습니까?") ) {
				return;
			}
			db.exec("update cmsFunc set useyn='N' where cmsCode=#{func_grp} and funcName=#{func_nm}", cur);		
		} else if( cur[type].eq('funcGroup') ) {
			not( page.confirm("$cur[func_grp] 함수그룹을 삭제하시겠습니까?") ) {
				return;
			}
			db.exec("update cmsFunc set useyn='N' where cmsCode=#{func_grp}", cur);	
		}
		if( cur.state(NODE.add) ) {
			cur.parent().remove(cur);
		} else {
			cur.parent().removeAll();
		}
		funcSrcTabPage.removeSrc(cur);
		tree.update();		
	});	
	page.action('func.reload').trigger(callback() {
		cur = page[contextNode];
		cur.removeAll();
		tree.update();
	});	 
}
FuncManager:funcTree.expandClose.onClick() {
	cur=tree.rootNode().child(0);
	treeExpandAll(tree, cur, false);
	tree.current(cur);
}
FuncManager:funcTree.tree.onChildData() {
	node=@node;
	not( node.parent() ) {
		node.addNode({ type:root, title: 공통함수 정보});
		return;
	}
	switch( node[type]) {
	case root:
		db.fetchAll("select 
			'funcGroup' as type, 
			cmsCode as func_grp, max(tm) as tm
		from cmsFunc 
		where useyn='Y' group by cmsCode ", node );
	case funcGroup:
		db.fetchAll("select 
			'funcSrc' as type, 
			cmsCode as func_grp, funcName as func_nm, funcParam as func_param, type as func_type, tm
		from cmsFunc 
		where cmsCode=#{func_grp} and useyn='Y'", node );
	}
}
FuncManager:funcTree.tree.onLessThan() {
	switch( sortField ) {
	case func: 
		switch( @left[type] ) {
		case funcGroup: 
			if( currentNode[type].eq('root') ) {
				return @left.lessThan('func_grp', @right, sortAsc); 
			}
			return;
		default: return @left.lessThan('func_nm', @right, sortAsc);
		}
	case tm:
		return @left.lessThan('tm', @right, sortAsc);
	default: 
		return;
	}
}
FuncManager:funcTree.tree.onFilter() {
	not( currentType ) return true;
	type=@node[type];
	if( type.eq('funcSrc') ) {
		if( currentType.eq(@node[func_type]) ) return true;
		return false;
	}
	return true;
}
FuncManager:funcTree.tree.onTip() {
	return @node[note];
}
FuncManager:funcTree.tree.onDraw() {
	this.drawTree(@draw, @node, @over);
}
FuncManager:funcTree.tree.onChange() {
	this.status.redraw();
	if( this.editStart ) return;
	this.treeChanged( @node );
}
FuncManager:funcTree.tree.onMouseDown() {
	if( @button.eq('right') ) return 'ignore';
	node = @me.at(@pos);
	rc = @me.nodeRect(node);
	rcIcon = rc.move('end',24).center(18,18);
	if( rcIcon.contains(@pos) ) {
		page=null;
		if( node[type].eq('funcSrc') ) {
			page=Cf.loadPage('devPopup.funcSrc');
		}
		if( page ) {
			page.initPage(node, this, @pos, tree, 720, 560);
			return 'ignore';
		} 
	}
}
FuncManager:funcTree.addTreeNode(node, sub) {
	not(sub) return;
	this.editStart=true;
	sub.state(NODE.add, true);
	tree.update();
	tree.expand(node, true);
	tree.current(sub);
	tree.edit(sub);
	tree.scroll(sub);
}
FuncManager:funcTree.tree.onEditEvent(type, node, data, index) {
	ty=node[type];
	switch(type) {
	case create: 
		if( ty.eq('root') ) return;
		input=@me.widget({tag:input});
		this.delay( callback() {
			if( ty.eq('funcSrc') ) {
				if( node[func_nm] ) {
					input.value("${node[func_nm]}(${node[func_param]})");
				}
			} else {
				input.value(node[func_grp]);
			}
			input.select();
		});
		return input;
	case geometry: 
		rc = data.incrX(18);
		return rc;
	case finish: 
		same=false;
		if( ty.eq('funcSrc') ) {
			if( data.eq("${node[func_nm]}(${node[func_param]})") ) {
				same=true;
			}
		} else {
			if( node[func_grp].eq(data) ) {
				same=true;
			}
		}
		if( same ) {
			return;
		}
		not( data ) {
			this.delay( callback() {
				node.parent().remove(node);
				tree.update();
			});
			return;
		}
		not( node.state(NODE.add) ) {
			node.state(NODE.modify, true);
		}
		_func=func(s) {
			if( s.find('(') ) {
				node[func_nm]=s.findPos('(').trim();
				node[func_param]=s.findPos(')').trim();
			} else {
				node[func_nm]=s, node[func_param]='';
			}
		};
		field='func_nm';
		if( ty.eq('funcSrc') ) {
			_func(data.ref());
			data=node[func_nm];
		} else {
			node[func_grp]=data;
			field='func_grp';
		}
		cancel=false, parent=node.parent();
		while( cur, parent ) {
			if( cur==node ) continue;
			if( cur[$field].eq(data) ) {
				cancel=true;
				break;
			}
		}
		if( cancel ) {
			if( ty.eq('funcSrc') ) {
				mainPage.alert("중복된 함수정보가 있습니다.");
			} else {
				mainPage.alert("중복된 함수그룹 정보가 있습니다.");
			}
			this.delay( callback() {
				tree.edit(node);
			});
		}
		this.editStart=false;
		@me.update(); 
	default: return;
	}
}
FuncManager:funcTree.sort_field.onChange() {
	@sortField=@me.value(); 
	tree.update();
}
FuncManager:funcTree.sort_order.onChange() {
	val=@me.value();
	@sortField=this.sort_field.value(); 
	@sortAsc=when( val.eq('asc'), true, false );
	tree.update();
}
FuncManager:funcTree.func_type.onChange() {
	@currentType=@me.value();
	tree.update();
}
FuncManager:funcTree.treeChanged(node) {
	if( node.childCount() ) {
		tree.expand(node);
	}
	@currentNode=node;
	switch( node[type] ) { 
	case funcSrc:		parentPage.setFuncSrcTab(node);
	default:				parentPage.setFuncGridPage(node);
	} 
}
FuncManager:main.onInit() {
	pageStart=false;
	tree=null, grid=null, tab=null;
	this.setLeftTree();	
	this.delay( callback() {
		this.tag('splitter').sizes(380);
		@pageStart=true;
	});	
}
FuncManager:main.setLeftTree() {
	&page=pageLoad(this, 'funcTree');
	@tree=page.tree;
	this.left.addPage(page, true);
}
FuncManager:main.setFuncGridPage(node) {
	&page=pageLoad(this, 'funcGrid');
	@grid=page.grid;
	page.initPage(node);
	this.content.addPage(page, true);
}
FuncManager:main.setFuncSrcTab(node) {
	&page=pageLoad(this, 'funcTab');
	page.initPage(node);
	this.content.addPage(page, true);
}
FuncManager:funcGrid.initPage(node) {
	this.func_grp.value(''), this.func_nm.value(''), this.func_type.value('');
	not( node ) return;
	if( node[type].eq('funcGroup') ) {
		this.func_grp.value(node[func_grp]);
	} else if( type.eq('funcSrc') ) {
		p=node.parent();
		this.func_grp.value(p[func_grp]);
		this.func_nm.value(node[func_nm]);
	}
	this.searchList();
}
FuncManager:funcGrid.onInit() { 
	db=instance('pages.model');
	grid=this.grid;
	grid.model(instance('funcList.model'), makeFields(conf('sql.funcGridList'), true) );
	this.func_type.addItem( class('code').getCodeNode('funcSrcType'), 'code,value', '==전체==' );
	this.searchList();
}
FuncManager:funcGrid.initBtn.onClick() {
	this.initForm();
}
FuncManager:funcGrid.grid.onFetchMore() {
	@node[offset]=@node[fetchNum];
	@node.incr('fetchNum',50);
	db.fetchAll(conf('sql.funcGridList'), @node);
	grid.update();
}
FuncManager:funcGrid.grid.onDraw() {
	this.drawGrid(@draw, @node, @over);
}
FuncManager:funcGrid.grid.onClicked() {
	switch( grid.field(@column) ) {
	case check: 
		gridCheck(@me, @node, this[delete]);
	case status: 
		node=@node;
		page = mainPage.getSrcPopup(node);
		rc = grid.nodeRect(node, 6);
		pt = grid.mapGlobal(rc.lb());
		rcOpen = Class.rect(pt, 680,520);
		openPopup(page,null,rcOpen);				 
	default:
	}
}
FuncManager:funcGrid.func_nm.onEnter() {
	this.searchList();
}
FuncManager:funcGrid.func_nm.onFocusIn() {
	@me.select();
}
FuncManager:funcGrid.search.onClick() {
	this.searchList();
}
FuncManager:funcGrid.searchList() {
	root=grid.rootNode().removeAll();
	root.func_grp = this.func_grp.value();
	root.func_nm = this.func_nm.value();
	root.func_type = this.func_type.value();
	root.offset	= 0;
	root.fetchNum	= 50;
	db.fetchAll(conf('sql.funcGridList'),root);
	grid.update();
	total=db.value(conf('sql.funcGridListTotal'), root);
	this.subStatus.value("전체 건수 : $total 건");
	this.delete.hide();
}
FuncManager:funcGrid.drawGrid( d, node, over) {
	rc=class('draw').gridOver(d, over);
	field=grid.field(d.index());
	print("rc=$rc, field=$field");
	switch( field ) {
	case check:		
		if( node[checked] ) 
			d.icon(rc.center(16.16), Icon.func.check);
		else
			d.icon(rc.center(16.16), Icon.func.add);
	case status:
		d.ctrl('btn', rc.center(60,20), "소스보기");
	case type:
		switch( node[type] ) {
		case 'A': 	d.text( rc, "일반함수", "center" );	
		case 'S': 	d.text( rc, "영구함수", "center" );
		default: 	d.text( rc, node[type], "center" );
		}
	case tm:		
		d.text( rc.incrX(2), System.date(node[$field],'yy-MM-dd hh:mm') );	
	default:
		d.text( rc.incrX(2), node[$field] );	
	} 
	not( over ) d.rectLine(rc,4,'#d0d0d0');
}
DevFuncEdit:coreFuncManager.initPage(node) {
	db.fetchAll("select idx, object_cd || ':' || func_nm as value  from core_object_func where func_nm=#{funcNm}", node.removeAll() );
	this.funcCombo.addItem(node,'idx,value');
	this.funcName.value(node[funcNm]);
	this.makeFuncSrcPage(node);
}
DevFuncEdit:coreFuncManager.onInit() {
	db=instance('help.model');
	funcSrcPage=null;
	funcHelpPage=null;
}
DevFuncEdit:coreFuncManager.makeFuncSrcPage(node) {
	not( funcSrcPage ) {
		@funcSrcPage=this.widget({
			layout: <page><editor id="src"></page>
			onInit() {
				setPageClass('dev.EditorSrcImpl', this,'coreFunc');
			}
			initPage(node) {
				this.pageImpl.initPage(node);
			}
		});
	}
	funcSrcPage.initPage(node);
	this.tab.addPage(funcSrcPage, '코어함수 소스', 'vicon.compress_defalut', true); 
}
DevFuncEdit:coreFuncManager.makeFuncHelpPage(node) {
	not( funcSrcPage ) {
		@funcSrcPage=this.widget({
			layout: <page><editor id="src"></page>
			onInit() {
				setPageClass('dev.EditorSrcImpl', this,'coreHelp');
			}
			initPage(node) {
				this.pageImpl.initPage(node);
			}
		});
	}
	funcSrcPage.initPage(node);
	this.tab.addPage(funcSrcPage, '코어함수 도움말', 'vicon.comment_add', true); 
}
DevFuncEdit:pageFuncTab.onInit() {
	tab=this.tab;
	tab.style("QTabWidget::pane { border: none; } QTabBar::tab { padding-left: 5px; padding-top: 5px; padding-right: 10px;}");
	// tab.findLayout().margin(2,0,0,0);
	mainPage.initSrcAction(this);
}
DevFuncEdit:pageFuncTab.status.onDraw() {
}
DevFuncEdit:pageFuncTab.tab.onChange() {
	this.status.redraw();
}
DevFuncEdit:pageSourceLoadPopup.initPage(node) {
	this.pageImpl.initPage(node);
	this.pageClassTemplate.value('');
	this.pageClassTemplate.disable();
}
DevFuncEdit:pageSourceLoadPopup.makePageData(pageGroup, pageCode) {
	fileName=this.pageFileSelectCombo.text();
	not( fileName ) {
		this.alert("페이지 파일정보가 입력되지 않았습니다");
		return null;
	}
	impl=this.pageImpl;
	cur=impl.currentNode;
	cur[pageFileName]=fileName.findPos('.').trim();
	templateNode=cur[templateNode];
	print("makePageData cur===========$cur");
	if( templateNode ) {
		not( this.confirm("템플릿 클래스를 저장하시겠습니까 ?")  ) return null;
		str=fmt( impl.parsePageSrc() );
		cur[classPath]="data/classes/pages/${pageGroup}_${pageCode}Impl.class";
		instance('my.file').writeAll(cur[classPath], str);
	}
	cur[fullPath]="data/pages/$fileName";
	if( instance('my.file').isFile(cur[fullPath]) ) {
		cur.src=instance('my.file').readAll(cur[fullPath]);
	} else if( templateNode[layout] ) {
		cur.src=fmt(templateNode[layout]);
	}
	return cur;
}
DevFuncEdit:pageSourceLoadPopup.onInit() {
	setPageClass('dev.pageClassTemplateImpl',this);
	pageSelect=newClass('dev.pageSelectCombo', this);
	this.pageClassTemplate.disable();
}
DevFuncEdit:pageSourceLoadPopup.cancel.onClick() {
	this.hide();
}
DevFuncEdit:pageSourceLoadPopup.pageFileSelectChange() {
	this.pageClassTemplate.enable();
	this.pageClassTemplate.focus();
}
DevFuncEdit:pageSourceLoadPopup.pageFileSelectCombo.onFocusIn() {
	this.delay(callback() {
		this.pageFileSelectCombo.selectText(true);
	});
}
DevFuncEdit:pageSourceLoadPopup.save.onClick() {
	pageId=pageSelect.getPageId();
	not( pageId ) {
		this.alert("페이지 정보가 입력되지 않았습니다");
		this.pageGroup.focus();
		return;
	}
	pageId.split('.').inject(pageGroup, pageCode);
	cur=this.makePageData(pageGroup, pageCode);
	not( cur ) return; 
	cur.put(pageGroup, pageCode);
	if( cur[src] ) {
		cur[pageSrc]="$cur[pageCode] : { $cur[src] }";
		pageDbCreate(cur, cur[pageGroup]);
	}
	pageSourceLoadOk=class('page').getParentFunction(this,'pageSourceLoadOk');
	if( pageSourceLoadOk ) pageSourceLoadOk(cur);
	this.hide();
}
DevFuncEdit:pageSourceLoadPopup.pageClassTemplateChange() {
}
DevFuncEdit:pageSourceLoadPopup.pageOpen.onClick() {
	cur=this.makePageData();
	not( cur ) return; 
	pageSourceLoadOk=class('page').getParentFunction(this,'pageSourceLoadOk');
	if( pageSourceLoadOk ) pageSourceLoadOk(cur);
	this.hide();
}
DevFuncEdit:main.makeConfManagerTab() {
	/* 공통 설정 처리 */
	not( confManagerPage ) {
		@confManagerPage=pageLoad('DevFuncEdit.confManager');
	}
	this.tab.addPage(confManagerPage, '설정 정보', 'vicon.cog_edit', true);
}
DevFuncEdit:main.editorConfigFunctionClick(funcNm, param, currentNode ) {
	print("@@  editorConfigFunctionClick $funcNm, $param @@");
	currentNode[confParam]=param;
	this.makeConfManagerTab();
	confManagerPage.initPage(currentNode);
}
DevFuncEdit:main.makeCoreFuncTab() {
	/* 내부함수 도움말보기 처리 */
	not( coreFuncPage ) {
		@coreFuncPage=pageLoad('DevFuncEdit.coreFuncManager');
	}
	this.tab.addPage(coreFuncPage, '시스템 함수 정보', 'ficon.balloon-buzz', true);
}
DevFuncEdit:main.editorCoreFunctionClick(funcNm, param, src ) {
	print("@@@@@ $src");
	this.makeCoreFuncTab();
	node=class('util').node('coreFuncNode');
	coreFuncPage.initPage( node.val(funcNm, param, src ) );
}
DevFuncEdit:pageSourceTab.initPage(node) {
	this.pageSourcePath.value(node.fullPath);
	this.classPath.value(node.classPath);
	sourcePage.initPage(node);
	classPage.initPage(node);
	this.tab.current(sourcePage);
}
DevFuncEdit:pageSourceTab.onInit() {
	sourcePage=null, classPage=null;
	this.tab.style("QTabWidget::pane { border: none; } QTabBar::tab { padding-left: 5px; padding-top: 5px; padding-right: 10px;}");
	this.makeSourcePage();
	this.makeClassPage();
}
DevFuncEdit:pageSourceTab.makeSourcePage(node) {
	not( sourcePage ) {
		@sourcePage=this.widget(conf('widget#editor.dev#pageSource'));
	}
	this.tab.addPage(sourcePage,'페이지소스', 'vicon.overlays_defalut', true);
}
DevFuncEdit:pageSourceTab.makeClassPage(node) {
	not( classPage ) {
		@classPage=this.widget(conf('widget#editor.dev#classSource'));
	}
	this.tab.addPage(classPage,'클래스소스', 'vicon.package_defalut');	
}
Common:PreviewFunction.onInit() {
		setPageClass('dev.PreviewFunctionImpl', this);
	}
Common:PreviewFunction.classFuncChange(node) {
		this.pageImpl.classFuncChange(node);
	}
Common:PreviewFunction.setCommFunc(funcNm) {
		this.pageImpl.setCommFunc(funcNm, true);
	}
Common:FuncEditPage.initPage(node) {
		@currentNode=node;
		this.func_type.value( nvl( node[func_type], 'A') );
		this.func_desc.value( node[func_desc] );
		src="";
		if( node[note] ) {
			src="/* $node[note] */\r\n";
		}
		src.append("${node[func_nm]}($node[func_param]) {\r\n$node[src]\r\n}" );
		impl_editor.setSrc( src );
	}
Common:FuncEditPage.onInit() {
		include('dev.EditorSrcImpl');
		db=Class.db('pages');
		currentNode=null;
		impl_editor=newClass('dev.EditorSrcImpl', this, 'commFunc');			
	}
Common:FuncEditPage.save.onClick() {
		not( this.confirm("함수를 저장 하시겠습니까?") ) return;
		node=currentNode;
		type=this.func_type.value();
		node[tm]=System.localtime();
		node[func_desc]=this.func_desc.value();
		node[func_type]=type;
		
		group=comboValue( getParentWidget(this, 'funcGroup') );
		if( node[func_grp] && node[func_grp].ne(group) ) {
			not( this.confirm("함수그룹이 변경되었습니다 계속 진행할까요?\n그룹명 $group (이전그룹명 $node[func_grp])") ) {
				return 0;
			}
			node[func_grp]=null;
		}
		not( node[func_grp] ) {
			not( group ) {
				this.alert("함수 그룹를 선택 하세요?");
				return 0;
			}
			node[func_grp]=group;
		} 
		parse=func(&s) {
			rtn=0;
			c=s.ch();
			if( c.eq('/') ) {
				if( s.ch(1).eq('/') ) {
					node[note]=s.findPos("\n").trim();
				} else if( s.ch(1).eq('*') ) {
					node[note]=s.match().trim();
				}
				c=s.ch();
			}
			fnm=s.move().trim();
			not( fnm.eq(node[func_nm]) ) {
				not( this.confirm("함수명이 다릅니다 새이름으로 저장할까요?\n함수명 $fnm (이전함수명 $node[func_nm])") ) {
					return 0;
				}
				node[func_nm]=fnm;
				rtn=2;
			}
			if( s.ch().eq('(') ) {
				param=s.match();
				if( s.ch().eq('{') ) {
					src=s.match(1);
					if( src.find('/*') || src.find('//') ) {
						node[funcSrc]=makeSrc(src);
					}
					node[src]=src;
					node[func_param]=param;
					rtn=1;
				}
			}
			return rtn;
		};
		src=this.src.value();
		rtn=parse(src.ref());
		print("rtn====$rtn");
		if( rtn.eq(0) ) {
			this.alert("함수저장중 오류가 발생했습니다.\n$node");
			return;
		} else if( rtn.eq(2) ) {
			db.exec( conf('sql#dev.funcInsert'), node)
		} else if( rtn ) {
			not( db.exec( conf('sql#dev.funcUpdate'), node) ) {
				db.exec( conf('sql#dev.funcInsert'), node);
			}
		}
		print("############## $node ###############");
		persist=when( type.eq('S'), true);
		Cf.func(src, persist);
		this.save.disable();	
	}
Common:FuncEditPage.run.onClick() {
		type=this.func_type.value();
		persist=when( type.eq('S'), true); 
		Cf.func(this.src.value(), persist);	
	}
Common:PreviewFunction.btnClose.onClick() {
		this.hide();
	}
DevFuncEdit:confManager.initPage(node) {
	param=node.confParam;
	if( param.find('.') ) {
		param.split('.').inject(type, id);
		if( type.find('#') ) {
			type.split('#').inject(t1, t2);
			this.conf_type.value(t1);
			this.conf_cd.value(t2);
		} else {
			this.conf_type.value(type);
		}
		if( id.find('#') ) {
			id.split('#').inject(confId, kind);
			this.conf_id.value(confId);
			this.conf_kind.value(kind);
		} else {
			this.conf_id.value(id);
		}
	}
	this.fireEvent('search.onClick');
}
DevFuncEdit:confManager.updateConfName() {
	s=this.conf_type.value();
	a=this.conf_cd.value();
	if( s && a ) {
		s.add("#$a");
	} else if( a ) {
		s.add(a);
	}
	b=this.conf_id.value();
	if( b ) {
		s.add(".$b");
		c=this.conf_kind.text();
		if( c ) s.add("#$c");
	}
	this.conf_nm.value(s);
	return s;
}
DevFuncEdit:confManager.onInit() {
	this.conf_type.check('editable',true);
	this.conf_kind.check('editable',true);
	this.conf_type.addItem( class('code').getCodeNode('dev#conf_type'), 'code,value', '=유형=');
	this.conf_kind.addItem( class('code').getCodeNode('dev#conf_kind'), 'code,value', '=종류=');
}
DevFuncEdit:confManager.copy.onClick() {
	val=this.conf_nm.value();
	System.copyText("conf('$val')");
}
DevFuncEdit:confManager.save.onClick() {
	val=this.updateConfName();
	not( val.find('.') ) {
		this.alert("설정코드가 온전히 입력되지 않았습니다");
		return;
	}
	conf(val, this.src.value(), true);
	if( val.eq('cc.dev#conf_type') ) {
		this.conf_type.removeAll().addItem( class('code').getCodeNode('dev#conf_type'), 'code,value', '=유형=');
	}
}
DevFuncEdit:confManager.conf_type.onFocusIn() {
	this.delay( callback() {
		this.conf_type.selectText(true);
	});
}
DevFuncEdit:confManager.conf_kind.onFocusIn() {
	this.delay( callback() {
		this.conf_kind.selectText(true);
	});
}
DevFuncEdit:confManager.conf_type.onChange() {
	val=this.conf_type.value();
	if( val.eq('cc') ) {
		this.conf_cd.disable();
	} else {
		this.conf_cd.enable();
		this.conf_cd.focus();
	}
	this.conf_cd.value('');
	this.conf_id.value('');
	this.conf_kind.value('');
	this.updateConfName();
}
DevFuncEdit:confManager.conf_kind.onChange() {
	val=this.conf_kind.text();
	this.updateConfName();
	not( val ) return;
	/* 영구히 입력정보를 저장한다. */
	node=class('code').getCodeNode('dev#conf_kind');
	cur=node.findOne('code',val);
	not( cur ) {
		node.addNode().val(code:val, value:val);
		s=conf('cc.dev#conf_kind');
		if( s ) s.add(',');
		s.add(val);
		conf('cc.dev#conf_kind',s,true);
	}
	this.fireEvent('search.onClick');
}
DevFuncEdit:confManager.conf_cd.onEnter() {
	this.conf_id.focus();
	this.updateConfName();
}
DevFuncEdit:confManager.conf_id.onEnter() {
	this.conf_kind.focus();
	this.conf_kind.selectText();
	this.updateConfName();
}
DevFuncEdit:confManager.search.onClick() {
	val=this.updateConfName();
	if( val.find('.') ) {
		this.src.value( conf(val) );
	} else {
		db=instance('config.model');
		root=class('util').node();
		db.fetchAll("select grp, cd, data from conf_info where grp='$val'", root );
		s='';
		while( cur, root ) {
			if( s ) s.add("\n");
			s.add("${cur[grp]}.${cur[cd]} = $cur[data]");
		}
		this.src.value(s);
	}
}
Common:ConfManager.initPage(param) {
	param.split('.').inject(a,b);
		print(a,b);
		if( a.find('#') ) {
			a.split('#').inject(a1,a2);
			this.conf_type.value(a1);
			this.conf_cd.value(a2);
		} else {
			this.conf_type.value(a);
			this.conf_cd.value('');
		}
		if( b.find('#') ) {
			b.split('#').inject(a1,a2);
			this.conf_id.value(a1);
			this.conf_kind.value(a2);
		} else {
			this.conf_id.value(b);
			this.conf_kind.value('');
		}
		this.fireEvent('search.onClick');
		this.save.disable();
		
	
}
Common:ConfManager.updateConfName() {
	s=this.conf_type.value();
		a=this.conf_cd.value();
		if( s && a ) {
			s.add("#$a");
		} else if( a ) {
			s.add(a);
		}
		b=this.conf_id.value();
		if( b ) {
			s.add(".$b");
			c=this.conf_kind.value();
			if( c ) s.add("#$c");
		}
		this.conf_nm.value(s);
		return s;
	
}
Common:ConfManager.onInit() {
	x=newClass('dev.ConfManagerEdit', this);
	x1=newClass('common/widget.PageFuncsManager', this);
	dataNode={};
	this.conf_type.check('editable',true);
	this.conf_type.addItem( class('code').getCodeNode('dev#conf_type'), 'code,value', '=유형=');
	
}
Common:ConfManager.cancel.onClick() {
	this.close(true);
	
}
Common:ConfManager.copy.onClick() {
	val=this.conf_nm.value();
		System.copyText("tr('$val')");
	
}
Common:ConfManager.save.onClick() {
	val=this.updateConfName();
		not( val.find('.') ) {
			this.alert("설정코드가 온전히 입력되지 않았습니다");
			return;
		}
		_saveAll=func(&s) {
			confId=null;
			while( s.valid() ) {
				left=s.findPos("##>");
				if( confId ) {
					left.ch();
					conf(confId, left.value(), true);
				}
				confId=s.findPos('=').trim();
			}
		};
		src=this.src.value();
		if( src.find("##>") ) {
			_saveAll(src.ref());
		} else {
			conf(val, src, true);
		}
		if( val.eq('cc.dev#conf_type') ) {
			this.conf_type.removeAll().addItem( class('code').getCodeNode('dev#conf_type'), 'code,value', '=유형=');
		}
		this.save.disable();
	
}
Common:ConfManager.conf_type.onFocusIn() {
	this.delay( callback() {
			this.conf_type.selectText(true);
		});
	
}
Common:ConfManager.conf_type.onChange() {
	val=this.conf_type.value();
		if( val.eq('cc') ) {
			this.conf_cd.disable();
		} else {
			this.conf_cd.enable();
			this.conf_cd.focus();
		}
		this.conf_cd.value('');
		this.conf_id.value('');
		this.conf_kind.value('');
		this.updateConfName();
	
}
Common:ConfManager.conf_kind.onEnter() {
	this.updateConfName();
		this.fireEvent('search.onClick');
	
}
Common:ConfManager.conf_cd.onEnter() {
	this.conf_id.focus();
		this.updateConfName();
		this.fireEvent('search.onClick');
	
}
Common:ConfManager.conf_id.onEnter() {
	this.conf_kind.focus();
		this.conf_kind.selectText();
		this.updateConfName();
	
}
Common:ConfManager.search.onClick() {
	val=this.updateConfName();
		db=Class.db('config');
		grp=this.conf_type.value();
		a=this.conf_cd.value();
		if( a )  grp.add("#$a");

		cd=this.conf_id.value();
		a=this.conf_kind.value();
		if( a ) cd.add("#$a"); 

		dataNode[grp]=grp;
		dataNode[cd]=cd;

 		db.fetchAll("select grp, cd, data from conf_info where grp like #{grp} ||'%' #[cd ? and cd like #{cd}||'%' ]", dataNode.removeAll() );
		s='';
		if( dataNode.childCount() > 1 ) {
			while( cur, dataNode ) {
				if( s ) s.add("\n\n");
				s.add("##> ${cur[grp]}.${cur[cd]} = $cur[data]");
			}
		} else {
			cur=dataNode.child(0);
			if( cur ) {
				s=cur[data];
			}
		}
		this.src.value(s);  
	
}
Common:ConfManager.src.onChange() {
	if( @me.isModify() ) {
			this.save.enable();
		} else {
			this.save.disable();
		}		
	
}
Common:FileUpload.onInit() {
		web=Class.web('upload');
		prog=this.progress;
		prog.range(0,100);
		log=this.src;
	}
Common:FileUpload.upload.onClick() {
		fullPath=this.uploadFile.value();
		not( fullPath ) {
			this.alert("업로드할 파일을 선택하세요?");
			return;
		}
		web.upload('http://localhost:8089/@kiosk.Common.upload', fullPath, callback(type, send, total) {
			switch(type) {
			case progress:  
				not( fileSize ) {
					fileSize=total;
					prog.range(fileSize);
				}
				prog.value(send);
			case finish:	
				log.append("업로드 완료 => 보낸파일:$send[fileName]", true);
 			case error:  	
				log.append("업로드 오류 => $send", true);
			}
		});
		
	}
Common:FileUpload.fileSelect.onClick() {
		fullPath = this.selectFile('업로드할 파일을 선택하세요', 'JEPG files (*.jpg);;PNG files (*.png);;Movie files (*.mp4);;All files (*.*)');
		not( fullPath )
			return;
		prog.value(0);
		this.uploadFile.value(fullPath);
	}
Common:confManager.initPage(param) {
		param.split('.').inject(a,b);
		print(a,b);
		if( a.find('#') ) {
			a.split('#').inject(a1,a2);
			this.conf_type.value(a1);
			this.conf_cd.value(a2);
		} else {
			this.conf_type.value(a);
		}
		if( b.find('#') ) {
			b.split('#').inject(a1,a2);
			this.conf_id.value(a1);
			this.conf_kind.value(a2);
		} else {
			this.conf_id.value(b);
		}
		this.fireEvent('search.onClick');
		this.save.disable();
		
	}
Common:confManager.updateConfName() {
		s=this.conf_type.value();
		a=this.conf_cd.value();
		if( s && a ) {
			s.add("#$a");
		} else if( a ) {
			s.add(a);
		}
		b=this.conf_id.value();
		if( b ) {
			s.add(".$b");
			c=this.conf_kind.value();
			if( c ) s.add("#$c");
		}
		this.conf_nm.value(s);
		return s;
	}
Common:confManager.onInit() {
		x=newClass('dev.ConfManagerEdit', this);
		dataNode={};
		this.conf_type.check('editable',true);
		this.conf_type.addItem( class('code').getCodeNode('dev#conf_type'), 'code,value', '=유형=');
	}
Common:confManager.copy.onClick() {
		val=this.conf_nm.value();
		System.copyText("tr('$val')");
	}
Common:confManager.save.onClick() {
		val=this.updateConfName();
		not( val.find('.') ) {
			this.alert("설정코드가 온전히 입력되지 않았습니다");
			return;
		}
		conf(val, this.src.value(), true);
		if( val.eq('cc.dev#conf_type') ) {
			this.conf_type.removeAll().addItem( class('code').getCodeNode('dev#conf_type'), 'code,value', '=유형=');
		}
		this.save.disable();
	}
Common:confManager.conf_type.onFocusIn() {
		this.delay( callback() {
			this.conf_type.selectText(true);
		});
	}
Common:confManager.conf_type.onChange() {
		val=this.conf_type.value();
		if( val.eq('cc') ) {
			this.conf_cd.disable();
		} else {
			this.conf_cd.enable();
			this.conf_cd.focus();
		}
		this.conf_cd.value('');
		this.conf_id.value('');
		this.conf_kind.value('');
		this.updateConfName();
	}
Common:confManager.conf_kind.onEnter() {
		this.updateConfName();
		this.fireEvent('search.onClick');
	}
Common:confManager.conf_cd.onEnter() {
		this.conf_id.focus();
		this.updateConfName();
		this.fireEvent('search.onClick');
	}
Common:confManager.conf_id.onEnter() {
		this.conf_kind.focus();
		this.conf_kind.selectText();
		this.updateConfName();
	}
Common:confManager.search.onClick() {
		val=this.updateConfName();
		db=Class.db('config');
		grp=this.conf_type.value();
		a=this.conf_cd.value();
		if( a )  grp.add("#$a");

		cd=this.conf_id.value();
		a=this.conf_kind.value();
		if( a ) cd.add("#$a"); 

		dataNode[grp]=grp;
		dataNode[cd]=cd;

 		db.fetchAll("select grp, cd, data from conf_info where grp like #{grp} ||'%' #[cd ? and cd like #{cd}||'%' ]", dataNode.removeAll() );
		s='';
		while( cur, dataNode ) {
			if( s ) s.add("\n\n");
			s.add("${cur[grp]}.${cur[cd]} = $cur[data]");
		}
		this.src.value(s);  
	}
Common:confManager.src.onChange() {
		if( @me.isModify() ) {
			this.save.enable();
		} else {
			this.save.disable();
		}		
	}
Common:KioskUpdate.onInit() {
		prog = this[prog]; 
	}
Common:KioskUpdate.upgrade.onClick() {
		prog.range(0,100);
		while( n, 100 ) {
			System.sleep(30);
			prog.value(n);
		}
		versionServer=conf('version#kiosk.server');
		conf('version#kiosk.main', versionServer, true);
		conf('version#kiosk.main#applyDate', date, true);
		conf('version#kiosk.main#createDate', dateCreate, true);
		pageLoad('KioskMain.Page').open();
		this.close();
	}
Common:KioskUpdate.cancel.onClick() {
		this.close();
	}
Common:Image.onInit() {
		db = Class.db('icons');
		sql = "select (type||'.'||id) as icon, type,id,tm,use from icons where 1=1 #[ type ? and type=#{type}] #[idSearch? and id like '%'||#{idSearch}||'%'] limit  #{offset}, 50";
		grid=this.grid;
		mainPage = this;
		comboNode = {};
		db.fetchAll("select type from icons group by type", comboNode);
		this[imageGroup].addItem(comboNode, 'type', '이미지 선택'); 
		grid.model(db,"check:*#45, icon:아이콘#60, type:그룹#110, id:아이디#180, use:사용여부#60, tm:등록일시#150");
		grid.check('treeMode',  false);
		this.search();
	}
Common:Image.cancel.onClick() {
		this.hide();
	}
Common:Image.imageGroup.onChange() {
		this[idSearch].value('');
		this.search();
	}
Common:Image.newImage.onClick() {
		&form = this.widget({
			title: VRS 이미지 등록,
			icon: Icon.igims.reg,
			layout:
				<page>
					<layout stretch=1>
						<row><label text="이미지 그룹명 : " align=right><input id=typeName></row>
						<row><label text="폴더선택 : " align=right><hbox><input id=folderName stretch=1><button id=folderSel text="..." width=32 height=26></hbox></row>
						<row stretch=1><label colspan=2></row>
						<row><hbox colspan=2><space><button id=apply text=적용><button id=cancel text=취소></hbox></row>
					</layout>
				</page>,
			onInit() {
				this[typeName].focus();
			}
			apply.onClick() {
				mainPage.newImageOk( this[folderName].value(), this[typeName].value() );
				this.close();
			}
			cancel.onClick() {
				this.close();
			}
			folderSel.onClick() {
				this[folderName].value( Cf.selectFolder('이미지 폴더 선택') );
			}
			initPage(main) {
				this[main] = main;
				this.size(460,165);
			}
		});
		form.initPage(this);
		form.open(this,'center');
	}
Common:Image.saveImage(type,id) {
		cf = Cf[imageData];
		tm=System.localtime();
		cf.put(id,type,tm);
		bindBlob= func(type,field) { return when( field.eq('data'), 'blob', 'bind'); };
		not( db.exec("update icons set data=#{data}, tm=#{tm} where id=#{id} and type=#{type}", cf, bindBlob)  ) {
			db.exec("insert into icons (id,type,data,prop,use,tm) values (#{id},#{type},#{data},'clipboard','Y',#{tm})", cf, bindBlob);
		}
	}
Common:Image.newImageOk(folder, type) {
		not( Cf[imageData] ) Cf[imageData]={};
		cf = Cf[imageData];
		tm=System.localtime();
		cf.put(type,tm);
		print();
		bindBlob= func(type,field) { return when( field.eq('data'), 'blob', 'bind'); };
		while( cur, instance("my.filefind").fetchAll(folder, '*.png') ) {
			cf[id] = cur[fileName].find('.').trim();
			print("id=$cf");
			cf[data] = instance('my.file').readAll("$folder/$cur[fileName]");
			not( db.exec("update icons set data=#{data}, tm=#{tm} where id=#{id} and type=#{type}", cf, bindBlob)  ) {
				db.exec("insert into icons (id,type,data,prop,use,tm) values (#{id},#{type},#{data},'save','Y',#{tm})", cf, bindBlob);
			}
		}
	}
Common:Image.ok.onClick() {
		cur=grid.current();
		not( cur ) return;
		fc=getParentFunc(this,'iconSelect');
		fc(cur);
		this.hide();
	}
Common:Image.grid.onFetchMore() {
		@node[offset]=@node[fetchNum];
		db.fetchAll(sql, @node);
		@node.incr('fetchNum',50);
		grid.update();
	}
Common:Image.grid.onClicked() {
		if( @column.eq(2,3) ) { 
			@me.edit(@node,@column); 
		}
	}
Common:Image.grid.onDoubleClick() {
		this.fireEvent('ok.onClick');
	}
Common:Image.grid.onEditEvent(type, node, data, index) {
		&pos = @me.offset(); 
		&hh = @me.headerHeight();
		switch(type) {
		case create:		 
			@me.check('sortEnable',false);
		case finish:
			field=@me.field(index);  
			not( node[$field].eq(data) ) {
				not( node.state(NODE.add) ) {
					node.state(NODE.modify, true);
				}
				node[$field] = data;
			}
			@me.update();
			@me.check('sortEnable',true);
		default: return;
		}
	}
Common:Image.grid.onDraw() {
		this.drawGrid(@me, @draw, @node, @over);
	}
Common:Image.clipboardCapture.onClick() {
		not( Cf[imageData] ) Cf[imageData]={};
		cf = Cf[imageData];
		watcher = instance('global.watcher');
		&page = this.widget({
			id: clipboard,
			title: 복사 이미지저장,
			tag: dialog,
			icon: ICON.igims.cloud , 
			layout:
				<page>
				<layout stretch=1>
					<row><label min=50 stretch=1 text="이미지 그룹"><input id=grp min=90 stretch=3></row>
					<row><label text="이미지 아이디"><input id=code></row>
					<row stretch=1><space rowspan=2></row>
				</layout>
				<hbox><space><button id=saveImage text=확인><button id=cancel text=취소></hbox>
				</page>,
			initPage() {
				this[grp].value('');
				this[code].value('');
			},
			saveImage.onClick() {
				mainPage.saveImage(this[grp].value(), this[code].value());
				this.hide();
			},
			cancel.onClick() {
				this.hide();
			}
		});
		page.initPage();
		if( cf[captureYn] ) {
			cf[captureYn] = false;
			watcher.clipboard(false);
			@me.value('클립보드 캡쳐');
			return;
		}
		cf[captureYn] = true;
		@me.value('클립보드 캡쳐중');
		watcher.clipboard(callback(ty,data) {
			switch(ty) {
			case image:
				cf[data] = data;
				page.initPage();
				page.open();
			default:
				print("clipboard change");
			}
		});		
	}
Common:Image.search.onClick() {
		this.search();
	}
Common:Image.search() {
		root = grid.rootNode();
		root.removeAll(); 
		root[type]			= this[imageGroup].value();
		root[idSearch]	= this[idSearch].value();
		root[offset]			= 0;
		root[fetchNum]	= 50;
		db.fetchAll(sql,root);
		grid.update();
	}
Common:Image.drawGrid(grid, d, node, over) {
		rc=d.rect(); 
		if( d.state(STYLE.Selected) ) {	
			d.fill( rc, '#f0f0f0' );
		} else {
			d.fill();
		}
		if( over ) d.rectLine(rc, 4, '#f0c0a0');
		field=grid.field(d.index());
		switch( field ) {
		case check:		
			if( node[checked] ) 
				d.icon(rc.center(16.16), Icon.func.check);
			else
				d.icon(rc.center(16.16), Icon.func.add);
		case icon:
			d.icon( rc.center(16,16), node[$field]);		
		case use:
			d.text( rc, node[$field], 'center' );
		case tm:
			d.text( rc, System.date(node[$field],'yyyy-MM-dd'), 'center' );
		default:
			d.text( rc.incrX(2), node[$field] );	
		} 
		not( over ) d.rectLine(rc,4,'#d0d0d0');
	}
Common:Image.grid.onDoubleClicked() {
		this.fireEvent('ok.onClick');
	}
Common:searchFunc.initPage(text) {
		this.funcData.value(text);
		this.fireEvent('search.onClick');
	}
Common:searchFunc.isModify() {
		return this.src.isModify();
	}
Common:searchFunc.initPageFunc(root) {
		sql="select cmsCode as groupCode from pageFunc group by cmsCode";
		this.group.addItem( db.fetchAll(sql,root), 'groupCode', '==전체==');
	}
Common:searchFunc.group.onChange() {
		root=_node(this,'GroupCombo');
		val=@me.value();
		cur=root.findOne('groupCode', val);
		print( val, cur);
		cur.removeAll();
		this.code.removeAll();
		switch( this.funcKind.value() ) {
		case a: 	this.codeUserFunc(cur, val);
		case b:	this.codePageFunc(cur, val);
		case c:	this.codeClassFunc(cur, val);
		}	
	}
Common:searchFunc.codePageFunc(cur, val) {
		items=db.fetchAll("select pageCode from pageFunc group by pageCode", cur );
		this.code.addItem( items, 'pageCode', '==전체==');		
	}
Common:searchFunc.onInit() {
		include('common/widget.FuncSearchGrid', true);
		include('common/widget.EditorSrc');
		x=newClass('common/widget.FuncSearchGrid', this);
		editorSrc=newClass('common/widget.EditorSrc', this);
	
		db=Class.db('pages');
		this.funcName.maxWidth(220);
		this.funcData.maxWidth(220);
		this.funcKind.addItem(getCommCodeNode('funcKind'), 'code,value', '=전체=');
	}
Common:searchFunc.initUserFunc(root) { 
		sql="select cmsCode as groupCode from cmsFunc group by cmsCode";
		this.group.addItem( db.fetchAll(sql,root), 'groupCode', '==전체==');
	}
Common:searchFunc.codeUserFunc(cur, val) {
		items=db.fetchAll("select funcName from cmsFunc where cmsCode=#{groupCode}", cur );
		this.code.addItem( items, 'funcName', '==전체==');		
	}
Common:searchFunc.close.onClick() {
		this.close(false);
	}
Common:searchFunc.setSrc(text) {
		editorSrc.setSrc(text);
	}
Common:searchFunc.getSrc(flag) {
		if( flag ) this.save.disable();
		return this.src.value();
	}
Common:searchFunc.save.onClick() {
	
	}
Common:searchFunc.funcKind.onChange() {
		val=this.funcKind.value();
		this.group.removeAll();
		this.code.removeAll();
		root=_node(this,'GroupCombo').removeAll();
		if( val ) {
			this.group.enable();
			this.code.enable();
			switch(val) {
			case a: 	this.initUserFunc(root);
			case b:	this.initPageFunc(root);
			case c:	this.initClassFunc(root);
			}
		} else {
			this.group.disable();
			this.code.disable();
		}
	}
Common:searchFunc.initClassFunc(root) {
		sql="select class_grp as groupCode from class_info group by class_grp";
		this.group.addItem( db.fetchAll(sql,root), 'groupCode', '==전체==');
	}
Common:searchFunc.codeClassFunc(cur, val) {
		items=db.fetchAll("select class_nm from class_info where class_grp=#{groupCode} group by class_nm", cur );
		this.code.addItem( items, 'class_nm', '==전체==');		
	}
Common:searchFunc.search.onClick() {
		kind=this.funcKind.value();
		group=this.group.value();
		code=this.code.value();
		x.search(kind, group, code);
	}
dev:main.cancel.onClick() {
	this.hide();
}
PageEdit:KioskHiTecEditMain.leftTab.onChange() {
		page=@me.current();
		if( page[id].find('DrawClassTree') ) {
			page[tree].update();
		}
		if( page.setContentPage ) {
			page.setContentPage();			
		} else {
			this.contentCurrentPage();
		}		
	}
PageEdit:KioskHiTecEditMain.addClassFuncsEdit(root, append) {
		not( root[currentClass] ) {
			this.alert("클래스 정보가 없습니다. 페이지를 다시 로딩하세요");
		}
		not( classFuncs ) {
			@classFuncs=pageLoad('PageEdit.KioskHiTecClassFuncsEdit');
		}
		classFuncs.initPage(root, append);
		this.leftTab.addPage(classFuncs, '클래스 함수', 'ficon.block', true);
	}
PageEdit:KioskHiTecEditMain.editTypeCombo.onChange() {
		val=@me.value();
		while( page, this.leftTab.widget() ) {
			fc=page[editTypeChange];
			if( fc ) fc(val);
		}
	}
PageEdit:KioskHiTecEditMain.scriptRun.onClick() {
		&page=pageLoad('dev.main',true);
		page.open(this, 'center');
		page.src.insert(tr('template#script.default'), true);
	}
PageEdit:KioskHiTecEditMain.touchUseCheck.onClick() {
		if( @me.checked() ) {
			Cf.touchUse(true);
		} else {
			Cf.touchUse(false);
		}
	}
PageEdit:KioskHiTecEditMain.scriptSearch.onClick() {
		page=pageLoad('Common.searchFunc');
		page.initPage( this.inputSearch.value() );
		page.open();
	}
PageEdit:KioskHiTecEditMain.inputSearch.onEnter() {
		this.fireEvent('scriptSearch.onClick');
	}
PageEdit:KioskHiTecEditMain.inputSearch.onFocusIn() {
		input=this.inputSearch;
		this.delay( callback() {
			input.select();	
		});
	}
PageEdit:KioskHiTecEditMain.onInit() {	Cf.reloadClass('common/control.PageBase');
	include('common.Config'); 
	include('common/control.PageBase', true); 
	include('common/widget.EditorSrc');
	setLogDb('log');
	pageInfo			=pageLoad('PageEdit.KioskHiTecPageInfo');
	drawClassInfo	=null;
	classFuncs		=null;
	pageFuncs		=null;
	debugInfo		=null;
	pageTagInfo	=null;

	Cf.define();
	pageTemplateType=null;
	this.currentProjectNode=Cf[projectNode];

	this.leftTab.addPage(pageInfo, '페이지 정보','ficon.application-blog', true);
	this.content.addPage(pageLoad('PageEdit.KioskHiTecCanvas'), true );
	this.editTypeCombo.addItem( class('code').getCodeNode("EditPage#edit_type"), "code,value");
	this.editTypeCombo.value('A');
	this.selectItemCheck.checked(true);
	this.pageStyle.addItems('window, splash');
	this.delay(callback() {
		this.tag('splitter').sizes(600);
		pageInfo.initPage(this.currentProjectNode);
	});}
PageEdit:KioskHiTecEditMain.getCurrentPage() {
		return this.content.current();
	}
PageEdit:KioskHiTecEditMain.closePage() {
		while( page, this.content.widget() ) {
			page.closePage();
		}
	}
PageEdit:KioskHiTecEditMain.onKeyDown() {
		not( @mode&KEY.ctrl ) return false;
		switch(@key) {
		case KEY.D: 	this.fireEvent('debugPage.onClick');
		case KEY.F:		
			if( pageTemplateType.eq('draw') ) {
				this.leftTab.current(drawClassInfo);
				pageTagInfo.drawClassInfo.update();
			} else {
				this.leftTab.current(pageTagInfo);
				pageTagInfo.tree.update();
			}
		case KEY.Q:		this.leftTab.current(drawClassInfo); 
		case KEY.W:	this.leftTab.current(pageInfo); 
		}
		return false;
	}
PageEdit:KioskHiTecEditMain.contentCurrentPage(page) {
		not( page ) {
			node= this.currentPageNode;
			page=node.page;
		}
		if( page ) {
			this.content.addPage(page,true);
		}
	}
PageEdit:KioskHiTecEditMain.addPageFuncsEdit(root, append) {
		not( root[currentPage] ) {
			this.alert("페이지 정보가 없습니다. 페이지를 다시 로딩하세요");
		}
		not( pageFuncs ) {
			@pageFuncs=pageLoad('PageEdit.KioskHiTecPageFuncsEdit');
		}
		pageFuncs.initPage(root, append);
		this.leftTab.addPage(pageFuncs, '페이지 함수', 'vicon.page_code', true);
	}
PageEdit:KioskHiTecEditMain.projectAddPorc(type) {
		tab=this.leftTab;
		tab.remove(drawClassInfo, true);
		tab.remove(classFuncs, true);
		tab.remove(pageFuncs, true);
		tab.remove(debugInfo, true);
		tab.remove(pageTagInfo, true);
		if( type.eq('project') ) {
			tab.remove(pageInfo, true);
		}
	}
PageEdit:KioskHiTecEditMain.pageEditOk(node) {
 		node.inject( page_group, page_code );
		pageInfo.initPage( this.currentProjectNode, page_group, page_code );
		if( node[sourceModify] ) {
			this.pageSelect(node, true);
		}
	}
PageEdit:KioskHiTecEditMain.confManager.onClick() {
		pageLoad('Common.ConfManager').open();
	}
PageEdit:KioskHiTecEditMain.debugPage.onClick() {
		not( debugInfo ) {
			@debugInfo=pageLoad('PageEdit.KioskHiTecDebugPage');
		}
		this.leftTab.addPage(debugInfo, '디버그 정보','vicon.bug_edit', true);
	}
PageEdit:KioskHiTecEditMain.pageStyle.onChange() {
		val=@me.value();
		page=getMainPage(this);
		rc=page.rect();
		if( val.eq('window') ) {
			page.flags('top',true);
			page.flags('window');
		} else if( val.eq('splash') ) {
			page.flags('splash,top');
			rc.incrH(80);
			page.size(rc);
		}
		page.show();
	}
PageEdit:KioskHiTecEditMain.pageCreateOk(node) {
 		node.inject( page_group, page_code, pageSource, classSource, page_template );
		page=pageLoad("${page_group}.${page_code}");
		if( page ) {
			this.alert("${page_group}.${page_code} 이미 등록된 페이지 입니다. 이전 페이지 정보를 로딩합니다.");
		} else {
			pageReload('pages', null, page_group, page_code, pageSource, true);
			if( classSource ) {
				not( classReload(classSource.ref(),  "${page_group}/widget", node ) ) {
					this.alert("페이지 클래스 생성중 오류가 발생했습니다.\n오류내용: $node[error]");
					return;
				}
			}
		}
		this.currentPageNode=null;
		pageInfo.initPage( this.currentProjectNode, page_group, page_code );
		this.pageSelect(node);
	}
PageEdit:KioskHiTecEditMain.pageSelect(node, reload) {
	not( node ) {
		this.alert("페이지가 선택되지 않았습니다. 선택된 페이지를 확인하세요");
		return;
	}
	node.inject(page_group, page_code, page_template);
	if( reload ) {
		page=node[page];
		not( page ) page=get("${page_group}.${page_code}");
		if( page ) {
			this.content.remove(page);
			pageObjectDelete(page);
		}
	}
	this.currentPageNode=node;

	page=pageLoad("${page_group}.${page_code}", reload);
	this.alert("페이지 로드 : ${page_group}.${page_code}");
	not( page ) {
		this.alert("페이지 로딩중 오류가 발생했습니다.\n페이지 정보: ${page_group}.${page_code}");
		return;
	}
	tab=this.leftTab;
	tab.remove(classFuncs, true);
	tab.remove(pageFuncs, true);
	tab.remove(debugInfo, true);
	node[page]=page;

	if( page_template.eq('draw') ) {
		not( drawClassInfo ) @drawClassInfo=pageLoad('PageEdit.KioskHiTecDrawClassTree');
		not( pageTemplateType.eq('draw') ) {
			tab.remove(pageTagInfo, true); 
		}
		not( pageTagInfo ) @pageTagInfo=pageLoad('PageEdit.KioskHiTecPageTagTree');
		not( pageTemplateType.eq('page') ) {
			tab.remove(drawClassInfo, true); 
		}
		tab.addPage(pageTagInfo, '페이지 태그','ficon.application-block');
		pageTagInfo.initPage(node, page);

		tab.addPage(drawClassInfo, 'Draw 태그','ficon.application-block', true);
		drawClass=getClassInfo(null, page);
		drawClass.loadMainPage( node[page_info] ); 
		drawClassInfo.changeXml(drawClass);
		pageLoad('PageEdit.@{ProjectId}Attr').removeGrid();
		@pageTemplateType='draw';

	} else {
		not( pageTagInfo ) @pageTagInfo=pageLoad('PageEdit.KioskHiTecPageTagTree');
		not( pageTemplateType.eq('page') ) {
			tab.remove(drawClassInfo, true); 
		}
		tab.addPage(pageTagInfo, '페이지 태그','ficon.application-block', true);
		pageTagInfo.initPage(node, page);
		@pageTemplateType='page';
	}
	this.contentCurrentPage(page);
}
PageEdit:KioskHiTecEditMain.selectItemCheck.onClick() {
		node=this.currentPageNode;
		not( node ) return;
		page=node[page];
		x=page[x];
		x.inject(cf);
		if( @me.checked() ) {
			 cf[selectedItem]=true;
		} else {
			 cf[selectedItem]=false;
		}
	}
PageEdit:KioskHiTecEditMain.goProjectGrid.onClick() {
	fc=getParentFunc(this,'addMainPage');
	if( fc ) fc();
	
	code=Cf[projectCode];
	dataPath="project/$code/data";
	fc=getParentFunc(this,'addMainPage');
	if( fc ) fc();
	file=Class.file();
	file.copy("$dataPath/pages.db", "$dataPath/watcher_pages.db", true );
	file.copy("$dataPath/config.db", "$dataPath/watcher_config.db", true );
	print("xxxxxxxxx $dataPath/pages.db xxxxxxxxxxx");	
}
PageEdit:KioskHiTecPageInfo.initPage(node, group, code) {
		if( node ) {
			x.initGrid(node);
			this.openPage.disable();
		}
		this.setWorkPage(group,code);
		this.grid.update();
	}
PageEdit:KioskHiTecPageInfo.openPage.onClick() {
		node=this.grid.current();
		if( node ) {
			this.pageSelect(node);
		}
	}
PageEdit:KioskHiTecPageInfo.deletePage.onClick() {
		not( this.confirm("선택된 페이지를 삭제하시겠습니까?") ) return;
		configDb=Class.db('config');
		db=Class.db('pages');
		while( cur, this.grid.rootNode() ) {
			not( cur[checked] ) continue;
			group=cur[page_group].upper(1);
			cur[classWidget]="$group/widget";
			cur[classControl]="$group/$cur[page_code]/control";
			db.exec("delete from pageLayout where cmsCode=#{page_group} and pageCode=#{page_code}", cur);
			db.exec("delete from pageObject where cmsCode=#{page_group} and pageCode=#{page_code}", cur);
			db.exec("delete from pageFunc where cmsCode=#{page_group} and pageCode=#{page_code}", cur);
			db.exec("delete from class_mst where class_grp=#{classWidget} and class_nm=#{page_code}", cur);
			db.exec("delete from class_mst where class_grp=#{classControl}", cur);
			db.exec("delete from class_info where class_grp=#{classWidget} and class_nm=#{page_code}", cur);
			db.exec("delete from class_info where class_grp=#{classControl}", cur);
			configDb.exec("delete from page_info where page_group=#{page_group} and page_code=#{page_code}", cur);
		}
		node=getParentObject(this,'currentProjectNode');
		x.initGrid(node);
	}
PageEdit:KioskHiTecPageInfo.onInit() {
		form=pageLoad("PageEdit.KioskHiTecPageForm");
		include("common/widget.PageInfoGrid");
		x=newClass("common/widget.PageInfoGrid", this);
		pageBlock=false;
	}
PageEdit:KioskHiTecPageInfo.addPage.onClick() {
		fc=getParentFunc(this,'projectAddPorc');
		fc('page');	
		form.initPage();
		this.openPage.disable();
	}
PageEdit:KioskHiTecPageInfo.pageChange(node) {
		if( pageBlock ) return;
		form.initPage(node);
		this.openPage.enable();
	}
PageEdit:KioskHiTecPageInfo.pageSelect(node) {
		fc=getParentFunc(this,'pageSelect');
		fc(node); 
	}
PageEdit:KioskHiTecPageInfo.setContentPage() {
		fc=getParentFunc(this,'contentCurrentPage');
		fc(form);	
	}
PageEdit:KioskHiTecPageInfo.setWorkPage(group, code ) {
		if( group, code ) {
			@pageBlock=true;
			find=findQuery( this.grid.rootNode(), "page_group=$group, page_code=$code");
			if( find ) {
				this.grid.current(find);
			}
			this.delay(callback() {
				@pageBlock=false;
			});
			return;
		}
		form.initPage();
		this.setContentPage();
	}
PageEdit:KioskHiTecPageForm.templateSaveClick( type ) {
		&page=pageLoad('Common.ConfManager', true);
		switch( type ) {
		case layout:		page.initPage('template#layout');
		case class:		page.initPage('template#class');
		case xml:			page.initPage('template#xml');
		}
		page.open(this,'center');
	}
PageEdit:KioskHiTecPageForm.initPage(node) {
		if( node ) {
			dataNode.pageMode='edit';
			db.fetch("select page_title, page_icon, page_info, page_kind, page_template, note from page_info where page_group=#{page_group} and page_code=#{page_code}", node);
			this.pageCreate.value("페이지 정보수정");
			node.inject(page_group, page_code);
			node[pageSource]=getPageString(node);
			node[classSource]=getClassInfo( Cf.info('funcVar', page, 'init') );

			dataNode[page_icon]=node[page_icon];
			layoutPage.setValue(node[pageSource]);
			classPage.setValue(node[classSource]);
			xmlPage.setValue(node[page_info]);		
			this.page_icon.icon(node[page_icon]);
			setFormValue(this,node);
			this.page_template.disable();
		} else {
			dataNode.pageMode='create';
			setFormValue(this);
			dataNode[page_icon]=null;
			this.pageCreate.value("페이지 생성");
			this.page_group.value(Cf[projectCode]);
			this.page_code.focus();
			layoutPage.setValue('');
			classPage.setValue('');
			xmlPage.setValue('');
			tab.remove(xmlPage, true);
			this.page_icon.icon("vicon.add_defalut");
			this.page_template.enable();
		}
	}
PageEdit:KioskHiTecPageForm.onInit() {
		db=Class.db('config');
		projectCode=Cf[projectCode];
		dataNode={};
		layoutPage	=pageLoad("PageEdit.KioskHiTecPageSrc",true);
		classPage	=pageLoad("PageEdit.KioskHiTecPageSrc",true);
		xmlPage		=pageLoad("PageEdit.KioskHiTecPageSrc",true);
		tab=this.pageSourceTab;
		this.page_group.maxWidth(220);
		this.page_code.maxWidth(220);
		this.page_kind.addItem( getCommCodeNode('pageKind'), "code,value", "=선택=");
		this.page_template.addItem( getCommCodeNode('pageTemplate'), "code,value", "=선택=");
		tab.addPage(layoutPage, "페이지 소스", "ficon.script-code");
		tab.addPage(classPage, "페이지 클래스", "ficon.script-block");
		tab.addPage(xmlPage, "XML 소스", "ficon.script-import");
		tab.current(layoutPage);
		layoutPage.setType('layout');
		classPage.setType('class');
		xmlPage.setType('xml');
	}
PageEdit:KioskHiTecPageForm.setDefaultPageSource(val) {
		not( dataNode.pageMode.eq('create') )  return;
		not( val ) {
			val=this.page_template.value();
			not( val ) return;
		}
		projectCode=this.page_group.value();
		pageCode=this.page_code.value();
		not( pageCode ) {
			this.alert("페이지 그룹, 페이지 코드를 입력하세요");
			this.page_group.focus();
			@me.value('');
			return;
		}
		page=pageLoad("${projectCode}.${pageCode}");
		if( page ) {
			if( this.confirm("${projectCode}.${pageCode} 는 이미 등록된 페이지입니다. 기존 정보를 로딩할까요?") ) {
				dataNode[page_group]=projectCode;
				dataNode[page_code]=pageCode;
				classInfo=getClassInfo( Cf.info('funcVar', page, 'init') );
				layoutPage.setValue( getPageString(dataNode) );
				classPage.setValue( classInfo );
				this.page_template.value('');
				return;
			}
		}

		layoutText=fmt( tr("template#layout.$val") );
		classText=fmt( tr("template#class.$val") );
		if( val.eq('canvas') ) {
			xmlPage.setValue( tr("template#xml.canvas") ) ;
		}
		layoutPage.setValue( layoutText );
		classPage.setValue( classText );	
	}
PageEdit:KioskHiTecPageForm.applyTemplate.onClick() {
		page=pageLoad('Common.ConfManager', true);
		page.initPage('cc.pageTemplate');
		page.modal(this, 'center');
		combo=this.page_template;
		combo.removeAll().addItem( getCommCodeNode('pageTemplate', true), "code,value", "=선택=");
		combo=this.page_kind;
		combo.removeAll().addItem( getCommCodeNode('pageKind', true), "code,value", "=선택=");
	}
PageEdit:KioskHiTecPageForm.page_kind.onChange() {
		val=@me.value();
		not( val ) return;
		if( val.eq('C') ) {
			this.page_template.value('draw');
			this.page_template.disable();
			this.setDefaultPageSource('canvas');
			tab.addPage(xmlPage, "XML 소스", "ficon.script-import", true);
		} else {
			this.page_template.value('');
			this.page_template.enable();
			tab.remove(xmlPage, true);
		}
	}
PageEdit:KioskHiTecPageForm.page_template.onChange() {
		this.setDefaultPageSource();
	}
PageEdit:KioskHiTecPageForm.iconSelect(node) {
		node.inject(id, type);
		dataNode[page_icon]="${type}.${id}";
		this.page_icon.icon("${type}.${id}");
	}
PageEdit:KioskHiTecPageForm.page_icon.onClick() {
		pageLoad('Common.Image').open(this,'center');
	}
PageEdit:KioskHiTecPageForm.pageCreate.onClick() {
		 src=layoutPage.src.value();
		 not( src ) {
			this.setDefaultPageSource();
		}
		getFormValue(this, dataNode);
		not( formValid(this, dataNode, 'page_title') ) {
			return;
		}		
		dataNode[page_info]		=when( dataNode[page_template].eq('draw'), xmlPage.getSrc() ); 
		switch( dataNode.pageMode ) {
		case create:
			dataNode[project_idx]		=getParentObject( this, 'currentProjectNode').get('project_idx');
			if( db.count("select count(1) from page_info where page_group=#{page_group} and page_code=#{page_code}",dataNode) ) {
				dataNode.inject(page_group, page_code);
				this.alert("${page_group}.${page_code} 는 이미등록된 페이지 입니다.");
				return;
			}
			db.exec( getQuery('page_info', 'page_group, page_code, page_title, page_icon, page_kind, page_template, page_info, project_idx, note'), dataNode );
			if( db.error() ) {
				this.alert("페이지 생성중 오류가 발생했습니다\n에러: $db.error() ");
				return;
			}
			dataNode[pageSource]	=layoutPage.getSrc();
			dataNode[classSource]	=classPage.getSrc();  
			fc=getParentFunc(this,'pageCreateOk');
			fc(dataNode);
		case edit:
			dataNode.inject(page_group, page_code);
			dataNode[sourceModify]=false;
			if( layoutPage.isModify() ) {
				if( this.confirm("페이지 레이아웃 소스가 변경되었습니다. 페이지를 다시로딩할까요?") ) {
					pageLayout=layoutPage.getSrc(true);
					pageReload('pages', null, page_group, page_code, pageLayout, true);
					dataNode[sourceModify]=true;
				}
			}
			if( classPage.isModify() ) {
				if( this.confirm("페이지 클래스 소스가 변경되었습니다. 클래스를 다시로딩할까요?") ) {
					pageClass=classPage.getSrc(true);
					not( classRealod(pageClass.ref(),  "${page_group}.widget", dataNode, this) ) return;
					dataNode[sourceModify]=true;
				}
			} 
			if( xmlPage.isModify() ) {
				if( this.confirm("페이지 XML 소스가 변경되었습니다. XML를 다시로딩할까요?") ) {
					dataNode[page_info]=xmlPage.getSrc(true); 
					dataNode[sourceModify]=true;
 				}
			} 
			db.exec( getQuery('page_info', 'page_group, page_code, page_title, page_icon, page_kind, page_template, page_info, note', 'page_group, page_code'), dataNode );
			if( db.error() ) {
				this.alert("페이지 정보 수정중 오류가 발생했습니다\n에러: $db.error() ");
				return;
			}
			fc=getParentFunc(this,'pageEditOk');
			fc(dataNode);
			this.alert("페이지 정보가 수정되었습니다");
		default:
			this.alert("페이지 모드가 정의되지 않았습니다.");
		}
	}
PageEdit:KioskHiTecPageSrc.isModify() {
		return this.save.is('enable');
	}
PageEdit:KioskHiTecPageSrc.onInit() {
		x=newClass('common/widget.EditorSrc', this);
	}
PageEdit:KioskHiTecPageSrc.setType(type) {
		this.editType=type;
	}
PageEdit:KioskHiTecPageSrc.setValue(text) {
		x.setSrc(text);
	}
PageEdit:KioskHiTecPageSrc.save.onClick() {
		fc=getParentFunc(this,'templateSaveClick');
		if( fc ) fc(this.editType);
	}
PageEdit:KioskHiTecPageSrc.getSrc(flag) {
		if( flag ) this.save.disable();
		return this.src.value();
	}
PageEdit:KioskHiTecDrawClassTree.editTypeChange(type) {
		this.editType=type;
		this.changeTag();
	}
PageEdit:KioskHiTecDrawClassTree.setClassFuncCombo(node) {
		if( node[tag].eq('Window', 'Page') ) {
			this.createClass.value('클래스 수정');
			impl_ClassCombo.makeComboData( DrawClass );
			return;
		}

		control=null;
		if( node[tag].eq('page') ) {
			control=node[x];
		} else if( node[@control] ) {
			control=node[@control];
		} else if( node[@classBase] ) {
			control=node;
		}
		if( control ) {
			this.createClass.value('클래스 수정');
		} else {
			this.createClass.value('클래스 추가');
		}
		impl_ClassCombo.makeComboData( control );
	}
PageEdit:KioskHiTecDrawClassTree.createClassOk(type, editNode) {
	tag=editNode.tag;
	pageCode=getParentObject(this, 'currentPageNode').get('page_code');
	not( pageCode ) {
		this.alert("페이지 코드가 존재하지 않습니다. 생성 클래스를 확인하세요");
		return;
	}
	path=editNode[ClassPath];
	if( path ) {
		classId="$path/control.$tag";
	} else {
		classId="KioskHiTec/$pageCode/control.$tag";
	}
	err=saveClassNode(classId, editNode[src].ref() );
	if( err ) {
		this.alert("페이지 저장중 오류가 발생했습니다\n에러내용: $err");
		return;
	}
	p=editNode.parent();
	not( p ) return;
	pctrl=p[@control];
	not( pctrl ) {
		pctrl=DrawClass;
	}
	editNode[@control]=newClass(classId, editNode, pctrl);
	pctrl.conf();
	this.tree.update();	}
PageEdit:KioskHiTecDrawClassTree.createClass.onClick() {
		page=pageLoad('PageEdit.KioskHiTecCreateClass');
		page.initPage(tagNode, this.createClassOk);
		mainPage=getMainPage(this);
		page.size(850,560);
		page.open(mainPage,'center');
	}
PageEdit:KioskHiTecDrawClassTree.onInit() {
		DrawClass=null;
		tagNode=null;
		classFuncGridPopup=null;
		classFuncGridPage=null;
		tree=this.tree;
		include('common/widget.EditPageTree');
		include('common/widget.ClassComboSelect');
		impl_TagTree=newClass('common/widget.EditPageTree', this);
		impl_ClassCombo=newClass('common/widget.ClassComboSelect', this); 
		
		this.sourcePages.addPage(pageLoad('PageEdit.KioskHiTecSrcTab'), true); 
		this.delay( callback() {
			this.sourcePages.hide();
		});
		this.editType='A';
	}
PageEdit:KioskHiTecDrawClassTree.reloadNode.onClick() {
	}
PageEdit:KioskHiTecDrawClassTree.editNode.onClick() {
		node=tagNode;
		if( node.kind ) {
			s=tr('template#kiosk.editNodeKind', node[tag], node[kind]);
		} else {
			if( node.id ) {
				id="#$node[id]";
			} else {
				p=node.parent();
				if( p.id ) {
					id="#$p[id]#$node[tag]";
				} else {
					id=node[tag];
				}
			}
			s=tr('template#kiosk.editNode', id);
		}
		p=pageLoad('dev.main');
		p.src.append(s);
		p.open();
	}
PageEdit:KioskHiTecDrawClassTree.classFuncGridOpen() {
		popup=classFuncGridPopup;
		not( popup ) {
			popup=this.widget(conf('page#dev.ClassFuncGrid'));
			popup.size(850,560);
			@classFuncGridPopup=popup;
		}
		mainPage=getMainPage(this);
		popup.initPage();
		popup.open(mainPage,'center');
		return popup;
	}
PageEdit:KioskHiTecDrawClassTree.classFuncGrid.onClick() {
		this.classFuncGridOpen();
	}
PageEdit:KioskHiTecDrawClassTree.changeXml(ctrl) {
		ctrl.inject(cf, xmlNode);
		Class.model('EditPageTree').rootNode(xmlNode);
		tree.selectClear();
		tree.update();
		root=xmlNode.child(0);
		if( root ) {
			tree.expand(root, true );
		}
		@DrawClass=ctrl;
	}
PageEdit:KioskHiTecDrawClassTree.changeTag(node) {
		if( node[tag] ) {
			DrawClass.inject(cf); 
			this.setClassFuncCombo(node);
			cf.selectedItem=node;
			DrawClass.update();
			this.classFuncGrid.enable();
		}
		if( this.editType.eq('A') ) {
			not( classFuncGridPage ) {
				@classFuncGridPage=this.widget(conf('page#dev.ClassFuncGrid')); 
			}
			classFuncGridPage.initPage(impl_ClassCombo,'KioskHiTec');
			this.sourcePages.addPage(classFuncGridPage, true);
			this.classFuncGrid.disable();
			this.sourcePagesShow();
		} else if( this.editType.eq('B') ) {
			not( node ) node=this.tree.current();
			page=pageLoad('PageEdit.KioskHiTecAttr');
			page.initPage(node);
			this.sourcePages.addPage(page, true);
			this.sourcePagesShow();
		} else {
			this.sourcePages.hide();
		}
		@tagNode=node;
	}
PageEdit:KioskHiTecDrawClassTree.initClassCombo() {
		if( this.editType.eq('A') ) {
			classFuncGridPage.initPage();
		}
	}
PageEdit:KioskHiTecDrawClassTree.sourcePagesShow() {
		this.sourcePages.show();
		arr=this.tag('splitter').sizes();
		if( arr[1].eq(0) ) {
			sz=arr[0];
			sz-=450;
			if( sz<0 ) {
				sz=arr[0];
				arr[1]=min(250,sz);
				arr[0]=sz-arr[1];
			} else {
				arr[0]=sz;
				arr[1]=450;
			}
			this.tag('splitter').sizes(arr);
		}
	}
PageEdit:KioskHiTecDrawClassTree.classFuncComboChange(node) {
		not( node[class_grp] ) {
			return false;
		}
		page=pageLoad('PageEdit.KioskHiTecSrcTab');
		this.sourcePages.addPage(page, true); 
		tab=page[tab];
		luid="${node[class_grp]}.${node[class_nm]}.${node[class_func]}";
		while( page, tab.widget() ) {
			if( page.luid.eq(luid) ) {
				tab.current(page);
				return;
			}
		}
 		page=pageLoad('PageEdit.KioskHiTecSrcEdit',true);
		page.luid=luid;
		page.addTabPage(tab, node);
		this.sourcePagesShow();
	}
PageEdit:KioskHiTecAttr.initPage(node) {
		@tagNode=node;
		impl_grid.makeTagAttribute(node);
	}
PageEdit:KioskHiTecAttr.onInit() {
		tagNode=null;
		include('common/widget.TagAttributeGrid');
		impl_grid=newClass('common/widget.TagAttributeGrid', this);
	}
PageEdit:KioskHiTecAttr.removeGrid(node) {
		impl_grid.initGridData(node);
	}
PageEdit:KioskHiTecAttr.initGridData.onClick() {
		this.removeGrid(tagNode);
	}
PageEdit:KioskHiTecCreateClass.initPage(node, callback) {
		if( node[@control] ) {
			if( typeof( node[@control], 'class') ) {
				node[src]=getClassSrc(node[@control]);
			}
		}
		x.initPage(node);
		tag=node[tag];
		this.callbackSave=callback;
 		this.className.value(tag);
		this.src.focus();
		this.controlClass.value(tag);
		@currentNode=node;
	}
PageEdit:KioskHiTecCreateClass.setClassEditor(val, type) {
		not( val ) return;
		cur=classNames.findOne('class_nm', val);
		tag=currentNode[tag];
		not( cur ) {
			this.alert("$val 클래스 함수를 찾을수 없습니다.");
		}
		db.fetchAll("select class_func, class_param, 	
				case when length(class_src)=0 then class_data else class_src end as class_data, 
				note, type 
			from class_info where class_grp='common/control' and class_nm=#{class_nm} order by type", cur.removeAll() );
		s='';
		check=this.newPageCheck.checked();
		while( sub, cur, n, 0 ) {
			if( n ) s.add("\r\n");
			sub.inject(class_func, class_param, class_data, note, type);
			if( note ) {
				funcDesc=note.trim();
				s.add("/* $funcDesc */\r\n");	
			}
			if( type.eq('A'), not(check) ) {
				s.add("${tag}( $class_param ) {$class_data }\r\n");
			} else {
				s.add("${class_func}( $class_param ) {$class_data }\r\n");
			}
		}
		if( check ) {
			p=pageLoad('dev.main').open();
			p.src.append("\r\n/* $val 클래스 정보 */\r\n\r\n$s");
			p.open();
		} else {
			x.setSrc(s);
		}
	}
PageEdit:KioskHiTecCreateClass.templateEdit.onClick() {
		page=pageLoad('Common.ConfManager');
		page.initPage('template#class.control');
		page.open(this,'center');
	}
PageEdit:KioskHiTecCreateClass.onInit() {
		db=Class.db('pages');
		include('common/widget.CreateClassEditor');
		x=newClass('common/widget.CreateClassEditor', this);
		classNames={};
		db.fetchAll("select class_nm from class_mst where class_grp='common/control' order by class_grp", classNames);
		this.controlClass.addItem( classNames, 'class_nm', '==선택==');
		currentNode=null;
	}
PageEdit:KioskHiTecCreateClass.close.onClick() {
		this.hide();
	}
PageEdit:KioskHiTecCreateClass.controlClass.onChange() {
		val=@me.value();
		this.setClassEditor(val);
	}
PageEdit:KioskHiTecCreateClass.save.onClick() {
		currentNode.src=this.src.value(); 
		if( typeof(this.callbackSave,'function') ) {
			this.callbackSave('save',currentNode);
			this.alert("$currentNode[tag] 클래스를 저장했습니다.");
		}
	}
PageEdit:KioskHiTecCreateClass.commonClassCreate.onClick() {
		db.exec("delete from class_mst where class_grp='common/control' and class_nm<>'PageBase' ");
		db.exec("delete from class_info where class_grp='common/control' and class_nm<>'PageBase' ");
		ff=Class.filefind();
		path='data/classes/common/control';
		page=this;
		_save=func(&s) {
			while( s.valid() ) {
				c=s.ch();
				not( c ) break;
				if( c.eq('/') ) {
					if( s.ch(1).eq('/') ) note.add( s.findPos("\n") );
					else if( s.ch(1).eq('*') ) note.add( s.match() );
					continue;
				}
				break;
			}
			className=s.move().trim();
			c=s.ch();
			print("className=$className, err=$c");
			if( c.eq('{') ) {
				src=s.match(1);
				saveClass('common/control', className, src, page);
			}
		};
		
		while( c, ff.fetchAll(path) ) {
			not( c[type].eq('file') ) continue;
			if( c[fileName].eq('PageBase.class') ) continue;
			src=fileRead("$path/$c[fileName]");
			_save( src.ref() );
		}
		db.fetchAll("select class_nm from class_mst where class_grp='common/control' order by class_grp", classNames.removeAll() );
		this.controlClass.removeAll();
		this.controlClass.addItem( classNames, 'class_nm', '==선택==');
	}
PageEdit:KioskHiTecSrcTab.initTabAction() {
		page=this;
		page.action([
			{id: 'tab.close',				text: 탭닫기,			icon:ICON.vicon.cancel_defalut },
			{id: 'tab.closeOther',		text: 다른 탭닫기,	icon:ICON.vicon.application_form_delete },
			{id: 'tab.deleteFunc',		text: 함수삭제,		icon:ICON.vicon.brick_delete },
		]); 
		page.action('tab.close').trigger(callback() {
			tab.remove(tab[buttonClickPage] );
			not( tab.widget().size() ) getParentObject('sourcePages').hide();
		}); 
		page.action('tab.closeOther').trigger(callback() {
			clickPage=tab[buttonClickPage];
			while( sub, tab.widget() ) {
				if( sub==clickPage ) continue;
				tab.remove(sub);
			}
		}); 
	}
PageEdit:KioskHiTecSrcTab.onInit() {
		tab=this.tab;
		tab.style("QTabWidget::pane { border: none; } QTabBar::tab { padding-left: 5px; padding-top: 5px; padding-right: 10px;}");
		this.initTabAction();
	}
PageEdit:KioskHiTecSrcEdit.onInit() {
		include('common/widget.ClassFuncEditor');
		x=newClass('common/widget.ClassFuncEditor', this);
	}
PageEdit:KioskHiTecSrcEdit.addTabPage(tab, node) {
		x.initPage(node);
		page=this;
		tabBtn=page.widget({tag:toolbutton, 
			onClick() {
				tab[buttonClickPage]=page; 
				page.menu("tab.close, tab.closeOther,-, tab.save, tab.deleteFunc", 12);
			}
			initButton(node) {
				if( node[class_nm].eq(node[class_func]) ) {
					icon="ficon.document-code";
				} else if( node[class_func].eq('initControl','conf','draw','mouseDown','mouseUp') ) {
					icon="ficon.application-plus-black";
				} else {
					icon="ficon.document-globe";
				} 
				this.icon(icon);
				this.currentNode=node;
			}
		});
		node.page=page;
		tabBtn.initButton(node);
		page.initPage(node);
		if( node[class_nm].eq(node[class_func]) ) {
			tab.addPage(page, "node[class_func]" , null, true);
		} else {
			tab.addPage(page, "$node[class_nm]/$node[class_func]" , null, true);
		}
		
		tab.tabButton(page, tabBtn, 'left');
	}
PageEdit:KioskHiTecClassFuncsEdit.initPage(root, append) {
		x.inject(cf);
		x.initPage(root, append);
		this.classInfo.value( root[inherit] );
	}
PageEdit:KioskHiTecClassFuncsEdit.onInit() {
		include('common/widget.ClassFuncsManager');
		x=newClass('common/widget.ClassFuncsManager', this);
		this.autoRunCheck.checked(true);
	}
PageEdit:KioskHiTecClassFuncsEdit.save.onClick() {
		not( this.confirm("클래스 함수를 저장하시겠습니까?") ) {
			return;
		}
		x.saveSrc();
		if( x.error ) {
			this.alert("클래스 함수 저장중 오류가 발생했습니다. 오류내용: $x[error]");
		}
		templateType=getParentObject(this,'pageTemplateType');
		if( templateType.eq('page') ) { 
			page=get('PageEdit.KioskHiTecPageTagTree');
			page.reloadClassFunc();
		}
	}
PageEdit:KioskHiTecClassFuncsEdit.src.onKeyDown() {
		if( x.editorKeyDown( @key, @mode) ) 
			return true;
		not( @mode&KEY.ctrl ) return false;
		switch(@key) {
		case KEY.R: 			this.fireEvent('run.onClick');
		case KEY.S: 			this.fireEvent('save.onClick');
		}
		return false;
	}
PageEdit:KioskHiTecClassFuncsEdit.run.onClick() {
		x.runSrc();
		if( x.error ) {
			this.alert("클래스 함수 실행중 오류가 발생했습니다. 오류내용: $x[error]");
		}
	}
PageEdit:KioskHiTecPageFuncsEdit.initPage(root, append) {
		page=root.currentPage;
		root[cmsCode]=page[@cms.code];
		root[pageCode]=page[id];
		root.inject(cmsCode, pageCode);
		x.initPage(root, append);
		this.pageInfo.value("${cmsCode}.${pageCode}");
	}
PageEdit:KioskHiTecPageFuncsEdit.onInit() {
		include('common/widget.PageFuncsManager');
		x=newClass('common/widget.PageFuncsManager', this);
		this.autoRunCheck.checked(true);
	}
PageEdit:KioskHiTecPageFuncsEdit.save.onClick() {
		not( this.confirm("페이지 함수를 저장하시겠습니까?") ) {
			return;
		}
		x.saveSrc();
		if( x.error ) {
			this.alert("페이지 함수 저장중 오류가 발생했습니다. 오류내용: $x[error]");
		}
		page=get('PageEdit.KioskHiTecPageTagTree');
		page.reloadPageFunc();
	}
PageEdit:KioskHiTecPageFuncsEdit.src.onKeyDown() {
		if( x.editorKeyDown( @key, @mode) ) 
			return true;
		not( @mode&KEY.ctrl ) return false;
		switch(@key) {
		case KEY.R: 			this.fireEvent('run.onClick');
		case KEY.S: 			this.fireEvent('save.onClick');
		}
		return false;
	}
PageEdit:KioskHiTecPageFuncsEdit.run.onClick() {
		x.runSrc();
		if( x.error ) {
			this.alert("페이지 함수 실행중 오류가 발생했습니다. 오류내용: $x[error]");
		}
	}
PageEdit:KioskHiTecPageTagTree.initPage(node, page, subpage) {
		not( page ) return;
		if( page==currentPage ) {
			return;
		}
		@currentNode=node;
		@currentPage=page;
		@currentClass=null;
		info=Cf.info('funcVar', page, 'init');
		map=getClassInfo(info, 'classMap', classComboNode.initNode() );
		combo=this.pageClassCombo;
		combo.removeAll().addItem(map, 'key,value', '= 페이지 클래스 선택 =');
		
		comboImpl.makeComboData();
		not( subpage ) treeImpl.initTree(page);
		
		pageFuncGridPage.initPage(page, page);
		classFuncGridPage.initPage();
		tab.current(pageFuncGridPage);
	}
PageEdit:KioskHiTecPageTagTree.onInit() {
		currentPage=null;
		currentClass=null;
		currentNode=null;
		classComboNode={};
		include('common/widget.PageTagTree');
		comboImpl 				= newClass('common/widget.ClassComboSelect', this);
		treeImpl 					= newClass('common/widget.PageTagTree', this);
		
		pageFuncGridPage		= this.widget(conf('page#dev.PageFuncGrid'));
		classFuncGridPage		= this.widget(conf('page#dev.ClassFuncGrid')); 
		
		tab=this.gridTab;
		tab.addPage(pageFuncGridPage, '페이지 함수', 'vicon.basket_put');
		tab.addPage(classFuncGridPage, '클래스 함수', 'vicon.bricks_defalut');
		tab.current(pageFuncGridPage);
	}
PageEdit:KioskHiTecPageTagTree.reloadPageFunc() {
		page=currentPage;
		pageFuncGridPage.initPage(page, page);
		tab.current(pageFuncGridPage);
	}
PageEdit:KioskHiTecPageTagTree.reloadClassFunc() {
		page=currentPage;
		info=Cf.info('funcVar', page, 'init');
		map=getClassInfo(info, 'classMap', classComboNode.initNode() );
		combo=this.pageClassCombo;
		combo.removeAll().addItem(map, 'key,value', '= 페이지 클래스 선택 =');
		classFuncGridPage.initPage();
		tab.current(classFuncGridPage);
	}
PageEdit:KioskHiTecPageTagTree.treeChange(node) {
 		if( node[tag].eq('page') ) {
			parent=node.parent();
			if( parent[tag].eq('page') ) {
				this.initPage(node, parent);
			}
			tab.current(pageFuncGridPage);
		} else if( node[tag].eq('subPage') ) {
			this.initPage(node, node[page], true);
			tab.current(pageFuncGridPage);
		}
	}
PageEdit:KioskHiTecPageTagTree.initClassCombo() {
		classFuncGridPage.initPage(comboImpl);
		tab.current(classFuncGridPage);
	}
PageEdit:KioskHiTecPageTagTree.pageClassCombo.onChange() {
		val=@me.value();
		not( val ) return;
		currentClass=currentPage[$val];
		comboImpl.makeComboData( currentClass );
		classFuncGridPage.initPage(comboImpl);
		tab.current(classFuncGridPage);
	}
PageEdit:KioskHiTecDebugPage.inputFilter.onTextChange() {
		this.filterValue=@me.value();
	}
PageEdit:KioskHiTecDebugPage.onInit() {
		include('common/widget.DebugEditor');
		x=newClass('common/widget.DebugEditor', this);
		w=instance('print.worker'); 
		me=this;
		w.start(callback(node) {
			me.postEvent(1, node);
		});
		this.src.check('wrapUse', true);
		this.inputFilter.value("#");
	}
PageEdit:KioskHiTecDebugPage.onEvent() {
		node=@node;
		not( node.message ) return;
		if( node.message.ch() ) {
			msg=node.message.match();
		} else {
			msg=node.message;
		}
		if( this.filterValue ) {
			not( msg.start(this.filterValue) ) return;
		}
		tm=System.date('HH-mm-ss');
		this.src.append("[$tm] $msg");
	}
PageEdit:KioskHiTecDebugPage.clearEditor.onClick() {
		this.src.clear();
	}
	
	
	
KioskHiTec:main.onInit() {
	include("common.CanvasBase");
	include("KioskHiTec/widget.mainCanvas");
	x=newClass("KioskHiTec/widget.mainCanvas", this);
}
KioskHiTec:PopupTest.onInit() {
	include("common.CanvasBase");
	include("KioskHiTec/widget.PopupTestCanvas");
	x=newClass("KioskHiTec/widget.PopupTestCanvas", this);
}
KioskHiTec:protocalTest.onInit() {
	include("KioskHiTec/widget.protocalTest");
	x=newClass("KioskHiTec/widget.protocalTest", this);
	
	this.reqType.addItem( getCommCodeNode('kiosk#reqType'), 'code,value', '요청타입 선택');
	this.delay( callback() {
		splitter=this.tag('splitter');
		splitter.sizes(1,320,0);
		splitter.sizes(300);
	}); 
	this[MS_NO].value('E0022V');
	this[POS_NO].value('02');
	this[EMP_ID].value('E0022V');
	this[EMP_PW].value('SH0003A');
}
KioskHiTec:protocalTest.request(url) {
	me=this;
	Class.web().call(url, callback(type, data) {
		switch( type ) {
		case read:		x.responseXML(data.ref(), me.reqType );
		case finish:		x.responseFinish(url);
		case error:
			me.alert("$url 호출 오류 \n$data");
		}
	});
}
KioskHiTec:protocalTest.requestUrl.onEnter() {
	this.fireEvent('send.onClick');
}
KioskHiTec:protocalTest.send.onClick() {
	this.checkSendAll=false;
	url=this.requestUrl.value();
	not( url ) {
		this.alert("호출 url 를 입력하세요");
		this.requestUrl.focus();
		return;
	}
	this.request(url);
}
KioskHiTec:protocalTest.reqType.onChange() {
	this.requestUrl.value( this.getUrl(true) );
}
KioskHiTec:dbManager.onInit() {
	db = Class.db('config');
	sql = "select dsn, driver, server, dbnm, uid, pwd, port from db_info";
	this[grid].check('treeMode', false);
	this[grid].model(localModel(),
		"check:*#40, dsn:DB아이디#90, driver:경로/드라이버#180, server:서버#110, dbnm:데이터베이스명#190, uid:사용자#80, pwd:비밀번호#80, port:포트#70");
	this[applyPassword].hide();
	this.search();

}
KioskHiTec:dbManager.apply.onClick() {
	&insert 		= class('db').insertQuery('db_info', 'dsn, driver, server, dbnm, uid, pwd, port');
	&update 	= class('db').updateQuery('db_info', 'driver, server, dbnm, uid,  port', 'dsn');
	root = this[grid].rootNode();
	while( cur, root ) {
		pw=cur[pwd].encode();
		cur[pwd]=pw;
		if( cur.state(NODE.add) ) {
			db.exec(insert,cur); 
		} else if( cur.state(NODE.modify) ) {
			db.exec(update,cur);
		}	
	}
	this.search();
}
KioskHiTec:dbManager.applyPassword.onClick() {
	while( cur, this[grid].rootNode() ) {
		not( cur[checked] ) continue;
		db.exec("update db_info set pwd=#{pwd} where dsn=#{dsn}", cur);
	}
	this.search();
}
KioskHiTec:dbManager.gridInput( index) {
	input  = {
		tag: input, 
		onKeyDown() {
			not( @key.eq( KEY.Enter, KEY.Return) ) return; 
			this[mainWindow].nextFocus(this, @mode&KEY.ctrl);
		}
		init( main, index) {
			this[mainWindow] = main;
			this[index] = index;
		}
	}
	input.init( this, index);
	return input;
}
KioskHiTec:dbManager.grid.onDraw() {
	this.drawGrid(@draw, @node );
}
KioskHiTec:dbManager.grid.onClicked() {
	b1=this[applyPassword], b2=this[delete];
	_check=func(grid, node) { 
		node.toggle('checked');
		grid.update();
		bchk = false;
		while( cur, grid.rootNode() ) {
			if( cur[checked] ) {
				bchk=true;
				break;
			}
		}
		if( bchk ) {
			b1.show(), b2.show();
		} else {
			b1.hide(), b2.hide();
		} 
	};
	switch( @column ) {
	case 0: 
		_check(@me, @node);
	default:
		@me.edit(@node,@column); 
	}
}
KioskHiTec:dbManager.grid.onEditEvent(type, node, data, index) {
	&pos = @me.offset(); 
	&hh = @me.headerHeight();
	switch(type) {
	case create:		 
		@me.check('sortEnable',false);
		return this.gridInput(index);
	case finish:
		field=@me.field(index);  
		not( node[$field].eq(data) ) {
			not( node.state(NODE.add) ) {
				node.state(NODE.modify, true);
			}
			if( field.eq('pwd') ) {
				node[$field] = data.encode();
			} else {
				node[$field] = data;
			}
		}
		@me.update();
		@me.check('sortEnable',true);
	default: return;
	}
}
KioskHiTec:dbManager.nextFocus(input, ctrl) {
	node = this[grid].current();
	if( ctrl ) {
		next = this[grid].nextNode(node);
		if( next ) {
			this[grid].current(next);
			this.delay( callback() {
				this[grid].edit(next,input[index]);
			});	
		} else {
			this.delay( callback() {
				this.fireEvent('add.onClick');
			});
		}
	} else {
		index = input[index] + 1;
		if( index>6 ) return;
		this.delay( callback() {
			this[grid].edit(node,index);
			this[grid].scroll(node,index);
		});	
	}
}
KioskHiTec:dbManager.add.onClick() {
	root = this[grid].rootNode();
	cur= root.addNode();
	gridAddRow(this[grid], cur, 1);
}
KioskHiTec:dbManager.search() {
	root = this[grid].rootNode();
	root.removeAll();
	db.fetchAll(sql, root);
	this[grid].selectClear();
	this[grid].update();
	this[delete].hide();
}
KioskHiTec:dbManager.drawGrid(d, node) {
	grid = this[grid];
	rc=d.rect();
	modify = drawGridModify(d,node,rc);
	not( modify ) {
		if( d.state(STYLE.Selected) ) {	
			d.fill( rc, '#f0f0f0' );
		} else {
			d.fill();
		}
	}
	field=grid.field(d.index());
	switch( field ) {
	case check:		
		if( node[checked] ) {
			d.icon(rc.center(16.16), Icon.func.check);
		} else {
			d.icon(rc.center(16.16), Icon.func.add);
		}
	case pwd:
		d.text( rc.incrX(2), node[$field].decode() );	
	default:
		d.text( rc.incrX(2), node[$field] );	
	} 
	d.rectLine(rc,4,'#d0d0d0');
	if( modify ) this[apply].show();
}
KioskHiTec:protocalTest.downloadImage(root, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	path="project/KioskHiTec/images/menus";
	url="http://61.78.39.132/DRIM/data.dir/prod_img/E002";
	downloadNext=callback() {
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[GOODS_IMG];
			print("download========= $file");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					print("download finish========= $file");
					menu[down_yn]='Y';
					downloadNext();
				case error:
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:DbQuery.onInit() {
	include("KioskHiTec/widget.QueryTest");
	x=newClass("KioskHiTec/widget.QueryTest", this);
	x.initTree();
}
KioskHiTec:KioskLogViewer.startLog.onClick() {
	me=this;
	Class.worker('log').start( callback() {
		logCheck=_node('watcherLogCheck');
		logCheck.inject( log, logPos);
		not( log ) {
			log=Class.file('log');
			logPath=conf('setup.kiosk#logPath');
			ff= Class.filefind();
			maxDt=0, cur=0;
			while( c, ff.fetchAll(logPath) ) {
				x=c[modifyDate];
				if( maxDt<x ) {
					cur=c;
					maxDt=x;
				}
			}
			log.open( "$logPath\\$cur[fileName]", "read" );
			logPos=log.size();
			logCheck.put( log, logPos);
			print( maxDt, cur, logCheck);
		}
		logSize=log.size();
		if( logPos<logSize ) {
			str=log.read(logPos, true);
			logCheck[data]=str.utf8();
			me.postEvent(1, logCheck);
			logCheck[logPos]=logSize;
			print( logSize, logPos );
		}
		System.sleep(500);
	});	
}
KioskHiTec:KioskLogViewer.onInit() {
}
KioskHiTec:KioskLogViewer.clearLog.onClick() {
	this[log].clear();
}
KioskHiTec:PingTest.onInit() {
	include("KioskHiTec/widget.PingTestGrid");
	x=newClass("KioskHiTec/widget.PingTestGrid", this);
	x.initPage();
	this.delay( callback() {
		this.tag('splitter').sizes(1,500);
		this.tag('splitter').sizes(200);
	});
	this.startMon=false;
	setCommCombo(this.serverInfo, 'kiosk#serverInfo', '=서버 선택=');
}
KioskHiTec:PingTest.serverSetup.onClick() {
	p=pageLoad('Common.ConfManager');
	p.open(this,'center');
	p.initPage('cc.kiosk#serverInfo');
}
KioskHiTec:PingTest.serverInfo.onChange() {
	this.serverIp.value( this.getServerDetail('data') );
}
KioskHiTec:PingTest.run.onClick() {
	ip=this.serverIp.value();
	not( ip ) {
		this.alert("서버 아이피를 설정하세요 ");
		return;
	}
	not( this.pingProc ) {
		this.runPingProc();
	}
	proc=this.pingProc;
	proc.command("ping $ip");
}
KioskHiTec:PingTest.runPingProc() {
	proc=Class.process('pingCmd');
	if( proc.run() ) return proc;
	proc.run('cmd', callback(type, data) {
		switch( type) {
		case read:
			str=data.utf8();
			this.parsePingData(str.ref());
		case finish:
			this.finishPingProc();
		case error:
			this.errorPingProc(data);
		}
	});
	this.pingProc=proc;
	return proc;
}
KioskHiTec:PingTest.finishPingProc() {

}
KioskHiTec:PingTest.errorPingProc(error) {
	this.alert("ping 모니터링 프로세스 오류: $error");
	proc=Class.process('pingCmd');
	proc.stop();
	this.pingProc=null;
	
}
KioskHiTec:PingTest.parsePingData(&str) {
	end=str.ch(-1);
	line=null, desc=null;
	node=_node('#PingInfo');
	
	if( str.start('ping') ) {
		if( end.eq('>') ) {
			str.findPos("\n");
			err=str.findPos("\n").trim();
			this[log].append("## START = $err");
		} else {
			sub=str.findPos("\n");
			sub.move();
			node[ip]=sub.trim();
			node[count]=0;
			node[startTick] =System.tick();
			str.ch();
			if( str.find(':') ) {
				str.findPos(':');
				str.ch();
			}
			line=str.trim();
		}
	} else {
		node[count++];
		line=str.findPos("\n").trim();
		if( str.ch() ) {
			node[desc]=str.trim();
			node[endCheck]=true;
			node[endTick]	=System.tick();
			this.makePingStatus(node, line.ref() );
		}
	}
	if( line ) {
		node.inject(ip, count);
		if( line.find(':') ) {
			this[log].append("## $count> $line", true);
		} else {
			this[log].append("## ERROR $count> $ip:$line");
			if( count.eq(4) ) {
				node[error]=line;
				this.makePingErrorLog(node);
			}
		}
	}
}
KioskHiTec:PingTest.getServerDetail(field) {
	val=this.serverInfo.value();
	not( val ) return null;
	root = getCommCodeNode('kiosk#serverInfo');
	printNode(root);
	find = root.findOne('code',val);
	not( field ) return find;
	return find[$field];
}
KioskHiTec:processInfoView.onInit() {
	include("KioskHiTec/widget.processInfoViewGrid");
	x=newClass("KioskHiTec/widget.processInfoViewGrid", this);
	this[killProcess].hide();
	x.initPage();
}
KioskHiTec:processInfoView.reload.onClick() {
	this[killProcess].hide();
	x.search();
}
KioskHiTec:processInfoView.processMonitor.onClick() {
	this.alert("감시 시작 클릭");
}
KioskHiTec:KioskLogViewer.onEvent() {
	node=@node;
	this[log].append( node[data] );
}
KioskHiTec:PingTest.reload.onClick() {
	setCommCombo(this.serverInfo, 'kiosk#serverInfo', '=서버 선택=', true);
}
KioskHiTec:LoginPage.onInit() {
	setCommCombo( this[int_gb], 'kiosk#INT_GB', '= 선택 =');
}
KioskHiTec:AdminMenu.onInit() {
	include("common.CanvasBase");
	include("KioskHiTec/widget.AdminMenuCanvas");
	x=newClass("KioskHiTec/widget.AdminMenuCanvas", this);
	
	db=Class.db('kiosk_hitec');
	cf={tag: confing };
	_log=func(s) {
		e=cf[logEditor];
		logType=null;
		if( s.start('##') ) {
			logType='E';
		} else if( s.start('#') ) {
			logType='I';
		}
		if( e && logType ) {
			e.append(s);
		} else {
			print("# $s");
		}
	};
	Cf[KioskWatcher]=this;
	
	tray = this.tray();
	tray.icon('vicon.bricks_defalut');
	tray.action([
		{id: tray.adminView,		text:관리자 화면열기,	icon: vicon.user_suit },
		{id: tray.setupView,		text:설정화면,				icon: vicon.application_form },
		{id: tray.queryView,		text:DB 쿼리툴,			icon: vicon.application_form_add },
		{id: tray.protocalView,	text:전문 테스트,			icon: vicon.application_form_magnify },
		{id: tray.pingTest,			text:네트워크 테스트,	icon: vicon.application_key },
		{id: tray.close,				text:종료,						icon: vicon.cancel_default }
	]);
	tray.contextMenu([tray.adminView,-,tray.logView,-,tray.queryView,tray.protocalView,tray.pingTest,-,tray.close]);
	tray.show();
	tray.onActivated=callback() {
		type=tray.reason;
		if( type.eq('click') ) pageLoad('KioskHiTec.adminSetup') .open();
	};
	
	this.timer(5000, callback() {
		watcherBatch(this, db);
	});
	this.initPage();
	this.qtMonStart();
}
KioskHiTec:PingTest.makePingErrorLog(node) {
	not( node[target] ) {
		node[target]=this.serverInfo.value();
		node[targetName]=this.serverInfo.text();
	}
	Class.db('kiosk_hitec').exec(conf('sql#hitec.addPingError'), node);
	x.search();
}
KioskHiTec:PingTest.makePingStatus(node, &line) {
	print("makePingStatus=>$node, $line");
}
KioskHiTec:PingTest.start.onClick() {
	if( this.startMon ) {
		this.stopMonitoring();
	} else {
		this.startMonitoring();
	}
}
KioskHiTec:PingTest.timeout() {
	node=_node('#PingInfo');
	if( node[startTick] ) {
		not( node[endCheck] ) {
			dist=System.tick() - node[startTick];
			if( dist>60000 ) {
				node[endCheck]=true;
			}
		}
	} else {
		node[startTick]=System.tick();
		node[endCheck]=true;
	}
	
	if( node[endCheck] ) {
		dist=System.tick() - node[endTick];
		if( dist<30000 ) {
			return;
		}
		targetNode=getCommCodeNode('kiosk#serverInfo');
		idx=node[runIndex++], total=targetNode.childCount();
		n=idx % total;
		cur=targetNode.child(n);
		print("timeout target=$cur");
		node[target]			=cur[code];
		node[targetName]	=cur[value];
		node[ip]				=cur[data];
		node[endCheck]	=false;
		node[callTick]		=System.tick();
		node[error]			=null;
		proc=this.runPingProc();
		proc.command("ping $node[ip]");
	}
}
KioskHiTec:protocalTest.serverInfoConf.onClick() {
	p=pageLoad('Common.ConfManager');
	p.initPage('cc.kiosk#reqType');
	p.open(this,'center');
}
KioskHiTec:protocalTest.serverInfoReload.onClick() {
	getCommCodeNode('kiosk#reqType', null, true);
	combo=this.reqType;
	combo.removeAll();
	combo.addItem( getCommCodeNode('kiosk#reqType'), 'code,value', '요청타입 선택');
	grid=this.grid;
	grid.rootNode().removeAll();
	grid.update();
}
KioskHiTec:AdminMenu.downloadResource(url, path, root, field) {
	web = Class.web('KioskDownload');
	not( url ) {
		url="http://61.78.39.132/DRIM/data.dir/prod_img/E002";
	}
	not( path ) {
		path="data/images/menus";
	}
	not( field ) {
		field='GOODS_IMG';
	}
	downloadNext=callback() {
		while( cur, root ) {
			if( cur[down_yn].eq('Y') ) continue;
			file=cur[$field];
			print("downloadResource => $url/$file");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					cur[down_yn]='Y';
					print("download menu ok =>$path/$file");
					downloadNext();
				case error:
					cur[error]=data;
					print("download error=> $cur[error]");
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:dbManager.delete.onClick() {
	not( this.confirm("선택된 DB연결을 삭제하시겠습니까?") ) {
		return;
	}
	while( cur, this.grid.rootNode() ) {
		not( cur[checked] ) continue;
		db.exec("delete from db_info where dsn=#{dsn}", cur);
	}
	this.search();
}
KioskHiTec:dbManager.reload.onClick() {
	this.search();
}
KioskHiTec:OrderTool.cartDelete.onClick() {
	arr=_arr();
	grid=this[g1], cart=this[g2];
	src=cart.rootNode(), dest=grid.rootNode();
	while( cur, src ) {
		not( cur[checked] ) continue;
		cur[checked]=false;
		arr.add(cur);
		dest.addNode(cur);
	}
	while( cur, arr ) {
		src.remove(cur);
	}
	grid.update(), cart.update();
}
KioskHiTec:OrderTool.onInit() {
	db=Class.db('kiosk_hitec');
	grid=this[g1], cart=this[g2];
	grid.model( Class.model('g1'), 'value' );
	cart.model( Class.model('g2'), 'value' );
	this.makeGrid();
	this.makeCombo();
}
KioskHiTec:OrderTool.g1.onClicked() {
	if( @column.eq(0) ) {
		gridCheck(@me, @node, this.cartSelect );
	}
}
KioskHiTec:OrderTool.g1.onDraw() {
	this.drawGrid(@draw, @node, @me );
}
KioskHiTec:OrderTool.g2.onClicked() {
	if( @column.eq(0) ) {
		gridCheck(@me, @node, this.cartDelete );
	}
}
KioskHiTec:OrderTool.g2.onDraw() {
	this.drawGrid(@draw, @node, @me );
}
KioskHiTec:OrderTool.cartSelect.onClick() {
	arr=_arr();
	grid=this[g1], cart=this[g2];
	src=grid.rootNode(), dest=cart.rootNode();
	while( cur, src ) {
		not( cur[checked] ) continue;
		cur[checked]=false;
		arr.add(cur);
		dest.addNode(cur);
	}
	while( cur, arr ) {
		src.remove(cur);
	}
	grid.update(), cart.update();
}
KioskHiTec:OrderTool.search.onClick() {
	this.searchMenu();
}
KioskHiTec:OrderTool.drawGrid(d, node, grid) {
	rc=d.rect();
	modify = drawGridModify(d,node,rc);
	not( modify ) {
		if( d.state(STYLE.Selected) ) {
			d.fill( rc, '#f0f0f0' );
		} else {
			d.fill();
		}
	}
	field=grid.field(d.index());
	switch( field ) {
	case check:
		if( node[checked] )
			d.icon(rc.center(16.16), Icon.func.check);
		else
			d.icon(rc.center(16.16), Icon.func.add);
	case pwd:
		d.text( rc.incrX(2), node[$field].decode() );
	default:
		d.text( rc.incrX(2), node[$field] );
	}
	d.rectLine(rc,4,'#d0d0d0');
	if( modify ) this[apply].show();
}
KioskHiTec:OrderTool.makeCombo() {
	node={};
	db.fetchAll("SELECT clplu_cd, clplu_nm FROM hitec_m03s", node);
	combo=this.cornerCombo;
	combo.removeAll();
	combo.addItem(node,'clplu_cd, clplu_nm', '=전체=');
	setCommCombo(this.soldOutCombo,'kiosk#soldOut', '=전체=');
}
KioskHiTec:OrderTool.makeGrid() {
	root=grid.rootNode();
	db.fetchAll("SELECT * FROM hitec_m10s limit 1 offset 0", root.removeAll(), true ), err=db.error();
	if( err ) {
		this.alert("DB조회 오류 :\n $err");
		grid.update();
		return;
	}
	node= root.child(0);
	if( node ) {
		s="";
		while( field, root[@fields], n, 0) {
			w=gridMaxFiledWidth(root, field);
			if( n ) s.add(",");
			s.add("$field:$field #", min(w,350) );
		}
		fields=grid.fields();
		gridMakeField(s, true, fields);
		grid.fields(fields);
		cart.fields(fields);
	}
	root.removeAll();
	grid.update();
	cart.update();
	this.searchMenu();
}
KioskHiTec:OrderTool.searchMenu() {
	root=grid.rootNode();
	root[clplu_cd]=this.cornerCombo.value();
	db.fetchAll("SELECT * FROM hitec_m10s where 1=1 #[clplu_cd? and clplu_cd=#{clplu_cd}]", root.removeAll(), true ), err=db.error();
	total=root.childCount();
	this[gridStatus].value("(총 $total 건) ");
	this[cartSelect].hide();
	this[cartDelete].hide();
	grid.update();
}
KioskHiTec:OrderTool.printBill.onClick() {
	while( cur, this.g2.rootNode() ) {
		print("cur=$cur");
	}
}
KioskHiTec:protocalTest.sendLast.onClick() {
	url=this.getUrl();
	this.requestUrl.value( url );
	this.request( url );
}
KioskHiTec:protocalTest.sendAll.onClick() {
	this.checkSendAll=true;
	url=this.getUrl(true);
	this.requestUrl.value( url );
	this.request( url );
}
KioskHiTec:protocalTest.getUrl(all ) {
	val=this.reqType.value();
	not( val ) {
		val='X10S';
	}
	node=getCommCodeNode('kiosk#reqType');
	cur=node.findOne('code', val);
	not( cur ) {
		this.alert("호출 url 를 찾을수 없습니다 URL를 확인하세요");
		return; 
	}
	ms_no=this[MS_NO].value();
	pos_no=this[POS_NO].value();
	emp_id=this[EMP_ID].value();
	emp_pw=this[EMP_PW].value();
	if( all ) {
		last_seq='0000';
	} else {
		last_seq=Class.db('kiosk_hitec').value("SELECT  max(log_seq) as log_seq FROM HITEC_${val}");
	}
	return fmt(cur[data]);
}
KioskHiTec:protocalTest.onEvent() {
	cur=@node;
	switch( @type ) {
	case 1:
		x.makeProtocalData();
	default:
	}
}
KioskHiTec:AdminMenu.wasStart() {
	was=Class.was('Kiosk');
	was.start(8089,"data/webpages");
}
KioskHiTec:AdminMenu.openLogView() {
	p=this[logView];
	not( p ) {
		p=pageLoad('KioskHiTec.KioskLogViewer', true);
		this[logView]=p;
	}
	p.flags('top,splash');
	p.move(0,0);
	p.size(1000,220);
	p.open();
}
KioskHiTec:AdminMenu.onEvent() {
	node=@node;
	switch( @type ) {
	case 1:
		this.openLogView();
	case 2:
		this.openThisPage();
	case 51:
			this.responseXML(node[data].ref(), node );
		this.updateStart();
	case 10:	x.qtMonRecvData( node[data], node );
	case 11:
		x.easyCardReadData(  node[data], node );
		cf[easyCardTick]=0;
	case 12:
		x.easyCardError( node[data], node );
		cf[easyCardTick]=0;
	default:
	}
}
KioskHiTec:main.onEvent() {
	node=@node;
	switch( @type ) {
	case 1:	x.qtMonRecvData( node[data], node );
	case 11:	x.easyCardReadData(  node[data], node );
	case 12:	x.easyCardError( node[data], node );
	case 21:	x.hitecResponseOk( node[data], node );
	case 22:	x.hitecResponseError( node[data], node );
	}
}
KioskHiTec:main.qtMonStart() {
	qtMonNode=_node('QtMonNode'); 
	worker = Class.worker("QtMon");
	qtMonNode[socket]=Class.socket("QtMon");
	x.inject( cf );
	
	me=this;
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist <13000 ) {
			return;
		}
		qtMonSocket = qtMonNode[socket];
		not( qtMonSocket.isConnect() ) {
			if( qtMonSocket[connected] ) {
				System.sleep(100);
				idx=cf[socket_idx++];
				qtMonNode[socket]=Class.socket("socket_$idx");
				print("qtMon 소켓 새로 생성($idx) : $qtMonNode");
			}
			not( qtMonSocket.connect( cf[qtMonHost], cf[qtMonPort], 30000) ) {
				qtMonSocket.close();
				order=cf[OrderHeader];
				if( order[total_qty] ) {
					return;
				}
				x._log("## 디바이스 연결오류, 호스트:$cf[qtMonHost], 포트: $cf[qtMonPort]", true);
				System.sleep(5000);
				return;
			}
			x.qtMonSendData('01,4,1,0,1,0');
			x.qtMonSendData('21,01,1,1');
			qtMonSocket[connected]=true;
			Class.db('kiosk_hitec').exec("update kiosk_error set error_status='S' where error_kind='qtmon' and error_status='R' ");
		}
		recv= qtMonSocket.readBuffer();
		if( recv ) {
			print("QtMon 응담 : $recv");
			qtMonNode[data]=recv;
			me.postEvent(1, qtMonNode);
		} else {
			print("QtMon read timeout or exception !!");
		}
	});
}
KioskHiTec:main.hitecSend(req, xml) {
	web=Class.web('HiTec');
		web[data]=xml;
		me=this;
	web.call( req, callback(type, data) {
		switch(type) {
		case read:
			req[data] = data;
			me.postEvent(21, req);
		case finish:
			req.status=null;
		case error:
			req[data] = data;
			me.postEvent(22, req);
			}
	});
}
KioskHiTec:main.easyCardSend(req, callback) {
	web=Class.web('EasyCard');
	web.timeout(90000);
		me=this;
	web.call( req, callback(type, data) {
		switch(type) {
		case read:
			req[data] = data;
			me.postEvent(11, req);
		case finish:
			req.status=null;
		case error:
			req[data] = data;
			print("easyCardSend error: $data");
			me.postEvent(12, req);
			}
	});
}
KioskHiTec:LoginPage.apply.onClick() {
	
}
KioskHiTec:LoginPage.c.onDraw() {
	d=@draw,  rc=d.rect();
	d.drawImage(rc, commonImage('admin_title') );
	d.font(28,'bold','#fd9437').text( rc.incrX(25), "키오스크 설정");
}
KioskHiTec:protocalTest.downloadM06s(root, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	path="project/KioskHiTec/images/menus";
	url="http://61.78.39.132/DRIM/data.dir/prod_img/E002";
	downloadNext=callback() {
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			print("downloadM06s========= $menu");
			not( menu[set_cd].eq('001','002','003') ) continue;
			file=menu[set_val]; 
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/kiosk_$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					print("download finish========= $file");
					menu[down_yn]='Y';
					downloadNext();
				case error:
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:protocalTest.downloadM03s(root, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	path="project/KioskHiTec/images/menus/corner";
	url="http://61.78.39.132/DRIM/data.dir/prod_img/E002";
	downloadNext=callback() {
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[img_file_nm];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					print("download finish========= $file");
					menu[down_yn]='Y';
					downloadNext();
				case error:
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:__download.updateStart(start ) {
	_log("updateStart=======================> [start=$start] ");
	root=cf[updateNode];
	not( root ) {
		root=_node(cf, 'updateNode');
		types='X10S, M06S, M03S, M10S';
		while( tag, types.split() ) {
			cur=root.addNode();
			cur[tag]=tag;
			_log("updateStart : $cur");
		}
	}
	if( start ) {
		while( cur, root ) {
			cur.removeAll();
			cur[finish]=false;
		}
	}
	me=this;
	while( cur, root ) {
		if( cur[finish] ) continue;
		url=this.getUrl(cur[tag], cf[checkAll] );
		_log("## URL=>$url");
		Class.web().call(url, callback(type, data) {
			switch( type ) {
			case read:
				cur[data]=data;
				me.postEvent(1, cur);
			case finish:
				cur[finish]=true;
			case error:
				_log("## $url 호출 오류 \n$data");
			break;
			}
		});
		_log("## URL END============================");
		return;
	}
}
KioskHiTec:__download.downloadMenuImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="project/KioskHiTec/images/menus";
	downloadNext=callback() {
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[GOODS_IMG];
			_log("메뉴다운 : $file");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					downloadNext();
				case error:
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:__download.downloadCornerImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="project/KioskHiTec/images/menus";
	downloadNext=callback() {
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			_log("##코너다운 : $menu");
			not( menu[set_cd].eq('001','002','003') ) continue;
			file=menu[set_val];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/kiosk_$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					downloadNext();
				case error:
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:__download.downloadAdImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="project/KioskHiTec/images/menus/corner";
	downloadNext=callback() {
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[img_file_nm];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			_log("##코너다운 : $menu");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					downloadNext();
				case error:
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:__download.onInit() {
	db=Class.db('kiosk_hitec');
	cf={};
	cf[ms_no]='E00202';
	cf[pos_no]='04'
	cf[emp_id]='E00202';
	cf[emp_pw]='SH0003A';
	e=this.e;
	_log=func(s) {
		e.append(s);
	};
}
KioskHiTec:__download.ok.onClick() {
	e.clear();
	cf[checkAll]=this[checkAll].checked();
	db.exec("update HITEC_M06S  set set_val='E002020427185323_LOGO.png' where set_val='E002020427185323_LOGO.jpg' ");
	this.updateStart(true);
}
KioskHiTec:__download.getUrl(reqType, all ) {
	_log("getUrl=======================> $reqType");
	node=getCommCodeNode('kiosk#reqType');
	cur=node.findOne('code', reqType);
	not( cur ) {
		_log("호출 url 를 찾을수 없습니다 URL를 확인하세요");
		return; 
	}
	cf.inject( ms_no, pos_no,emp_id, emp_pw );
	if( all ) {
		last_seq='0000';
	} else {
		not( reqType.eq('X10S') ) {
			_log("## SELECT  max(log_seq) as log_seq FROM HITEC_${reqType}");
			last_seq=db.value("SELECT  max(log_seq) as log_seq FROM HITEC_${reqType}");	
		}
	}
	return fmt(cur[data]);
}
KioskHiTec:__download.err(data) {
	print("error : xxxxxxx $data xxxxxxxxxxx");
}
KioskHiTec:__download.responseXML(data, root) {
	_log("responseXML ================================> $root[tag]");
	reqType=root[tag];
	parseXml=func(&data, node, fiistNode) {
		not( node ) {
			node=xmlNode;
			node.removeAll();
		}
		while( data.valid() ) {
			ch=data.ch();
			not( ch.eq('<') ) {
				break;
			}
			if( data.ch(1).eq('!') ) {
				data.match('<!--','-->');
				continue;
			}
			if( data.ch(1).eq('?') ) {
				data.match('<?','?>');
				continue;
			}
			sp=data.cur();
			tag=data.incr().move();
			sub = node.addNode();
			not( fiistNode ) {
				fiistNode=sub;
			}
			if( data.ch().eq('-') ) {
				sub[kind]=data.incr().move();
				print("tag--->$tag, $kind");
			}
			if( tag.eq('br', 'space', 'image') ) {
				prop=data.findPos(">");
				this.parseProp( sub, tag, prop);
			} else {
				in=data.find('>');
				if( in.ch(-1).eq('/') ) {
					prop=data.findPos('/>');
					parseProp( sub, tag, prop);
				} else {
					data.pos(sp);
					if( sub[kind] ) {
						in=data.match("<$tag-$sub[kind]","</$tag-$sub[kind]>");
					} else {
						in=data.match("<$tag","</$tag>",8);
					}
					not( in ) {
						print("@@ xml parse $tag not match");
						in=data.findPos("</$tag>");
					}
					prop=in.findPos(">");
					parseProp( sub, tag, prop);
					if( tag.eq('html', 'text') ) {
						val=in.trim();
						if( val ) sub[data]=val;
					} else {
						if( in.ch().eq('<') ) {
							parseXml(in, sub, fiistNode);
						} else {
							val=in.trim();
							if( val ) sub[data]=val;
						}
					}
				}
			}
		}
		return fiistNode;
	}
	
	parseProp=func(node, tag, &prop) {
		node[tag]=tag;
		idx=node.index();
		arr=null;
		not( idx ) arr = _arr(node,'fieldsArray');
		while( prop.valid() ) {
			k=prop.findPos('=').trim();
			not( k ) break;
			if( arr ) arr.add(k);
			ch=prop.ch();
			if( ch.eq() ) {
				node[$k]=prop.match().trim();
			} else if( ch.eq('[') ) {
				in=prop.match();
				arr=[];
				while( in.valid() ) {
					arr.add( in.findPos(',').trim() );
				}
				node[$k]=arr;
			} else {
				node[$k]=prop.findPos(" \t\n",4).trim();
			}
		}
	}
	
	
	
	
	parseXml( data, root.removeAll() );
	cur=findTag('HEADER', root);
	not( cur ) {
		_log("xml 헤더를 찾을수 없습니다. $root");
	}
	sub=cur.child(0);
	if( sub[tag].eq('DETAIL') ) {
		data=sub.child(0);
		this.makeKioskData(reqType, data[fieldsArray], sub);
	} else {
		_log("DETAIL NOT FOUND !!!");
	}
}
KioskHiTec:__download.makeKioskData(tag, fields, root) {
	_log("makeKioskData ================================> ($tag, $fields, $root)");
	table="HITEC_${tag}";
	a='', b='';
	while( k, fields, n, 0 ) {
		if( n ) {
			a.add(',');
		} else {
			b.add(k);
		}
		a.add(k);
	}
	a.add(",tm");
	switch( tag ) {
	case M03S: 	b='CLPLU_CD';
	case M10S:		b='GOODS_CD';
	case M12S:	 	b='GOODS_CD';
	case M05S:		b='CLPLU_CD';
	case M06S:		b='SET_CD';
	}
	
	if( cf[checkAll] ) {
		db.exec("delete from $table");
	}
	ins=getQuery(table, a);
	upd=getQuery(table, a, b);
	_log("update query: $upd");
	
	tm=0;
	while( cur, root ) {
		cur[tm]=tm;
		if( tag.eq("M10S", "M12S") ) {
			
			if( cur[JP_NM] ) {
				decode	= cur[JP_NM].decode('a2u');
				cur[JP_NM]=decode;
			}
			if( cur[CN_NM_GAN] ) {
				decode	= cur[CN_NM_GAN].decode('a2u');
				cur[CN_NM_GAN]=decode;
			}
		}
		if( cur[PROC_GB].eq('D') ) {
			db.exec("update $table set use_yn='N' where $b=#{$b}", cur);
		} else {
			not( db.exec(upd, cur) ) {
				db.exec(ins, cur);
			}
		}
	}
	root.removeAll();
	
	not( cf[prod_img_url] ) {
		db.fetch("select prod_img_url from hitec_x10s limit 1 offset 0", cf);
	}
	
	_log("다운로드 시작: tag=$tag, 경로: $path URL: $url");
	url=cf[prod_img_url], path=conf("setup#kiosk.imagePath");
	
	switch( tag ) {
	case M10S:
		root=_node();
		db.fetchAll("select goods_cd, goods_img from hitec_m10s where goods_img<>'' and tm='0' ", root);
		this.downloadMenuImage(url, root, "$path/menus");
	case M06S:
		root=_node();
		db.fetchAll("select set_cd, set_val from hitec_m06s where set_val<>'' and tm='0' ", root);
		page.downloadM06s(url, root, "$path/menus/kiosk");
	case M03S:
		root=_node();
		db.fetchAll("select clplu_cd, clplu_nm, img_file_nm from hitec_m03s where use_yn='Y' and tm='0' ", root);
		page.downloadM03s(url, root, "$path/menus/corner");
	default:
	}
	tm=System.localtime();
	db.exec("update $table set tm='${tm}' where tm='0' ", cur);
}
KioskHiTec:__download.onEvent() {
	node=@node;
	_log("onEvent ======================> $node ");
	this.responseXML(node[data].ref(), node );
	this.updateStart();
}
KioskHiTec:AdminMenu.updateStart(start, all) {
	if( cf[easyCardTick] ) {
		dist=System.tick() - cf[easyCardTick];
		if( dist<30000 ) {
			print("###### 결제취소중 업데이트 무시 ###### 설정정보 : $cf");
			return;
		}
		cf[easyCardTick]=0;
	}
	
	root=cf[updateNode];
	if( all ) {
		print("###### 전체다시 다운로드 ######");
		root.delete();
		root=null;
	}
	not( root ) {
		root={};
		if( all ) {
			types='X10S, M06S, M03S, M10S, M40S, M21S, M23S, M60S';
		} else {
			types='X10S, M06S, M03S, M10S, M40S';
		}
		db.fetch(conf("sql#hitec.selectKioskSetup"),  cf);
		while( tag, types.split() ) {
			cur=root.addNode();
			cur[tag]=tag;
			_log("# 업데이트 태그정보 : $cur[tag]");
		}
		cf[updateNode]=root;
	}
	if( start ) {
		print("# 업데이트 시작 [start=$cf] ");
		if( cf[startUpdateTick] ) {
			while( cur, root ) {
				print("업데이트 노드 => $cur[finish]");
			}
			db.exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status, tm) values( 'update', 'timeout', '오류', '상품정보 업데이트 오류', 'R', 0 )" );
			tray.hide();
			Cf.exit();
			print("############## 프로그램 종료 오류 ################ ");
			System.kill('KioskWatcher.exe');
			return;
		}
		cf[startUpdateTick]=System.tick();
		cf[UpdateCount]=0;
		cf[UpdateReturnCount]=0;
		while( cur, root ) {
			cur.removeAll();
			cur[finish]=false;
		}
	}
	me=this;
	webIndex=cf[WebIndex];
	web=Class.web("update_$webIndex");
	while( cur, root ) {
		if( cur[finish] ) {
			continue;
		}
		url=this.getUrl(cur[tag], all );
		not( url ) {
			cur[finish]=true;
			continue;
		}
		web.call(url, callback(type, data) {
			switch( type ) {
			case read:
				cur[finish]=true;
				cur[data]=data;
				me.postEvent(51, cur);
			case finish:
				cur[finish]=true;
			case error:
				_log("## $url 호출 오류 \n$data");
			break;
			}
		});
		return;
	}
	dist=System.tick() - cf[startUpdateTick];
	cf[startUpdateTick]=0;
	_log("# makeKioskData($webIndex) : 전체 변경건수 $cf[UpdateCount] 건, 처리시간: ${dist} ms");
	cf[logEditor]=null;
}
KioskHiTec:AdminMenu.downloadMenuImage(url, root, path, reload) {
	print("##### url=$url, root=$root, path=$path, reload=$reload");
	
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="data/images/menus";
	downloadNext=callback() {
		fileCheck=Class.file('down');
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[goods_img];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			if( fileCheck.isFile("$path/$file") ) {
				print("이미 다운로드된 파일: $path/$file");
				menu[down_yn]='Y';
				continue;
			}
			_log("# 메뉴다운 : $file");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					print("download menu ok =>$path/$file");
					downloadNext();
				case error:
					_log("## $url 메뉴 파일:$file 다운로드 오류: $data");
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:AdminMenu.downloadCornerImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="data/images/menus/corner";
	downloadNext=callback() {
		fileCheck=Class.file('down');
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[img_file_nm];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			_log("# 코너다운 : $menu");
			if( fileCheck.isFile("$path/$file") ) {
				print("이미 다운로드된 파일: $path/$file");
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					print("download corner ok =>$path/$file");
					downloadNext();
				case error:
					_log("## $url 코너 파일:$file 다운로드 오류: $data");
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:AdminMenu.downloadAdImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	downloadNext=callback() {
		fileCheck=Class.file('down');
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			not( menu[set_cd].eq('001','002','003') ) continue;
			_log("# 광고 이미지 다운 : $menu");
			file=menu[set_val];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			if( fileCheck.isFile("$path/$file") ) {
				print("이미 다운로드된 파일: $path/$file");
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					print("download ad ok =>$path/$file");
					downloadNext();
				case error:
					_log("## $url 광고 파일:$file 다운로드 오류: $data");
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
KioskHiTec:AdminMenu.getUrl(reqType, all) {
	node=getCommCodeNode('kiosk#reqType');
	cur=node.findOne('code', reqType);
	not( cur ) {
		_log("# 호출 url 를 찾을수 없습니다 URL를 확인하세요 (타입:$reqType)");
		return;
	}
	cf.inject( ms_no, pos_no,emp_id, emp_pw );
	last_seq='0000';
	not( all ) {
		not( reqType.eq('X10S') ) {
			last_seq=db.value("SELECT  max(log_seq) as log_seq FROM HITEC_${reqType}");
			_log("# $reqType => [최종순번:$last_seq]");
			if( cf[logEditor] ) {
				not( last_seq ) last_seq='0000';
			}
			not( last_seq ) return null;
		}
	}
	url= fmt(cur[data]);
	if( cf[logEditor] ) {
		_log("# $reqType => [URL:$url] ");
	}
	return url;
}
KioskHiTec:AdminMenu.responseXML(data, root) {
	reqType=root[tag];
	parseProp=func(node, tag, &prop) {
		node[tag]=tag;
		idx=node.index();
		arr=null;
		not( idx ) arr = _arr(node,'fieldsArray');
		while( prop.valid() ) {
			k=prop.findPos('=').trim();
			not( k ) break;
			if( arr ) arr.add(k);
			ch=prop.ch();
			if( ch.eq() ) {
				node[$k]=prop.match().trim();
			} else if( ch.eq('[') ) {
				in=prop.match();
				arr=[];
				while( in.valid() ) {
					arr.add( in.findPos(',').trim() );
				}
				node[$k]=arr;
			} else {
				node[$k]=prop.findPos(" \t\n",4).trim();
			}
		}
	};
	parseXml=func(&data, node, fiistNode) {
		not( node ) {
			node=xmlNode;
			node.removeAll();
		}
		while( data.valid() ) {
			ch=data.ch();
			not( ch.eq('<') ) {
				break;
			}
			if( data.ch(1).eq('!') ) {
				data.match('<!--','-->');
				continue;
			}
			if( data.ch(1).eq('?') ) {
				data.match('<?','?>');
				continue;
			}
			sp=data.cur();
			tag=data.incr().move();
			sub = node.addNode();
			not( fiistNode ) {
				fiistNode=sub;
			}
			if( data.ch().eq('-') ) {
				sub[kind]=data.incr().move();
			}
			if( tag.eq('br', 'space', 'image') ) {
				prop=data.findPos(">");
				this.parseProp( sub, tag, prop);
			} else {
				in=data.find('>');
				if( in.ch(-1).eq('/') ) {
					prop=data.findPos('/>');
					parseProp( sub, tag, prop);
				} else {
					data.pos(sp);
					if( sub[kind] ) {
						in=data.match("<$tag-${sub[kind]}","</$tag-${sub[kind]}>");
					} else {
						in=data.match("<$tag","</$tag>",8);
					}
					not( in ) {
						in=data.findPos("</$tag>");
					}
					prop=in.findPos(">");
					parseProp( sub, tag, prop);
					if( tag.eq('html', 'text') ) {
						val=in.trim();
						if( val ) sub[data]=val;
					} else {
						if( in.ch().eq('<') ) {
							parseXml(in, sub, fiistNode);
						} else {
							val=in.trim();
							if( val ) sub[data]=val;
						}
					}
				}
			}
		}
		return fiistNode;
	};
	
	parseXml( data, root.removeAll() );
	cur=findTag('HEADER', root);
	not( cur ) {
		_log("# xml 헤더를 찾을수 없습니다. $root");
	}
	sub=cur.child(0);
	not( sub ) {
		return;
	}
	if( sub[tag].eq('DETAIL') ) {
		data=sub.child(0);
		not( data ) {
			return;
		}
		this.makeKioskData(reqType, data[fieldsArray], sub);
	} else {
		_log("DETAIL NOT FOUND !!!");
	}
}
KioskHiTec:AdminMenu.makeKioskData(tag, fields, root) {
	if( fields ) {
		_log("makeKioskData ================================> ($tag, $fields, $root)");
	}
	table="HITEC_${tag}";
	a='', b='';
	while( k, fields, n, 0 ) {
		if( n ) {
			a.add(',');
		} else {
			b.add(k);
		}
		a.add(k);
	}
	a.add(",tm");
	switch( tag ) {
	case M03S: 	b='CLPLU_CD';
	case M10S:		b='GOODS_CD';
	case M12S:	 	b='GOODS_CD';
	case M05S:		b='CLPLU_CD';
	case M06S:		b='SET_CD';
	case M40S:		b='MS_NO';
	case M15S:		b='EMP_NO';
	case M21S:		b='GOODS_CD,PRINT_NO';
	case M60S:		b='VAN_CD';
	}
	cnt=root.childCount();
	not( cnt ) {
		return;
	}
	cf[UpdateCount+=cnt];
	if( cf[checkAll] ) {
		db.exec("delete from $table");
	}
	a.add(', USE_YN');
	ins=getQuery(table, a);
	upd=getQuery(table, a, b);
	
	tm=0;
	while( cur, root ) {
		cur[tm]=tm;
		cur[USE_YN]='Y';
		if( tag.eq('M03S') ) {
			if( cur[CLPLU_NM_JP] ) {
				decode	= cur[CLPLU_NM_JP].decode('a2u');
				cur[CLPLU_NM_JP]=decode;
			}
			if( cur[CLPLU_NM_CH] ) {
				decode	= cur[CLPLU_NM_CH].decode('a2u');
				cur[CLPLU_NM_CH]=decode;
			}
		} else if( tag.eq("M10S", "M12S") ) {
			if( cur[JP_NM] ) {
				decode	= cur[JP_NM].decode('a2u');
				cur[JP_NM]=decode;
			}
			if( cur[CN_NM_GAN] ) {
				decode	= cur[CN_NM_GAN].decode('a2u');
				cur[CN_NM_GAN]=decode;
			}
		}
		if( cur[PROC_GB].eq('D') ) {
			db.exec("update $table set use_yn='N' where $b=#{$b}", cur);
		} else {
			if( cur[PROC_GB].eq('U') ) {
				db.exec(upd, cur);
			} else {
				/* M23S 전문의 경우 무조건 INSERT */
				if(tag.eq('M23S')) {
					db.exec(ins, cur);
				} else {
					not( db.exec(upd, cur) ) {
						db.exec(ins, cur);
					}
				}
			}
		}
	}
	root.removeAll();
	if( tag.eq('X10S') ) {
		return;
	}
	
	not( cf[prod_img_url] ) {
		db.fetch("select prod_img_url from hitec_x10s limit 1 offset 0", cf);
	}
	path=cf[imagePath];
	
	url=cf[prod_img_url], path=conf("setup#kiosk.imagePath");
	checkDown=true;
	
	_log("# 다운로드 시작: tag=$tag, 이미지 경로: $path URL: $url");
	switch( tag ) {
	case M10S:
		root=_node();
		db.fetchAll("select goods_cd, goods_img from hitec_m10s where goods_img<>'' and tm='0' ", root);
		this.downloadMenuImage(url, root, "$path/menus");
	case M06S:
		root=_node();
		db.fetchAll("select set_cd, set_val from hitec_m06s where set_val<>'' and tm='0' ", root);
		this.downloadAdImage(url, root, "$path/menus/kiosk");
	case M03S:
		root=_node();
		db.fetchAll("select clplu_cd, clplu_nm, img_file_nm from hitec_m03s where use_yn='Y' and tm='0' ", root);
		this.downloadCornerImage(url, root, "$path/menus/corner");
	default:
		checkDown=false;
	}
	if( checkDown ) {
		while( cur, root ) {
			not( cur[error] ) continue;
			_log("## 다운로드 에러(태그: $tag) => $cur[error]");
			cur[tag]=tag;
			cur[type]='E';
			cur[info]="node=$cur";
			db.exec( conf('sql#watcher.addDownLoadInfo'), cur);
		}
	}
	tm=System.localtime();
	db.exec("update $table set tm='${tm}' where tm='0' ");
}
KioskHiTec:KioskLogViewer.startUpdate.onClick() {
	kw=Cf[KioskWatcher];
	kw.inject(cf);
	cf[logEditor]=this.log;
	kw.updateStart(true);
}
KioskHiTec:AdminMenu.onAction() {
	switch(@id) {
	case 'tray.setupView':
		pageLoad('KioskHiTec.adminSetup') .open();
	case 'tray.adminView':
		this.openThisPage();
	case 'tray.queryView':
		pageLoad('KioskHiTec.DbQuery') .open();
	case 'tray.protocalView':
		pageLoad('KioskHiTec.protocalTest', true) .open();
	case 'tray.pingTest':
		pageLoad('KioskHiTec.PingTest', true) .open();
		case 'tray.close':
		tray.hide();
		Cf.exit();
	default:
		print();
	}
}
KioskHiTec:AdminMenu.onClose() {
	this.hide();
	return 'ignore';
}
KioskHiTec:KioskLog.startLog.onClick() {
	me=this;
	Class.worker('log').start( callback() {
		logCheck=_node('watcherLogCheck');
		logCheck.inject( log, logPos);
		not( log ) {
			log=Class.file('log');
			logPath=conf('setup.kiosk#logPath');
			ff= Class.filefind();
			maxDt=0, cur=0;
			while( c, ff.fetchAll(logPath) ) {
				x=c[modifyDate];
				if( maxDt<x ) {
					cur=c;
					maxDt=x;
				}
			}
			log.open( "$logPath\\$cur[fileName]", "read" );
			logPos=log.size();
			logCheck.put( log, logPos);
			print( maxDt, cur, logCheck);
		}
		logSize=log.size();
		if( logPos<logSize ) {
			str=log.read(logPos, true);
			logCheck[data]=str.utf8();
			me.postEvent(1, logCheck);
			logCheck[logPos]=logSize;
			print( logSize, logPos );
		}
		System.sleep(500);
	});	
}
KioskHiTec:KioskLog.startUpdate.onClick() {
	kw=Cf[KioskWatcher];
	kw.inject(cf);
	cf[logEditor]=this.log;
	kw.updateStart(true);
}
KioskHiTec:KioskLog.onInit() {
}
KioskHiTec:KioskLog.onEvent() {
	node=@node;
	this[log].append( node[data] );
}
KioskHiTec:KioskLog.clse.onClick() {
	this.hide();
}
KioskHiTec:KioskLog.clearLog.onClick() {
	this[log].clear();
}
KioskHiTec:KioskLogViewer.cancel.onClick() {
	this.hide();
}
KioskHiTec:AdminMenu.openThisPage() {
	p=this;
	p.flags('top,splash');
	screenRect=System.info('screenRect', 0);
	p.move(screenRect.lt() );
	screenRect.size().inject(w,h);
	p.open();
}
KioskHiTec:PingTest.startMonitoring() {
	not( this.pingProc ) {
		this.runPingProc();
	}
	this[start].value("모니터링 중지");
	this.startMon=true;
	this.timer(2000, callback() {
		this.timeout();
	});
	
}
KioskHiTec:PingTest.stopMonitoring() {
	this[start].value("모니터링 시작");
	this.startMon=false;
	while( tm, this.timer() ) {
		this.killTimer(tm);
	}
}
KioskHiTec:AdminMenu.initPage() {
	cf[easyCardTick]=0;
	cf[startUpdateTick]=0;
	cf[WebIndex]=0;
	
	db.value("select send_yn from tb_sale_header limit 1 offset 0");
	if( db.error() ) {
		_log("# 매출전송여부 필드 추가");
		db.exec("ALTER TABLE tb_sale_header ADD send_yn char(1) default 'Y' ");
	}
	
	if( db.count("select count(1) from kiosk_setup where use_yn='Y'") ) {
		db.fetch("
		   SELECT A.ms_no, A.pos_no, A.service_start_time, A.service_end_time, A.refresh_time, A.order_start_no, A.order_end_no,
		 				A.qt_mon_ip, A.qt_mon_port, A.emp_id, A.emp_pw, A.kiosk_id, A.kiosk_pw,
		 				B.van_cd, B.ms_cat_id
	 			  FROM kiosk_setup A
	 			   LEFT JOIN hitec_m60s B ON A.ms_no = B.ms_no AND A.pos_no = B.pos_no AND B.use_yn = 'Y'
			    WHERE A.use_yn='Y'
				  LIMIT 1",  cf);
		return;
	}
	cf[qt_mon_ip]=conf('setup#kiosk.qtMonHost');
	cf[qt_mon_port]=conf('setup#kiosk.qtMonPort');
	cf[easyCardUrl]	= conf('setup#kiosk.easyCardUrl');
		
	db.exec( conf('sql#hitec.addKioskSetup'), cf);
}
KioskHiTec:adminSetup.initPage() {
	watcharPage.inject(cf);
	cf[logEditor]=this.log;	
	
	setup=_node('SetupInfo');
	db=Class.db('kiosk_hitec');
	root=grid.rootNode();
	db.fetch("SELECT REC_MENU_YN FROM hitec_m10s limit 1 offset 0");
	if( db.error() ) {
		db.exec("ALTER TABLE hitec_m10s ADD REC_MENU_YN text");
		db.exec("ALTER TABLE hitec_m10s ADD REC_MENU_SEQ integer");
	}
	
	db.fetch(conf("sql#hitec.selectKioskSetup"),  setup);
	db.fetchAll(conf("sql#hitec.CornerInfo"), root.removeAll() );
	setFormValue(this, setup);
	grid.update();
	
	this[updateAll].enable();
	
	this.show();
	/* 이전에 세팅된 타이머가 있다면 삭제*/
	while( tm, this.timer() ) {
		this.killTimer(tm);
	}
	/* 자동 hide 타이머 설정(10초) */
	me=this;
	this.timer( 10000, callback() {
		if( cf[logEditor] ) {
			return;
		}
		if( isFirstLoad ) {
			me.hide();	
			isFirstLoad = 0;
		}		
	});
}
KioskHiTec:adminSetup.onInit() {
	isFirstLoad=1;
	db=Class.db('kiosk_hitec');
	watcharPage=Cf[KioskWatcher];
	grid=this.grid;
	model=Class.model('AdminSetup');
	grid.model( model, 'clplu_cd: 매장코드, clplu_nm:매장명, img_file_nm: 이미지명, kitchen_ip1: 주방IP, screen_ip:주문스크린IP');
	this.initPage();
}
KioskHiTec:adminSetup.cornerDown.onClick() {
	not( this.confirm('매장 DB를 초기화하고, 다시 다운로드 받으시겠습니까?') ) {
		return;
	}
	not( watcharPage ) watcharPage=Cf[KioskWatcher];
	watcharPage.inject(cf);
	cf[logEditor]=this.log;
	db.exec('delete from hitec_m03s');
	watcharPage.updateStart(true);
}
KioskHiTec:adminSetup.cancel.onClick() {
	watcharPage.inject(cf);
	cf[logEditor]=null;
	this.alert("어드민 키오스크에서 실행하거나 트레이 아이콘에서 실행하세요");
	this.hide();
}
KioskHiTec:adminSetup.goodsDown.onClick() {
	not( this.confirm('상품 DB를 초기화하고, 다시 다운로드 받으시겠습니까?') ) {
		return;
	}
	not( watcharPage ) watcharPage=Cf[KioskWatcher];
	watcharPage.inject(cf);
	cf[logEditor]=this.log;
	db.exec('delete from hitec_m10s');
	watcharPage.updateStart(true);	
}
KioskHiTec:adminSetup.adDown.onClick() {
	not( this.confirm('광고 DB를 초기화하고, 다시 다운로드 받으시겠습니까?') ) {
		return;
	}
	not( watcharPage ) watcharPage=Cf[KioskWatcher];
	watcharPage.inject(cf);
	cf[logEditor]=this.log;
	db.exec('delete from hitec_m06s');
	watcharPage.updateStart(true);
}
KioskHiTec:adminSetup.reload.onClick() {
	setupProcess(db, cf);
	this.initPage();
	this.setLogEditor();
}
KioskHiTec:AdminMenu.qtMonStart() {
	cf[qt_mon_ip]=conf('setup#kiosk.qtMonHost');
	cf[qt_mon_port]=conf('setup#kiosk.qtMonPort');
	node=_node('QtMonNode');
	socket=node[socket];
	not( socket ) {
		socket=Class.socket("QtMon");
		node[socket]=socket;
		node[socketStartTick]=System.tick();
	}
		
	worker = Class.worker("QtMon");
	me=this;
	worker.start( func() {
		dist = System.tick() - node[socketStartTick];
		if( dist < 15000 ) {
			return;
		}
		not( socket.isConnect() ) {
			not( socket.connect( cf.qt_mon_ip, cf.qt_mon_port, 30000) ) {
				socket.close();
				_log("## 디바이스 연결오류, 호스트:$cf[qtMonHost], 포트: $cf[qtMonPort]", true);
				System.sleep(2000);
				return;
			}
		}
	
		recv= socket.readBuffer();
		if( recv ) {
			node[data]=recv;
			me.postEvent(10, node);
		}
	});
}
KioskHiTec:AdminMenu.easyCardSend(req, callback) {
	cf[easyCardTick]=System.tick();
	web=Class.web('EasyCard');
		me=this;
	web.call( req, callback(type, data) {
		switch(type) {
		case read:
			_log("easyCardRead : $data");
			req[data] = data;
			me.postEvent(11, req);
		case finish:
			req.status=null;
		case error:
			_log("## easyCardError : $data");
			req[data] = data;
			me.postEvent(12, req);
			}
	});
}
KioskHiTec:adminSetup.adminOpen.onCick() {
	screenRect=System.info('screenRect', 0);
	p=pageLoad('KioskHiTec.AdminMenu');
	p.flags('splash');
	p.move(screenRect.lt() );
	p.size(screenRect.size() );
	p.open();
}
KioskHiTec:adminSetup.updateAll.onClick() {
	not( this.confirm('전체 DB를 다시 다운로드 받으시겠습니까?') ) {
		return;
	}
	not( watcharPage ) watcharPage=Cf[KioskWatcher];
	watcharPage.inject(cf);
	cf[logEditor]=this.log;
	watcharPage.updateStart(true, true);
}
KioskHiTec:adminSetup.dataInit.onClick() {
	not( this.confirm("데이터 베이스를 초기화 하시겠습니까? ") ) {
		return;
	}
	node=_node();
	db.fetchAll("select tablename as table from pg_tables where tableowner='isit' order by tablename", node);
	while( cur, node ) {
		if( cur[table].start('comm_', 'tb_') ) {
			print("테이블명 = $cur[table] skip");
			continue;
		}
		if( cur[table].eq('kiosk_setup') ) {
			print("테이블명 = $cur[table] skip");
			continue;
		}
		if( cur[table].eq('kiosk_print_setup') ) {
			print("테이블명 = $cur[table] skip");
			continue;
		}
		db.exec("delete from $cur[table]");
	}
	cf[reloadAll]=true;
	this.alert("모든 테이블 정보가 삭제되었습니다.\n관리자 페이지에서 키오스크 설정 메뉴에서 정보를 등록 하세요");
}
KioskHiTec:adminSetup.onClose() {
	this.hide();
	return 'ignore';
}
KioskHiTec:adminSetup.adminOpen.onClick() {
	screenRect=System.info('screenRect', 0);
	p=pageLoad('KioskHiTec.AdminMenu');
	p.flags('splash');
	p.move(screenRect.lt() );
	p.size(screenRect.size() );
	p.open();
}
KioskHiTec:main.screenOrderStart1(ipNode) {
	code="ScreenOrder_1";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=11000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(31): $recv");
		} else {
			print("screenOrderStart1 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.screenOrderStart2(ipNode) {
	code="ScreenOrder_2";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=16000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(400);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(32): $recv");
		} else {
			print("screenOrderStart2 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.screenOrderStart3(ipNode) {
	code="ScreenOrder_3";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=21000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(600);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(33): $recv");
		} else {
			print("screenOrderStart3 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.screenOrderStart4(ipNode) {
	code="ScreenOrder_4";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=26000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(800);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(34): $recv");
		} else {
			print("screenOrderStart4 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.screenOrderStart5(ipNode) {
	code="ScreenOrder_5";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=31000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1000);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(35): $recv");
		} else {
			print("screenOrderStart5 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.screenOrderStart6(ipNode) {
	code="ScreenOrder_6";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(36): $recv");
		} else {
			print("screenOrderStart6 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.onClose() {
	Cf.exit();
}
KioskHiTec:adminSetup.setLogEditor() {
	not( watcharPage ) watcharPage=Cf[KioskWatcher];
	watcharPage.inject(cf);
	cf[logEditor]=this.log;
}
KioskHiTec:OrderTool.onEvent() {
	node=@node;
	switch( @type ) {
	case 1:		x.qtMonRecvData( node[recvData], node );
	case 11:	x.easyCardReadData( node[data], node );
	case 12:	x.easyCardError( node[data], node );
	case 21:	x.hitecResponseOk( node[data], node );
	case 22:	x.hitecResponseError( node[data], node );
	}
}
KioskHiTec:main.onActivationChange() {
}
KioskHiTec:main.onMove() {
}
KioskHiTec:main.screenOrderStart7(ipNode) {
	code="ScreenOrder_7";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(37): $recv");
		} else {
			print("screenOrderStart7 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.onInit() {
	include("common.CanvasBase");
	include("KioskHiTec/widget.AdminMenuCanvas");
	x=newClass("KioskHiTec/widget.AdminMenuCanvas", this);
	
	db=Class.db('kiosk_hitec');
	cf={tag: confing };
	_log=func(s) {
		e=cf[logEditor];
		logType=null;
		if( s.start('##') ) {
			logType='E';
		} else if( s.start('#') ) {
			logType='I';
		}
		if( e && logType ) {
			e.append(s);
		} else {
			print("# $s");
		}
	};
	Cf[KioskWatcher]=this;
	
	tray = this.tray();
	tray.icon('vicon.bricks_defalut');
	tray.action([
		{id: tray.adminView,		text:관리자 화면열기,	icon: vicon.user_suit },
		{id: tray.setupView,		text:설정화면,				icon: vicon.application_form },
		{id: tray.queryView,		text:DB 쿼리툴,			icon: vicon.application_form_add },
		{id: tray.protocalView,	text:전문 테스트,			icon: vicon.application_form_magnify },
		{id: tray.pingTest,			text:네트워크 테스트,	icon: vicon.application_key },
		{id: tray.close,				text:종료,						icon: vicon.cancel_default }
	]);
	tray.contextMenu([tray.adminView,-,tray.logView,-,tray.queryView,tray.protocalView,tray.pingTest,-,tray.close]);
	tray.show();
	tray.onActivated=callback() {
		type=tray.reason;
		if( type.eq('click') ) pageLoad('KioskHiTec.adminSetup') .open();
	};
	
	this.timer(5000, callback() {
		watcherBatch(this, db);
	});
	this.initPage();
	this.qtMonStart();
}
:.onActivationChange() {
}
:.onEvent() {
	node=@node;
	switch( @type ) {
	case 1:
		this.openLogView();
	case 2:
		this.openThisPage();
	case 51:
			this.responseXML(node[data].ref(), node );
		this.updateStart();
	case 10:	x.qtMonRecvData( node[data], node );
	case 11:
		x.easyCardReadData(  node[data], node );
		cf[easyCardTick]=0;
	case 12:
		x.easyCardError( node[data], node );
		cf[easyCardTick]=0;
	default:
	}
}
:.onMove() {
}
:.onClose() {
	this.hide();
	return 'ignore';
}
:.screenOrderStart1(ipNode) {
	code="ScreenOrder_1";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=11000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(31): $recv");
		} else {
			print("screenOrderStart1 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart2(ipNode) {
	code="ScreenOrder_2";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=16000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(400);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(32): $recv");
		} else {
			print("screenOrderStart2 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart3(ipNode) {
	code="ScreenOrder_3";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=21000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(600);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(33): $recv");
		} else {
			print("screenOrderStart3 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart4(ipNode) {
	code="ScreenOrder_4";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=26000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(800);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(34): $recv");
		} else {
			print("screenOrderStart4 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart5(ipNode) {
	code="ScreenOrder_5";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=31000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1000);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(25000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(35): $recv");
		} else {
			print("screenOrderStart5 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart6(ipNode) {
	code="ScreenOrder_6";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(36): $recv");
		} else {
			print("screenOrderStart6 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart7(ipNode) {
	code="ScreenOrder_7";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(36): $recv");
		} else {
			print("screenOrderStart7 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart8(ipNode) {
	code="ScreenOrder_8";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(36): $recv");
		} else {
			print("screenOrderStart8 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.screenOrderStart9(ipNode) {
	code="ScreenOrder_9";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(36): $recv");
		} else {
			print("screenOrderStart9 read timeout or exception");
			System.sleep(2000);
		}
	});
}
:.qtMonStart() {
	cf[qt_mon_ip]=conf('setup#kiosk.qtMonHost');
	cf[qt_mon_port]=conf('setup#kiosk.qtMonPort');
	node=_node('QtMonNode');
	socket=node[socket];
	not( socket ) {
		socket=Class.socket("QtMon");
		node[socket]=socket;
		node[socketStartTick]=System.tick();
	}
		
	worker = Class.worker("QtMon");
	me=this;
	worker.start( func() {
		dist = System.tick() - node[socketStartTick];
		if( dist < 15000 ) {
			return;
		}
		not( socket.isConnect() ) {
			not( socket.connect( cf.qt_mon_ip, cf.qt_mon_port, 30000) ) {
				socket.close();
				_log("## 디바이스 연결오류, 호스트:$cf[qtMonHost], 포트: $cf[qtMonPort]", true);
				System.sleep(2000);
				return;
			}
		}
	
		recv= socket.readBuffer();
		if( recv ) {
			node[data]=recv;
			me.postEvent(10, node);
		}
	});
}
:.easyCardSend(req, callback) {
	cf[easyCardTick]=System.tick();
	web=Class.web('EasyCard');
		me=this;
	web.call( req, callback(type, data) {
		switch(type) {
		case read:
			_log("easyCardRead : $data");
			req[data] = data;
			me.postEvent(11, req);
		case finish:
			req.status=null;
		case error:
			_log("## easyCardError : $data");
			req[data] = data;
			me.postEvent(12, req);
			}
	});
}
:.hitecSend(req, xml) {
	web=Class.web('HiTec');
		web[data]=xml;
		me=this;
	web.call( req, callback(type, data) {
		switch(type) {
		case read:
			req[data] = data;
			me.postEvent(21, req);
		case finish:
			req.status=null;
		case error:
			req[data] = data;
			me.postEvent(22, req);
			}
	});
}
KioskHiTec:main.screenOrderStart8(ipNode) {
	code="ScreenOrder_8";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(38): $recv");
		} else {
			print("screenOrderStart8 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:main.screenOrderStart9(ipNode) {
	code="ScreenOrder_9";
	socket=Class.socket(code), worker=Class.worker(code);
	
	ipNode[screen_socket]=socket;
	ipNode[screen_code]=code;
	ip=ipNode[screen_ip];
	interval=ipNode[screen_interval];
	not( interval ) interval=35000;
	
	me=this;
	x.inject( cf );
	worker.start( callback() {
		dist=System.tick() - cf[pageStartTick];
		if( dist < interval ) {
			return;
		}
		not( socket.isConnect() ) {
			if( socket[connected] ) {
				System.sleep(1200);
				idx=cf[socket_idx++];
				@socket=Class.socket("socket_$idx");
				ipNode[screen_socket]=socket;
				print("소켓 새로 생성($idx) : $ipNode");
			}
			not( socket.connect(ip, 2018, 30000 ) ) {
				socket.close();
				print("# 주문스크린 연결실패 ($ipNode[clplu_nm]): $ip");
				System.sleep(15000);
				return;
			}
			socket[connected]=true;
			print("주문 스크린 연결: $ip");
		}
		recv= socket.readBuffer();
		if( recv ) {
			ipNode[data]=recv;
			print("주문 스크린 응답(39): $recv");
		} else {
			print("screenOrderStart9 read timeout or exception");
			System.sleep(2000);
		}
	});
}
KioskHiTec:AdminMenu.onActivationChange() {
}
KioskHiTec:AdminMenu.onMove() {
}
:.onAction() {
	switch(@id) {
	case 'tray.setupView':
		pageLoad('KioskHiTec.adminSetup') .open();
	case 'tray.adminView':
		this.openThisPage();
	case 'tray.queryView':
		pageLoad('KioskHiTec.DbQuery') .open();
	case 'tray.protocalView':
		pageLoad('KioskHiTec.protocalTest', true) .open();
	case 'tray.pingTest':
		pageLoad('KioskHiTec.PingTest', true) .open();
		case 'tray.close':
		tray.hide();
		Cf.exit();
	default:
		print();
	}
}
:.downloadAdImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	downloadNext=callback() {
		fileCheck=Class.file('down');
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			not( menu[set_cd].eq('001','002','003') ) continue;
			_log("# 광고 이미지 다운 : $menu");
			file=menu[set_val];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			if( fileCheck.isFile("$path/$file") ) {
				print("이미 다운로드된 파일: $path/$file");
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					print("download ad ok =>$path/$file");
					downloadNext();
				case error:
					_log("## $url 광고 파일:$file 다운로드 오류: $data");
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
:.downloadCornerImage(url, root, path, reload) {
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="data/images/menus/corner";
	downloadNext=callback() {
		fileCheck=Class.file('down');
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[img_file_nm];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			_log("# 코너다운 : $menu");
			if( fileCheck.isFile("$path/$file") ) {
				print("이미 다운로드된 파일: $path/$file");
				menu[down_yn]='Y';
				continue;
			}
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					print("download corner ok =>$path/$file");
					downloadNext();
				case error:
					_log("## $url 코너 파일:$file 다운로드 오류: $data");
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
:.downloadMenuImage(url, root, path, reload) {
	print("# 다운로드 메뉴 이미지 : url=$url, root=$root, path=$path, reload=$reload");
	
	web = Class.web('KioskDownload');
	if( reload ) {
		while( menu, root ) {
			menu[down_yn]='N';
		}
	}
	not( path ) path="data/images/menus";
	downloadNext=callback() {
		fileCheck=Class.file('down');
		while( menu, root ) {
			if( menu[down_yn].eq('Y') ) continue;
			file=menu[goods_img];
			not( file ) {
				menu[down_yn]='Y';
				continue;
			}
			/* 파일 존재여부 체크 */
			if( fileCheck.isFile("$path/$file") ) {
				print("이미 다운로드된 파일: $path/$file");
				menu[down_yn]='Y';
				continue;
			}
			_log("# 메뉴다운 : $file");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					menu[down_yn]='Y';
					print("download menu ok =>$path/$file");
					downloadNext();
				case error:
					_log("## $url 메뉴 파일:$file 다운로드 오류: $data");
					menu[error]=data;
				}
			});
			break;
		}
	};
	downloadNext();
}
:.downloadResource(url, path, root, field) {
	web = Class.web('KioskDownload');
	not( url ) {
		url="http://61.78.39.132/DRIM/data.dir/prod_img/E002";
	}
	not( path ) {
		path="data/images/menus";
	}
	not( field ) {
		field='GOODS_IMG';
	}
	downloadNext=callback() {
		while( cur, root ) {
			if( cur[down_yn].eq('Y') ) continue;
			file=cur[$field];
			print("downloadResource => $url/$file");
			web.download("$url/$file", "$path/$file", callback(type, recv, total) {
				switch( type ) {
				case finish:
					cur[down_yn]='Y';
					print("download menu ok =>$path/$file");
					downloadNext();
				case error:
					cur[error]=data;
					print("download error=> $cur[error]");
				}
			});
			break;
		}
	};
	downloadNext();
}
:.initPage() {
	cf[easyCardTick]=0;
	cf[startUpdateTick]=0;
	cf[WebIndex]=0;
	
	db.value("select send_yn from tb_sale_header limit 1 offset 0");
	if( db.error() ) {
		_log("# 매출전송여부 필드 추가");
		db.exec("ALTER TABLE tb_sale_header ADD send_yn char(1) default 'Y' ");
	}
	
	if( db.count("select count(1) from kiosk_setup where use_yn='Y'") ) {
		db.fetch("
		   SELECT A.ms_no, A.pos_no, A.service_start_time, A.service_end_time, A.refresh_time, A.order_start_no, A.order_end_no,
		 				A.qt_mon_ip, A.qt_mon_port, A.emp_id, A.emp_pw, A.kiosk_id, A.kiosk_pw,
		 				B.van_cd, B.ms_cat_id
	 			  FROM kiosk_setup A
	 			   LEFT JOIN hitec_m60s B ON A.ms_no = B.ms_no AND A.pos_no = B.pos_no AND B.use_yn = 'Y'
			    WHERE A.use_yn='Y'
				  LIMIT 1",  cf);
		return;
	}
	cf[qt_mon_ip]=conf('setup#kiosk.qtMonHost');
	cf[qt_mon_port]=conf('setup#kiosk.qtMonPort');
	cf[easyCardUrl]	= conf('setup#kiosk.easyCardUrl');
		
	db.exec( conf('sql#hitec.addKioskSetup'), cf);
}
:.openLogView() {
	p=this[logView];
	not( p ) {
		p=pageLoad('KioskHiTec.KioskLogViewer', true);
		this[logView]=p;
	}
	p.flags('top,splash');
	p.move(0,0);
	p.size(1000,220);
	p.open();
}
:.openThisPage() {
	p=this;
	p.flags('top,splash');
	screenRect=System.info('screenRect', 0);
	p.move(screenRect.lt() );
	screenRect.size().inject(w,h);
	p.open();
}
:.updateStart(start, all) {
	if( cf[easyCardTick] ) {
		dist=System.tick() - cf[easyCardTick];
		if( dist<30000 ) {
			print("###### 결제취소중 업데이트 무시 ###### 설정정보 : $cf");
			return;
		}
		cf[easyCardTick]=0;
	}
	
	root=cf[updateNode];
	if( all ) {
		print("###### 전체다시 다운로드 ######");
		root.delete();
		root=null;
	}
	not( root ) {
		root={};
		if( all ) {
			types='X10S, M06S, M03S, M10S, M40S, M21S, M23S, M60S';
		} else {
			types='X10S, M06S, M03S, M10S, M40S';
		}
		db.fetch(conf("sql#hitec.selectKioskSetup"),  cf);
		while( tag, types.split() ) {
			cur=root.addNode();
			cur[tag]=tag;
			_log("# 업데이트 태그정보 : $cur[tag]");
		}
		cf[updateNode]=root;
	}
	if( start ) {
		print("# 업데이트 시작 [start=$cf] ");
		if( cf[startUpdateTick] ) {
			while( cur, root ) {
				print("업데이트 노드 => $cur[finish]");
			}
			db.exec("insert into kiosk_error ( error_type, error_kind, error_nm, error_data, error_status, tm) values( 'update', 'timeout', '오류', '상품정보 업데이트 오류', 'R', 0 )" );
			tray.hide();
			Cf.exit();
			print("############## 프로그램 종료 오류 ################ ");
			System.kill('KioskWatcher.exe');
			return;
		}
		cf[startUpdateTick]=System.tick();
		cf[UpdateCount]=0;
		cf[UpdateReturnCount]=0;
		while( cur, root ) {
			cur.removeAll();
			cur[finish]=false;
		}
	}
	me=this;
	webIndex=cf[WebIndex];
	web=Class.web("update_$webIndex");
	while( cur, root ) {
		if( cur[finish] ) {
			continue;
		}
		url=this.getUrl(cur[tag], all );
		not( url ) {
			cur[finish]=true;
			continue;
		}
		web.call(url, callback(type, data) {
			switch( type ) {
			case read:
				cur[finish]=true;
				cur[data]=data;
				me.postEvent(51, cur);
			case finish:
				cur[finish]=true;
			case error:
				_log("## $url 호출 오류 \n$data");
			break;
			}
		});
		return;
	}
	dist=System.tick() - cf[startUpdateTick];
	cf[startUpdateTick]=0;
	_log("# makeKioskData($webIndex) : 전체 변경건수 $cf[UpdateCount] 건, 처리시간: ${dist} ms");
	cf[logEditor]=null;
}
:.wasStart() {
	was=Class.was('Kiosk');
	was.start(8089,"data/webpages");
}
:.getUrl(reqType, all) {
	node=getCommCodeNode('kiosk#reqType');
	cur=node.findOne('code', reqType);
	not( cur ) {
		_log("# 호출 url 를 찾을수 없습니다 URL를 확인하세요 (타입:$reqType)");
		return;
	}
	cf.inject( ms_no, pos_no,emp_id, emp_pw );
	last_seq='0000';
	not( all ) {
		not( reqType.eq('X10S') ) {
			last_seq=db.value("SELECT  max(log_seq) as log_seq FROM HITEC_${reqType}");
			_log("# $reqType => [최종순번:$last_seq]");
			if( cf[logEditor] ) {
				not( last_seq ) last_seq='0000';
			}
			not( last_seq ) return null;
		}
	}
	url= fmt(cur[data]);
	if( cf[logEditor] ) {
		_log("# $reqType => [URL:$url] ");
	}
	return url;
}
:.makeKioskData(tag, fields, root) {
	if( fields ) {
		_log("makeKioskData ================================> ($tag, $fields, $root)");
	}
	table="HITEC_${tag}";
	a='', b='';
	while( k, fields, n, 0 ) {
		if( n ) {
			a.add(',');
		} else {
			b.add(k);
		}
		a.add(k);
	}
	a.add(",tm");
	switch( tag ) {
	case M03S: 	b='CLPLU_CD';
	case M10S:		b='GOODS_CD';
	case M12S:	 	b='GOODS_CD';
	case M05S:		b='CLPLU_CD';
	case M06S:		b='SET_CD';
	case M40S:		b='MS_NO';
	case M15S:		b='EMP_NO';
	case M21S:		b='GOODS_CD';
	case M60S:		b='VAN_CD';
	}
	cnt=root.childCount();
	not( cnt ) {
		return;
	}
	cf[UpdateCount+=cnt];
	if( cf[checkAll] ) {
		db.exec("delete from $table");
	}
	a.add(', USE_YN');
	ins=getQuery(table, a);
	upd=getQuery(table, a, b);
	
	tm=0;
	while( cur, root ) {
		cur[tm]=tm;
		cur[USE_YN]='Y';
		if( tag.eq('M03S') ) {
			if( cur[CLPLU_NM_JP] ) {
				decode	= cur[CLPLU_NM_JP].decode('a2u');
				cur[CLPLU_NM_JP]=decode;
			}
			if( cur[CLPLU_NM_CH] ) {
				decode	= cur[CLPLU_NM_CH].decode('a2u');
				cur[CLPLU_NM_CH]=decode;
			}
		} else if( tag.eq("M10S", "M12S") ) {
			if( cur[JP_NM] ) {
				decode	= cur[JP_NM].decode('a2u');
				cur[JP_NM]=decode;
			}
			if( cur[CN_NM_GAN] ) {
				decode	= cur[CN_NM_GAN].decode('a2u');
				cur[CN_NM_GAN]=decode;
			}
		}
		if( cur[PROC_GB].eq('D') ) {
			db.exec("update $table set use_yn='N' where $b=#{$b}", cur);
		} else {
			if( cur[PROC_GB].eq('U') ) {
				db.exec(upd, cur);
			} else {
				/* M23S 전문의 경우 무조건 INSERT */
				if(tag.eq('M23S')) {
					db.exec(ins, cur);
				} else {
					not( db.exec(upd, cur) ) {
						db.exec(ins, cur);
					}
				}
			}
		}
	}
	root.removeAll();
	if( tag.eq('X10S') ) {
		return;
	}
	
	not( cf[prod_img_url] ) {
		db.fetch("select prod_img_url from hitec_x10s limit 1 offset 0", cf);
	}
	path=cf[imagePath];
	
	url=cf[prod_img_url], path=conf("setup#kiosk.imagePath");
	checkDown=true;
	
	_log("# 다운로드 시작: tag=$tag, 이미지 경로: $path URL: $url");
	switch( tag ) {
	case M10S:
		root=_node();
		db.fetchAll("select goods_cd, goods_img from hitec_m10s where goods_img<>'' and tm='0' ", root);
		this.downloadMenuImage(url, root, "$path/menus");
	case M06S:
		root=_node();
		db.fetchAll("select set_cd, set_val from hitec_m06s where set_val<>'' and tm='0' ", root);
		this.downloadAdImage(url, root, "$path/menus/kiosk");
	case M03S:
		root=_node();
		db.fetchAll("select clplu_cd, clplu_nm, img_file_nm from hitec_m03s where use_yn='Y' and tm='0' ", root);
		this.downloadCornerImage(url, root, "$path/menus/corner");
	default:
		checkDown=false;
	}
	if( checkDown ) {
		while( cur, root ) {
			not( cur[error] ) continue;
			_log("## 다운로드 에러(태그: $tag) => $cur[error]");
			cur[tag]=tag;
			cur[type]='E';
			cur[info]="node=$cur";
			db.exec( conf('sql#watcher.addDownLoadInfo'), cur);
		}
	}
	tm=System.localtime();
	db.exec("update $table set tm='${tm}' where tm='0' ");
}
:.responseXML(data, root) {
	reqType=root[tag];
	parseProp=func(node, tag, &prop) {
		node[tag]=tag;
		idx=node.index();
		arr=null;
		not( idx ) arr = _arr(node,'fieldsArray');
		while( prop.valid() ) {
			k=prop.findPos('=').trim();
			not( k ) break;
			if( arr ) arr.add(k);
			ch=prop.ch();
			if( ch.eq() ) {
				node[$k]=prop.match().trim();
			} else if( ch.eq('[') ) {
				in=prop.match();
				arr=[];
				while( in.valid() ) {
					arr.add( in.findPos(',').trim() );
				}
				node[$k]=arr;
			} else {
				node[$k]=prop.findPos(" \t\n",4).trim();
			}
		}
	};
	parseXml=func(&data, node, fiistNode) {
		not( node ) {
			node=xmlNode;
			node.removeAll();
		}
		while( data.valid() ) {
			ch=data.ch();
			not( ch.eq('<') ) {
				break;
			}
			if( data.ch(1).eq('!') ) {
				data.match('<!--','-->');
				continue;
			}
			if( data.ch(1).eq('?') ) {
				data.match('<?','?>');
				continue;
			}
			sp=data.cur();
			tag=data.incr().move();
			sub = node.addNode();
			not( fiistNode ) {
				fiistNode=sub;
			}
			if( data.ch().eq('-') ) {
				sub[kind]=data.incr().move();
			}
			if( tag.eq('br', 'space', 'image') ) {
				prop=data.findPos(">");
				this.parseProp( sub, tag, prop);
			} else {
				in=data.find('>');
				if( in.ch(-1).eq('/') ) {
					prop=data.findPos('/>');
					parseProp( sub, tag, prop);
				} else {
					data.pos(sp);
					if( sub[kind] ) {
						in=data.match("<$tag-${sub[kind]}","</$tag-${sub[kind]}>");
					} else {
						in=data.match("<$tag","</$tag>",8);
					}
					not( in ) {
						in=data.findPos("</$tag>");
					}
					prop=in.findPos(">");
					parseProp( sub, tag, prop);
					if( tag.eq('html', 'text') ) {
						val=in.trim();
						if( val ) sub[data]=val;
					} else {
						if( in.ch().eq('<') ) {
							parseXml(in, sub, fiistNode);
						} else {
							val=in.trim();
							if( val ) sub[data]=val;
						}
					}
				}
			}
		}
		return fiistNode;
	};
	
	parseXml( data, root.removeAll() );
	cur=findTag('HEADER', root);
	not( cur ) {
		_log("# xml 헤더를 찾을수 없습니다. $root");
	}
	sub=cur.child(0);
	not( sub ) {
		return;
	}
	if( sub[tag].eq('DETAIL') ) {
		data=sub.child(0);
		not( data ) {
			return;
		}
		this.makeKioskData(reqType, data[fieldsArray], sub);
	} else {
		_log("DETAIL NOT FOUND !!!");
	}
}
