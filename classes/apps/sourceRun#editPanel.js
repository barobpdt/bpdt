initPage() {
	@editor=widget('editor')
	@editorTitle=widget('editorTitle')
	@editorInfo=widget('editorInfo')
	@searchInput=widget('searchInput')
	
	addModule(editor,'editor:editorSource')
	setEvent(searchInput,'onEnter',this.searchText, this)
	this.setEditorSyntax() 
	this.setKeyMap()
	editorTitle.value("소스테스트 실행창")
}
editorFocus() {
	this.timer(250, func() { editor.focus() })
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
	val='';
	editor.focus();
	pos=editor.pos();
	if(key.eq(KEY.B)) {
		editor.searchPrev('~~');
		sp=editor.pos();
		if( sp.lt(pos)) {
			editor.pos(sp+2);
		} else {
			editor.pos(pos);
		}
		editor.searchNext('~~');
		ep=editor.pos();
		if(sp.eq(pos)) {
			val=editor.value();
		} else if(sp.lt(pos)) {
			if(ep.lt(pos)) {
				ep=editor.pos("end");
			} else {
				ep-=2;
			}
			if(pos.lt(ep)) {
				val=editor.text(sp,ep);
			} else {
				return print("run postion error", sp, ep, pos);
			}
		} else if(sp.eq(ep)) {
			if(sp.gt(pos)) {
				sp=0;
				ep-=2;
			} else {
				ep=editor.pos("end");            	
			}
			if(sp.lt(ep)) val=editor.text(sp,ep);
		} else if(pos.lt(ep) && ep.lt(sp) ) {
			sp=0, ep-=2;
			val=editor.text(sp,ep);
		}
		editor.pos(pos);
		not(val) return print("run postion error", sp, ep, pos);
	} else { 
		if(sel) {
			val=editor.text("select");
		} else {
			val=editor.value();
		}
	}
	val.ref();
	if(val.find("~~",1) ) {
		src=val.findLast("~~",1).right();
		src.findPos("\n");
		print("src==$src");
	} else {
		src=val;
	} 
	not(src) this.alert("실행할 소스가 없습니다");
	Cf.error(true);
	if( src.ch('<') ) {
		Cf.sourceApply(src);  
	} else {
		Cf.sourceCall(src, true); 
	}
	err=Cf.error();
	if(err) {
		logWriter("sourceRun").appendLog("실행오류:\n${err}");
	}
}
keyTemplateInput() {
	dialog=dialog('keyTemplateInput') not(dialog) return this.alert("키템플릿설정 대화상자 오류");
	dialog.open(this,"center")
	dialog.formValue.inject(key, note)
	not(key) return insertBindText(editor);
}
keyMapInput() {
	dialog=dialog('keyMapInput');
	dialog.open(this,"center")
}
searchFocus() {
	page().active()
	this.timer(50, func() {
		searchInput.focus()
	})
}
setEditorSyntax() {
	node=Cf.getObject("syntax","default");
	not(node) {
		node=Cf.getObject("syntax","default",true);
		node.parseJson(conf("editor.syntax"));
	}
	editor.syntax(node);
}
setKeyMap() {
	map=object("map.editorKey");
	node=map.addNode('default');
	not(node.childCount()) {
		db=Baro.db("config");
		db.fetchAll("select key, data from keymap_info where targetId='default'", node);
		while(sub, node) {
			sub.inject(key,data);
			node.set(key,data);
		}
	}
	editor.var(keyMap, node);
}
searchText() { 
	str=searchInput.value();
	if(str) global("prevSearchValue",str);
	this.editorFocus()
	editor.editorSearch(0);	
}
keymapInfo() {
	
}