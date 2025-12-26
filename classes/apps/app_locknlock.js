/* API 관리 */
class main {
	initClass() {		
		@content=this.getWidget('content') 
		@topMenu=this.getWidget('topMenu', true)		
		this.timer(500)
	}
	initPage() {
		this.positionLoad()
		mainMenu=this.conf('mainMenu', true)
		this.action(mainMenu.actionList )
		topMenu.setMenuInfo(this, mainMenu)
		topMenu.changeMenu('menu.dbTableMng')
	}
	onTimer() {
		if( this.firstCall ) {
			this.initPage();
			this.firstCall=false
		}
	}
	onClose() {
		this.positionSave()
	}
	showMenu(menuCode, menuData, pos, target) {
		menus=topMenu.makeMenu(menuCode, menuData)
		if( typeof(menus,'node')) {
			target.menu(this, menus, pos)
			this.currentMenuTarget=target
		}
	}
	onAction(action) {
		if(action.cmp('type','page') ) {
			topMenu.changeMenu(action.id)
		} else {
			target=this.currentMenuTarget
			if(target) {
				target.execAction(action)
			}
		} 
	}
	changeAction(action, tab) {
		if(this.currentTab==tab) return;
		this.currentTab=tab
		action.inject(id, type, sub)
		name=right(id)
		if( type.eq('page') ) {
			page=this.findWidget(name)
			if(page) {
				content.current(page)
			} else {
				page=this.makePage(name,"margin:0,spacing:0", sub, true)
				content.addPage(page, true)
				page.initPage()	
			}
		} else {
			alert("메뉴 액션 $type 이 정의되지 않았습니다")
		}		
	}
} 

/* DB테이블 조회 페이지 */
class dbTableMng {
	initClass() {
		@db=Baro.db('locknlock')
		@popup=this.makeWidget('popupTableInfoForm')
		@splitter=this.getWidget('splitter')
		@left=this.makeWidget()
		@page=this.makePage('dbTableCenterPage',"margin:0,spacing:0", "splitter")
		@input=this.makeWidget('input','inputDbTableFilter') 
		@inputTable=this.makeWidget('input','inputDbTableName') 
		@tree=this.makeWidget('tree','treeDbTable') class(tree,'tree')
		@grid=this.makeWidget('grid','gridDbTable')	class(grid,'grid')

		@editor=this.makeWidget('editor','editorDbTable') class(editor, 'editorSql')
		@form=this.makeWidget()		

		tree.var(treeMode, false)
		tree.setEvent('onDraw', this, this.treeDraw)
		tree.setEvent('onMouseDown', this, this.treeMouseDown)
		tree.setEvent('onMouseMove', this, this.treeMouseMove)
		tree.setEvent('onFilter', this, this.treeFilter)
		tree.model('name')
		grid.model('text')
		grid.setEvent('onDraw',this, this.gridDraw)
		grid.setEvent('onDoubleClick',this, this.gridDoubleClick)
		grid.setEvent('onMouseDown',this, this.gridMouseDown)
		grid.setEvent('onMouseWheel', this, this.gridMouseWheel)
		grid.setEvent('onHeaderClick', this, this.gridHeaderClick)		
		grid.var(bgColor, color('#4466aacc'))
		input.setEvent('onTextChange', this, this.inputFilterChange)
		gridInput=grid.setInput()
		gridInput.setEvent("onKeyDown", this, this.gridInputKeyDown, true)

		fn=editor.set('keydownCallback', call(this.editorKeyDown))
		fn.set('sender',editor)
	}
	initPage() {
		not( this.firstCall) return;
		left.setFormInfo('dbTableLeftFormInfo',this)
		form.setFormInfo('dbTableContentFormInfo',this)
		splitter.orientation('hbox')
		splitter.addPage(left)
		splitter.addPage(page)
		this.timer(600)
	}
	onTimer() {
		if( this.firstCall) {
			this.initFirst()
			this.firstCall=false;
		}
		if( this.var(filterChangeTick)) {
			dist=System.tick() - this.var(filterChangeTick);
			if(dist>500) { 
				this.filterValue=input.value() 
				this.var(filterChangeTick, 0)
				tree.update() 
			}
		}
		if( this.var(strEditQuery) ) {
			this.setQuery(this.var(strEditQuery))
			this.var(strEditQuery,null)
		}
	}
	initFirst() {
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'3,7'))
		splitter.stretchFactor(1)
		this.setContent() 
		this.treeData()
		this.treeQueryInfo()		
	}
	setContent() {
		splitter=page.getWidget('splitter')
		splitter.orientation('vbox')
		splitter.addPage(editor)
		splitter.addPage(form)
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'*,38'))
	}
	drawDetail(dc,rc) {
		node=this.currentLogNode not(node) return;
		dc.save().font('size:11, weight:bold')
		dc.pen('#5566aacc').text(node.rect.incr(10,2), node.logText)
		dc.restore()
	}
	gridDraw(dc, node, index, state) {
		field=grid.field(index)
		rc=grid.drawState(dc, node, state, index, field );
		text=node.get(field) 
		if(  node.flag(NODE.new) ) {
			@draw.modifyPath(dc,rc,'#00f')
		} else if( nodeEq(node,"@modifyFields",field) ) {
			@draw.modifyPath(dc,rc)
		}
		dc.text(rc,text)
	}
	gridDoubleClick(pos, mode) {
		hh=grid.headerHeight()
		node=grid.at(pos.incrY(hh)) 
		not(node) return false;
		field=node.var(code)
		grid.edit(node, field)
	} 
	gridInputKeyDown(k,a) {
		if( k.eq(KEY.Return, KEY.Enter) ) {
			node=grid.var(editNode)
			field=grid.var(currentEditField) 
			arr=node.addArray("@modifyFields") not(arr.find(field)) arr.add(field)
			node.set(field, grid.inputValue())
			node.flag(NODE.modify, true) 
			grid.update()
			grid.inputHide()
		}
	}
	gridMouseWheel(delta) {
		grid.inputHide()
	}
	gridMouseDown(pos) {
		grid.inputHide()
	}
	gridHeaderClick() { 
		idx=grid.var(clickIndex)
		grid.sort(idx, 'asc')
	}	
	setQuery(&query, skip) {
		not(query.ch()) {
			return this.alert("실행할 쿼리가 없습니다")
		}
		isExecType = func(&s) {
			c=s.ch()
			while(c.eq('-','/')) {
				if(c.eq('-')) s.findPos("\n") else s.match(1);
				c=s.ch()
			}
			not(c) return;
			type=s.move().lower()
			if(type.eq('update','insert', 'delete', 'alter', 'exec')) return type;
			return;
		};
		isCommand = func(&s) {
			not(s.ch()) return;
			not(s.start('##', true)) return;
			line=s.findPos("\n")
			ty=line.move().lower()
			if( ty.eq('apply') ) {
				this.click_ApplyModify(ty, line)
				return true;
			}
			if( ty.eq('source') ) {
				dist=System.localtime() - conf('localtime.soureMove')
				moveSqlModify(dist)
				moveAuxModify(dist)
				moveJavaModify(dist)
				conf('localtime.soureMove', System.localtime(), true)
				return true;
			}
			if( ty.eq('popup') ) {
				list=_node('formInfo.configEdit').var(targetList)
				if(list ) {
					while(form, list) {
						not(form.is('visible')) continue;
						form.active()
					}
				}
				return true;
			}
			return;
		};
		if( isCommand(query) ) {
			return;
		}
		sql=parseQuery(query)
		if(sql) { 
			return editor.append("\n$sql\n;", true)
		}
		execType=isExecType(query)
		if(execType) {
			rows=0;
			while( query.valid()) {
				sql=query.findPos(";",1);
				not(sql.ch()) break;
				db.exec(sql)
				if(db.error()) {
					return this.alert("DB $execType 처리중 오류가 발생했습니다\n${db.error()}")
				}
				rows+=db.var(affectRows);
			} 
			this.alert("$execType $rows 건 처리했습니다")
			return;
		}
		
		splitter=page.getWidget('splitter')
		if(skip) {
			if( query.find(';',1)) query=query.findPos(';',1)
		} else {
			editor.append("\n$query\n;", true)
		} 
		hh=splitter.sizes().get(1)
		if(hh<100) {
			tot=splitter.sizes().sum()
			splitter.sizes(recalc(tot,'*,300'))
		} 		
		root=grid.rootNode().removeAll()
		db.fetchAll(query, root, true) 
		if(db.error()) {
			return this.alert("DB 처리중 오류가 발생했습니다\n${db.error()}")
		}
		grid.is('sortEnable', false)
		grid.fields(root.var(fields))
		System.sleep(10)
		grid.update(true)
		if( form.isCheck('CheckSameSize') ) {
			grid.fullWidth(true)
		} else {
			grid.fullWidth('resizeToContent')
		}
		form.update() 
	}
	editorGetQuery() {
		if( editor.is('select')) {
			query=editor.text('select')
			if( query.size()>5 ) return query;
		}
		line=editor.text('lineStart')
		if( line.start('##')) {
			return editor.text('lineStart', 'lineEnd');
		}
		editor.getQueryBlock().inject(sp,ep)
		if(sp<ep) {
			if( form.isCheck('CheckRunSelect')) {
				editor.select(sp,ep)
			}
			return editor.text(sp,ep);
		}
		return;
	}
	runQuery() {
		query=this.editorGetQuery()
		not(query) this.alert("실행할 쿼리가 없습니다")
		this.setQuery(query, true)
	} 
	onKeyDown(k,a) {
		ctrl=a&KEY.ctrl;
		if(k.eq(81) && ctrl ) {
			input.focus()
			return true;
		} 
		return this.editorKeyDown(k,a);
	} 
	editorKeyDown(k,a) {
		ctrl=a&KEY.ctrl;
		if( ctrl && k.eq(KEY.P) ) {
			text=editor.text("select")
			if(text) {
				s=text.ref()
				ss=''
				while(s.valid(), n) {
					if(n) ss.add(",\n")
					left=s.findPos(",").trim()
					not(left) break;
					ss.add(left)
				}
				System.copyText(ss)
				return true;
			}
		}
		if( ctrl && k.eq(KEY.Return, KEY.Enter) ) {
			this.runQuery()
			return true;
		}
	}
	inputFilterChange() {
		this.var(filterChangeTick, System.tick())
	}
	popupTableInfo(node) {
		rc=tree.nodeRect(node) not(typeof(rc,'rect')) return;
		rcPopup=rc(rc.rb(),900,450)
		rcGlobal=tree.mapGlobal(rcPopup)
		chk=eq(popup.parentWidget(), this)
		not(chk) {
			popup.parentWidget(this)
			popup.flags('tool', true)
		} 
		popup.move(rcGlobal)
		popup.open()
		popup.active()
		popup.setTable(node)
	}
	treeDraw(dc, node, index, state, over) { 
		rc=tree.drawSelect(dc, dc.rect(), node, col, state, over)
		node.rcIcon=rc.moveLeft(18,18,-2,0,true) 
		if( node.tag ) {
			dc.save().font('size:16px, weight:bold')
			dc.text(rc, node.text)
			dc.restore()
			return;
		}
		node.inject(name, type, comment) 
		dc.save()
		dc.font('size:15px').pen('#223').text(rc, name)
		if( type.eq('queryInfo')) {			
			dc.font('size:12px')
			st=''
			if( node.flag(NODE.new) ) {
				st='신규'
			} else if( node.flag(NODE.modify) ) {
				st='수정'				
			} else if( node.flag(NODE.delete) ) {
				st='삭제'
			}
			if(st) {
				rcStat=rc.rightCenter(50,22) 
				dc.pen('#55e').text(rcStat,st,'center')
			} else {
				dc.textSize(node.cd).inject(tw)
				tw+=15;
				rcStat=rc.rightCenter(tw,22)
				dc.pen('#aaa').text(rcStat,node.cd,'center')
			}
		} else {
			dc.textSize(name).inject(tw, th)
			dist=rc.width() - tw;
			if(dist.gt(60)) {
				tw+=4;
				rcIcon=rc.incrX(tw).leftCenter(20,20) 
				dc.rectLine(rcIcon,0,'#eee')
				dc.image(rcIcon.center(18,18), 'vicon:database_table')
				node.rcBtn=rcIcon
				if(comment) {
					tw+=20;
					rcComment=rc.incrXW(tw, 4) 
					dc.font(9).pen('#966').text(rcComment, "- $comment", "right")
				}
			}
		}
		dc.restore()
	}
	treeData() {
		root=tree.rootNode().removeAll()
		root.addNode().with(tag:tableInfo, text:락앤락 테이블정보)
		root.addNode().with(tag:queryInfo, text:쿼리저장 정보)
		node=findTag(root,'tableInfo')
		db.fetchAll(this.conf('sqlTableInfo'), node)
		while(cur, node) {
			cur.type='tableName'
			cur.comment=conf("tableComment.${cur.name}")
		}		
		tree.update()
		tree.expand(node,true, true)
	}
	treeQueryInfo() {
		root=tree.rootNode()
		queryInfo=findTag(root,'queryInfo').removeAll()
		queryInfo.grp='locknlockQueryInfo'
		db=Baro.db('config')
		db.fetchAll("select 'queryInfo' as type, grp, cd, note, data from conf_info where grp=#{grp} order by cd asc ", queryInfo)
		while(cur, queryInfo) {
			cur.name=cur.note
		}
		tree.update()		
	}
	treeMouseMove(pos) {
		node=tree.at(pos)
		if(node ) {
			node.inject(rcBtn, rcCopy)
			if( rcBtn && rcBtn.contains(pos) ) {
				this.rcOver=node.rcBtn
				this.cursor(CURSOR.PointingHandCursor)
				return
			}
		}
		if( this.rcOver) {
			this.rcOver=null
			this.cursor(0)
		}
	}
	treeMouseDown(pos,a,b) { 
		node=tree.at(pos)
		not(node) return;
		node.inject(rcIcon, rcBtn)
		if(rcIcon.contains(pos)) return;		
		if( this.rcOver ) {
			if( rcBtn.contains(pos) ) {
				inputTable.value(node.name)
				this.popupTableInfo(node)
				return 'ignore'
			}
		}
		if( node.cmp('type','queryInfo') ) {
			if(b.eq('right')) {
				this.openPromptEdit(node)
				return true;
			}
			inputTable.value('')
			editor.insertQuery(node.data)
			return;
		}
		if( node.cmp('type','tableName') ) {
			form=@form.get('inputComment', this)
			node.grp='tableComment'
			node.cd=node.name
			node.data=node.comment
			node.useData=true
			inputTable.value(node.name)
			form.openTool(this)
			form.setData("${node.cd} 테이블 상세 설명입력", node, this)
			return true;
		}
		if(this.currentTreeNode!=node) {
			this.currentTreeNode=node
			tree.current(node)
			tree.expand(node, true, true)
			page('main').var(currentTreeNode, node)
		}
		return 'ignore';
	}
	treeFilter(node) { 
		if(node.tag) return true;
		if(node.cmp('type','queryInfo')) return true;

		filter=this.filterValue not(filter) return true;
		if(node.get('name').find(filter,2)) return true;
		return false;
	} 
	click_RunQuery() {
		this.runQuery();
	}
	popupClosed(form) {
		if( form.var(formCode)=='inputComment') {
			node=form.var(refNode)
			node.inject(grp, cd)
			node.comment=conf("${grp}.${cd}")
			tree.update()
			return;
		}
		root=tree.rootNode()
		queryInfo=findTag(root,'queryInfo')
		node=form.var(refNode)
		not( node.flag(NODE.ok)) return;
		if( node.flag(NODE.new)) {
			cur=queryInfo.addNode()
			cur.copyNode(node,true)
			cur.flag(NODE.new,true)
			cur.type='queryInfo'
			cur.name=cur.note
		} else {
			node.inject(grp, cd, note)
			node.name = note
		}
		tree.update()
		not( tree.is('expand', queryInfo) ) {
			tree.expand(queryInfo,true,true)
		}
		tree.current(cur, true)
	}
	click_CopyGrid() {
		root=grid.rootNode()
		ss=''
		a=false, b=false;
		if( form.isCheck('CheckTab') ) a=true
		if( form.isCheck('CheckJsValue') ) b=true
		sep=when(a,"\t",",")
		fields=grid.fields()
		while(fn,fields,c) {
			field=fn.field 
			if(c) ss.add(sep)
			ss.add(when(b,Cf.jsValue(field),field) )
		}
		while(cur, root) {
			ss.add("\n")
			while(fn,fields,c) {
				field=fn.field
				v=cur.get(field)
				if(c) ss.add(sep)
				ss.add(when(b,Cf.jsValue(v),v) )
			}
		}
		System.copyText(ss)
	}
	click_AddRow() {
		root=grid.rootNode()
		last=root.child(-1)
		node=root.addNode()
		if(last ) {
			node.copyNode(last, true)
		}
		node.flag(NODE.new, true)
		grid.update()
		grid.current(node)
	}
	click_ApplyModify(type, &s) {
		execCheck=form.isCheck('CheckRunSelect')
		wa=_arr()
		table=inputTable.value() 
		fields=grid.fields()		
		if( typeof(s,'string')) {
			kind=s.move()
			if( type.eq('apply') && kind.eq('update') ) {
				table=s.move()
				while(s.valid()) {
					v=s.findPos(',').trim()
					wa.add(v)
				}
				execCheck=true
			}
		} else {
			wf=fields.get(0).get('field') 
			wa.add(wf)
		}
		idx=0
		if( execCheck ) {
			not(table) return this.alert("적용할 테이블명이 정의되지 않았습니다");			
			while(cur, grid.rootNode()) {
				if(cur.flag(NODE.new)) {
					fa='', fb=''
					while(fn, fields, c) {
						field=fn.field
						if(c) {
							fa.add(", ")
							fb.add(", ")
						}
						fa.add("$field")
						fb.add("#{$field}")
					}
					sql="insert into $table ($fa) values ($fb)";
					db.exec(sql, cur)
					idx++;
				} else if(cur.flag(NODE.modify)) {
					fa='', fb='';
					while(field, cur.var(modifyFields), c) {
						if(c) fa.add(", ")
						fa.add("$field=#{$field}")
					}
					while(field, wa) {
						fb.add(" and $field=#{$field}")
					}
					if(fb) {
						sql="update $table set $fa where 1=1 $fb";
						db.exec(sql, cur)
						if(db.error()) {
							return this.alert("DB적용 오류\n${db.error()}");
						}
						idx++;
					}
				}
			}
			if(idx) {
				this.alert("$idx 건 내용을 변경하였습니다")
			} else {
				this.alert("그리드 변경된 내용이 없습니다")
			}
		} else {
			ss=''
			while(cur, grid.rootNode()) {
				if(cur.flag(NODE.new)) {
					fa='', fb=''
					while(fn, fields, c) {
						field=fn.field
						if(c) {
							fa.add(", ")
							fb.add(", ")
						}
						fa.add("$field")
						fb.add(Cf.jsValue(cur.get(field),true) )
					}
					if(idx) ss.add(";\r\n")
					ss.add("insert into $table ($fa) values ($fb)");
					db.exec(sql, cur)
					idx++;
				} else if(cur.flag(NODE.modify)) {
					fa='', fb='';
					while(field, cur.var(modifyFields), c) {
						if(c) fa.add(", ")
						fa.add("$field=",Cf.jsValue(cur.get(field),true))
					}
					while(field, wa) {
						fb.add("and $field=",Cf.jsValue(cur.get(field),true))
					}
					if(fb) {
						if(idx) ss.add(";\r\n")
						ss.add("update $table set $fa where 1=1 $fb");
						idx++;
					}
				}
			}
			if(ss) {
				editor.append("\n$ss\n;", true)
			} else {
				this.alert("그리드 변경된 내용이 없습니다")
			}
		} 
	}
	click_CheckRunSelect(item) {
		if( item.checked ) {
			this.editorGetQuery()
		}
	}
	click_CheckSameSize(item) { 
		if( item.checked ) {
			grid.fullWidth(true)
		} else {
			grid.fullWidth('resizeToContent')
		}
		grid.update(true, true);
	}
	click_SaveQuery(item) { 
		query=this.editorGetQuery()
		not(query) return this.alert("저장할 쿼리가 없습니다")
		editor.var(currentQuery, query)
		this.openPromptEdit()
	}
	openPromptEdit(node) {
		db=Baro.db('config')
		not(node) {
			node=_node('data.temp').removeAll(true)
			db.fetch("select max(cd) as cd from conf_info where grp='locknlockQueryInfo' and cd like '0%'", node)
			num=node.cd.toInt() +1
			grp='locknlockQueryInfo'
			cd=lpad(num,3)
			data=editor.var(currentQuery)
			note="신규쿼리 저장"
			node.with(grp, cd, data)
			node.flag(NODE.new, true)
		}
		list=_node('formInfo.configEdit').var(targetList)
		if(list ) {
			while(form, list) {
				code=form.var(pageCode)
				not(code) {
					ref=form.var(refNode)
					if(ref) code=ref.cd					
				}
				if(code.eq(node.cd)) {
					form.open()
					form.active()
					return;
				}
			}
		}
		form=@form.getVisible('configEdit',this)
		form.var(pageCode, node.cd)
		form.setData("금형 관련쿼리 정보", node, this)
		form.open()
		form.active()
	}
}

/* 테이블 컬럼정보 팝업 */
class popupTableInfoForm {
	initClass() { 
		@grid=this.makeWidget('grid', 'gridTableInfo') class(grid, 'grid') 
		input=grid.setInput(true);
		input.setEvent('onKeyDown', this, this.keydown)
		grid.model(#[
			chk:선택						#40
			, COLUMN_NAME:컬럼명			#200
			, TYPE: 데이터타입			#100
			, COLUMN_DESCRIPTION: 설명	#200
			, IS_NULLABLE: NULL여부		#80
			, COLLATION_NAME: 키정보		#80
			, TABLE_DESCRIPTION:테이블정보	
		])
		grid.is('stretchLast', true)
		grid.is('sortEnable', true)
		grid.var(bgColor, color('#528B3DE0'))
		grid.setEvent('onDraw', this, this.gridDraw)
		grid.setEvent('onMouseDown', this, this.gridMouseDown)
		grid.setEvent('onMouseWheel', this, this.gridMouseWheel)
		grid.setEvent('onHeaderClick', this.gridHeaderClick )
		this.setFormInfo('popupTableInfoForm')
		this.timer(500)
	}
	onTimer() {
		if( grid.var(editStartTick)) {
			grid.var(editStartTick,0)
			grid.inputFocus()
		}		
	}	 
	keydown(k,a,b) {
		if(k.eq(KEY.Escape)) {
			return grid.inputHide();
		}
		if(k.eq(KEY.Enter, KEY.Return, KEY.Tab)  ) {
			node=grid.var(editNode)
			if(node.flag(NODE.add)) {
				field=grid.var(currentEditField)
				fields=grid.fields()
				cur=findField(fields, 'field', field) idx=cur.index() + 1;
				next=fields.child(idx)
				grid.inputHide(true)
				if(next) {
					grid.edit(node, next.field)
				} else {
					grid.inputHide()
				}
			} else {
				next=grid.nextNode(node)
				grid.inputHide(true)
				if(next) grid.edit(next,'column_comment')
			}
		}
	}
	gridMouseWheel(delta) {
		grid.inputHide()
	}
	gridMouseDown(pos) {
		hh=grid.headerHeight()
		node=grid.at(pos.incrY(hh))
		not(node) {			
			return; 
		}
		grid.inputHide()
		field=node.var(code) 
		if(field.eq('chk')) {
			chk=when(node.flag(NODE.check), false, true)
			node.flag(NODE.check, chk)
			grid.update()
		} else if(field.eq('COLUMN_DESCRIPTION') ) {
			grid.edit(node, field)
		} else if(node.flag(NODE.add)) {
			grid.edit(node, field)
		}
	}
	gridDraw(dc, node, index, state) {
		field=grid.field(index)
		rc=grid.drawState(dc, node, state, index, field )
		this.gridDrawNode(dc, rc, node, field)
	}
	gridDrawNode(dc,rc, node, field) { 
		if(field.eq('chk')) {
			if(node.flag(NODE.add)) {
				dc.fill(rc.incr(2),'#eaa')
			} else if(node.flag(NODE.modify)) {
				dc.fill(rc.incr(2),'#aae')
			}
			if(node.flag(NODE.check)) {				
				dc.image(rc.center(20,20), 'icons:check1')
			} else {
				dc.rectLine(rc.center(16,16), 0, '#888', 2)
			} 
		} else {
			dc.text(rc.incrX(2), node.get(field))
		}
	}
	gridHeaderClick() {
		grid=this
		idx=grid.var(clickIndex)
		if(idx==0) {
			if(grid.var(checkedAll)) {
				bchk=false				
			} else {
				bchk=true
			}
			grid.var(checkedAll, bchk)
			while(cur, grid.rootNode()) {
				cur.flag(NODE.check, bchk)
			} 
		}
	}
	setTable(node) {
		db=Baro.db('locknlock')
		root=grid.rootNode().removeAll()
		root.name=node.name
		table=root.name
		db.fetchAll(this.conf("sqlTableDetail"), root, true)
		this.title("테이블: ${node.name} 컬럼수: ${root.childCount()}")
		while(cur, root) {
			comment=conf("columnComment.${table}:${cur.COLUMN_NAME}")
			if(comment) {
				cur.COLUMN_DESCRIPTION=comment
			}
		}
		grid.update()
	}	 
	click_ApplyComment() {
		root=grid.rootNode()
		table=root.name
		cnt=0
		while(cur, root, num) {
			not(cur.flag(NODE.modify)) continue;
			conf("columnComment.${table}:${cur.COLUMN_NAME}", cur.COLUMN_DESCRIPTION, true)
			cur.flag(NODE.modify, false)
			cnt++;
		}
		alert("$cnt 건 설명을 추가했습니다")
		grid.update()
	}
	click_MoveDown() {
		cur=grid.current()
		next=grid.nextNode(cur)
		if(next) grid.current(next)
	}
	click_MoveUp() {
		cur=grid.current()
		prev=grid.prevNode(cur)
		if(prev) grid.current(prev)
	}
	click_MakeQuery() {
		root=grid.rootNode()
		table=root.name
		ss='', num=0
		while(cur, root) { 
			if( cur.flag(NODE.check) ) { 
				if(ss) ss.add(', ')
				ss.add(cur.COLUMN_NAME)
				num++;
			}
		}
		not(num) {
			return this.alert('선택된 필드정보가 없습니다')
		} 
		sql=#[
			SELECT
				TOP 100 ${ss}
			FROM
				${table}
			WHERE 1=1
		];
		parent=this.parentWidget()
		if(parent) {
			parent.var(strEditQuery, sql) 
		}
		this.hide()
	}
}

/* 다국어 조회 페이지 */
class mmLangMng {
	initClss() {
		@db=Baro.db('locknlock')
		@form=this.makeWidget()
		@grid=this.makeWidget('combo', 'gridLangMng')
		@comboLangType=this.makeWidget('combo','comboDbTableLangType')
		@comboScreenIds=this.makeWidget('combo', 'cbScreenIds')
		@comboUxCodes=this.makeWidget('combo', 'cbUxCodes')
		grid.model('text')
		grid.is('sortEnable', true)
		comboScreenIds.setEvent('onChange', this, this.changeScreenId)
		form.setFormInfo('mmLangMngFrom',this)
	}
	initPage() {
		node=_node('combo.langTypes')
		db.fetchAll(this.conf('sqlLangTypes'), node )
		comboLangType.addItem(node, 'LANGUAGE,TEXT','=언어전체=')
		comboLangType.update()

		arr=this.conf('screenIds').split()
		node=_node('combo.screenIds').removeAll()
		while(code, arr) {
			text=code
			node.addNode().with(code, text)
		}
		comboScreenIds.addItem(node,'code,text','==스크린번호 선택==')
		comboUxCodes.addItems('UX254,UX251,UX249,UX250,UX257')
	}
	changeScreenId() { 
		not(comboScreenIds.current()) return;
		this.search()
	}
	search() {
		cur=comboScreenIds.current()
		root=grid.rootNode().removeAll()
		lang=comboLangType.current()
		root.SCREEN_ID=cur.code
		root.LANGUAGE=when(lang,lang.get('LANGUAGE'))
		db.fetchAll(this.conf('sqlLangDtlList'), root, true)		
		grid.fields(root.var(fields)) 
		if( form.isCheck('CheckSameSize') ) {
			grid.fullWidth(true)
		} else {
			grid.fullWidth('resizeToContent')
		}
		grid.selectClear()
		grid.update()
	}
}

class conf {
	mainMenu={
		actionList: [
			{id:menu.dbTableMng, text:DB테이블관리, icon:'vicon:application_lightning', type:"page", sub:"splitter"}
			{id:menu.mmLangMng, text:금형다국어관리, icon:'vicon:html_go', type:"page", sub:"form"}
			{id:menu.mmSourcePage, text:금형소스조회, icon:'vicon:script_code_red', type:"page", sub:"div"}
			{id:menu.close, text:프로그램 종료, icon:'icons:close'}
		]
		menus: [
			{actionId:menu.dbTableMng }
			{actionId:menu.mmLangMng }
			{actionId:menu.mmSourcePage }
			{type:separator }
			{actionId:menu.close }
		]
	}
	dbTableLeftFormInfo: { rows: {
		vbox:'30,*,34', cellMargin:4
		{
			{tag:label, text:'DB테이블 정보'}
		}, {
			widget:@tree, margin:2
		}, { 
			{tag:label, text:필터 }
			{widget:@input, width:150}
			{tag:space}
		}
	}}
	dbTableContentFormInfo: {  rows: {
		rowInfo:'*,38', cellMargin:4
		{
			widget:@grid, margin:2
		}, {
			{tag:btn, id:RunQuery, text:쿼리실행, icon:'vicon:database_table'}
			{tag:btn, id:AddRow, text:레코드추가, icon:'vicon:drive_add'}
			{tag:check, id:CheckRunSelect, text:쿼리선택, tip:조회후 쿼리선택  }
			{tag:check, id:CheckSameSize, text:같은폭, checked:true, tip:그리드 필드를 같은폭으로 맞추기 }
			{widget:@inputTable, width:140}
			{tag:btn, id:ApplyModify, text:적용쿼리생성, icon:'vicon:drive_edit'}
			{tag:check, id:CheckTab, text:탭넣기, checked:treu}
			{tag:check, id:CheckJsValue, text:따옴표넣기}
			{tag:btn, id:CopyGrid, text:그리드복사, icon:'vicon:drive_go'}
			{tag:space}	
			{tag:btn, id:SaveQuery, text:쿼리저장, tip:쿼리실행시 저장 입력폼 팝업 }
		}
	}}
	popupTableInfoForm: {  rows: {
		rowInfo:'*,34', cellMargin:4
		{
			widget:@grid, margin:2
		}, {		 
			{tag:btn, id:ApplyComment, text:적용}
			{tag:btn, id:MoveDown, text:아래로}
			{tag:btn, id:MoveUp, text:위로}
			{tag:btn, id:MakeQuery, text:선택 쿼리작성}
			{tag:space}
			{tag:btn, id:DeleteRow, text:삭제}
			{tag:btn, id:NewTable, text:새테이블 만들기}
		}
	}}

	mmLangMngFrom: { cellMargin:[4,2], rowInfo:'*,34', rows: {
		{
			widget:@grid, margin:2
		}, {		 
			{tag:btn, id:ApplyData, text:적용} 
			{tag:space}
			{widget:@comboLangType, width:90 }
			{widget:@comboScreenIds}
			{widget:@comboUxCodes}
		}
	}}
}


class conf {
	screenIds: 
	<text>
		MM_612_1,MM_603,MM_616_7,MM_615_2,MM_615,MM_601_2,MM_604,MM_607_2,MM_603_1,MM_616_5,MM_615_1,MM_616_8,MM_616_3,MM_616_9,MM_616_6,MM_610_4,MM_610_2,MM_613_1,MM_614_1,MM_7_2_POP,MM_610_1,MM_610_3,MM_604_1,MM_606_1,MM_618_2,MM_616
	</text>
	
	messageIds:
	<text>
		0004,0002,6002,1002,NO_EDIT_DATA,1004,1008,1006,5016,1018,1016,3000,0001,9999,0091,MSG_1001,5107,1007,0095,5015,EDITING,1015,1049,1021,TITLE_USER_SEARCH
	</text>
	
	
	sqlLangTypes:
	<sql>
		SELECT CODE AS LANGUAGE ,TEXT1 AS TEXT, SORT_SEQ AS SEQ
		FROM SCODE
		WHERE 1=1
			AND LANGUAGE   = 'KO'
			AND USE_FLAG   = 'Y'                  
			AND TYPE       = 'AD001'              
		ORDER BY SORT_SEQ
	</sql>
	
	sqlLangDtlList:
	<sql>
		SELECT 
           CODE.SCREEN_ID
           ,CODE.CODE
           ,CODE.CODE CODE_HD
		   ,LANG.LANGUAGE
           ,LANG.CONTENTS AS CONTENTS
		FROM (
			SELECT DISTINCT SCREEN_ID ,CODE FROM SLANG
			WHERE 1=1
				AND SCREEN_ID  = #{SCREEN_ID}
				AND LANGUAGE in (SELECT CODE FROM SCODE WHERE TYPE ='AD001')              
		) CODE
		JOIN SLANG LANG 
			ON CODE.SCREEN_ID  = LANG.SCREEN_ID 
				AND CODE.CODE       = LANG.CODE
				#[LANGUAGE ? AND LANG.LANGUAGE = #{LANGUAGE}]
		ORDER BY HOUSE_CODE, SCREEN_ID, CODE 	
	</sql>
	
	sqlTableInfo: 
	<sql>
	  select a.name, a.crdate
	  FROM SYSOBJECTS A join SYSUSERS B on A.uid=B.uid and a.xtype='U'
	  where 1=1
		and a.name not like 'np_%'
	  order by A.name
	</sql>
	
	sqlTableDetail: 
	<sql>
		SELECT D.COLORDER                	AS COLUMN_IDX            
			, A.NAME                    	AS TABLE_NAME            
			, C.VALUE                    	AS TABLE_DESCRIPTION
			, D.NAME                    	AS COLUMN_NAME
			, E.VALUE                    	AS COLUMN_DESCRIPTION
			, F.DATA_TYPE                	AS TYPE  
			, F.CHARACTER_OCTET_LENGTH    	AS LENGTH
			, F.IS_NULLABLE            		AS IS_NULLABLE
			, F.COLLATION_NAME            	AS COLLATION_NAME
		FROM SYSOBJECTS A  
		JOIN SYSCOLUMNS D        ON D.ID = A.ID
		JOIN INFORMATION_SCHEMA.COLUMNS F
			ON A.NAME = F.TABLE_NAME
			AND D.NAME = F.COLUMN_NAME
		LEFT OUTER JOIN SYS.EXTENDED_PROPERTIES C
			ON C.MAJOR_ID = A.ID
			AND C.MINOR_ID = 0
			AND C.NAME = 'MS_Description'
		LEFT OUTER JOIN SYS.EXTENDED_PROPERTIES E
			ON E.MAJOR_ID = D.ID
			AND E.MINOR_ID = D.COLID
			AND E.NAME = 'MS_Description'  
		WHERE 1=1
		AND A.TYPE = 'U'
		AND A.NAME = #{name} and A.uid ='1'
		ORDER BY D.COLORDER
	</sql>
}

class func {
	insertSLANG(formId, code) {
		db=Baro.db('locknlock')
	
		arr=[KO,EN,VN,ZH]		
		while(lang, arr) {
			ss="insert into slang  (house_code, screen_id, language, code, contents, add_user_id, add_date, add_time, change_user_id, change_date, change_time, del_flag, grid_width, form_width) values ('000', '$formId', '$lang', '$code', 'xxx', 'locknlock.srm4@locknlockdev.com', '20240725', '135638', 'locknlock.srm4@locknlockdev.com', '20240725', '161405', 'N', '120', '')"
			not(db.fetch("select code from slan where house_code='000' and screen_id='$formId' and language='$lang' and code='$code' ")) {
				db.exec(ss)
			}
		}
	}
	sformJson(formId) {
		row=db.fetch("select form_json from sform where form_id='$formId'")
		node=_node(formId).removeAll(true)
		data=sformParse(row.form_json)
		node.parseJson("form:$data")
		return do(node)
	} 
	sformParse(&s) {
		ss=''
		while(s.valid()) {
			left=s.findPos('&#34;')
			ss.add(left)
			not(s.ch()) break;
			ss.add('"')
		}
		return ss;
	}
	moveSqlModify(dist) {
		base='D:\Dev\03.WorkSpace\Poa-STD-SUXv4' 
		dest='C:\poastd'
		rel='\poasrm\Poa-Package\service_query\mssql\sepoa\svc'
		path=Cf.val(base,rel)
		tm=System.localtime()
		not(dist) dist=24*60*60*60
		tm-=dist;
		parse(path, tm)
		parse=func(path, tm, pathLen) {
			not(pathLen) {
				pathLen=path.size()
			}
			fo=Baro.file()
			fo.list(path, func(info) {
				while(info.next()) {
					info.inject(type, name, fullPath, ext, modifyDt)
					if(type.eq('folder')) {
						parse(fullPath, tm, pathLen)
						continue;
					}
					if(modifyDt < tm) continue;
					relative=fullPath.trim(pathLen)
					destPath=Cf.val(dest,rel,relative) 
					if(isFile("${destPath}.old")) fo.delete("${destPath}.old")	
					fo.rename(destPath, "${destPath}.old")
					rst=fo.copy(fullPath,  destPath)
					print("xxx ", relative, modifyDt, destPath, rst)
				}
			})
		}; 
	}
	moveAuxModify(dist) {
		base='D:\Dev\03.WorkSpace\Poa-STD-SUXv4' 
		dest='C:\poastd'
		rel='\poasrm\Poa-Package\sepoaux\xml'
		path=Cf.val(base,rel)
		tm=System.localtime()
		not(dist) dist=24*60*60*60
		tm-=dist;
		parse(path, tm)
		parse=func(path, tm, pathLen) {
			not(pathLen) {
				pathLen=path.size()
			}
			fo=Baro.file()
			fo.list(path, func(info) {
				while(info.next()) {
					info.inject(type, name, fullPath, ext, modifyDt)
					if(type.eq('folder')) {
						parse(fullPath, tm, pathLen)
						continue;
					}
					if(modifyDt < tm) continue;
					relative=fullPath.trim(pathLen)
					destPath=Cf.val(dest,rel,relative) 
					if(isFile("${destPath}.old")) fo.delete("${destPath}.old")
					fo.rename(destPath, "${destPath}.old")
					rst=fo.copy(fullPath, destPath)
					print("xxx ", relative, modifyDt, destPath, rst)
				}
			})
		}; 
	}
	moveJavaModify(dist) {
		base='D:\Dev\03.WorkSpace\Poa-STD-SUXv4\poasrm\Poa-Package\sepoaserviceclasses\sepoa\svc\mm' 
		dest='C:\poastd\poasrm\Poa-Package\sepoaserviceclasses\sepoa\svc\mm'
		tm=System.localtime()
		not(dist) dist=24*60*60 
		tm-=dist;
		parse(base, tm)
		parse=func(path, tm, pathLen) {
			not(pathLen) {
				pathLen=path.size()
			}
			fo=Baro.file()
			fo.list(path, func(info) {
				while(info.next()) {
					info.inject(type, name, fullPath, ext, modifyDt)
					if(type.eq('folder')) {
						parse(fullPath, tm, pathLen)
						continue;
					}
					if( modifyDt < tm) continue;
					relative=fullPath.trim(pathLen)
					destPath=Cf.val(dest,relative) 
					if(isFile("${destPath}.old")) fo.delete("${destPath}.old")
					fo.rename(destPath, "${destPath}.old")
					rst=fo.copy(fullPath, destPath)
					print("xxx ", relative, modifyDt, destPath, rst)
				}
			})
		}; 
	} 
	copySourceTemp(dist) {
		fo=Baro.file();
		date=System.date('yyyyMMdd')
		path="C:/poastd/poasrm/Poa-Package"
		dest="C:/TEMP/na/$date" 
		not(isFolder(dest)) fo.mkdir(dest)
		tm=conf("localtime.lastCopySourceTemp")
		if(dist) tm=null
		not(tm) {
			not(dist) dist=24*60*60*40
			tm=System.localtime()
			tm-=dist;
		}
		conf("localtime.lastCopySourceTemp", System.localtime(), true)
		ss='';
		ss.add("mkdir ${localPath(dest,'backup')}\r\n")
		parse(path, tm)
		parse=func(path, tm, pathLen) {
			not(pathLen) {
				pathLen=path.size()
			}
			fo=Baro.file()
			fo.list(path, func(info) {
				while(info.next()) {
					info.inject(type, name, fullPath, ext, modifyDt)
					if(type.eq('folder')) {
						parse(fullPath, tm, pathLen)
						continue;
					}
					if(name.find(".old")) continue;
					if(modifyDt < tm) continue; 
					a=localPath(fullPath)
					b=localPath(dest,name)
					c=localPath(dest,"backup",name)
					ss.add("copy $a $c /y\r\n")
					ss.add("copy $b $a /y\r\n")
					rst=fo.copy(a, b)
				}
			})
		};
		// System.copyText(ss)
		fileWrite("$dest/copy.bat", ss)
	} 
}

class layout {
	<page id="main" title="락앤락 DB 관리툴" margin="0">
		<canvas id="topMenu" height="38">
		<div id="content">
	</page>
}

