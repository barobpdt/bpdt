class main {
	initClass() {
		@splitter=findTag(this,'splitter')
		@left=this.makeWidget('canvas','leftPanel') class(left,'leftPanel',true)
		@content=this.getWidget('content') class(content,'contentPage',true)
		@page.margin(content,4,0,8,0) 
		this.var(bgColor, color('#3D448B'))
		this.timer(500)
	}
	initPage() {
		this.positionLoad()
		this.open()
		splitter.addPage(left)
		splitter.addPage(content)
		tot=splitter.sizes().sum()
		print("init page splitter ", tot, splitter.sizes() )
		splitter.sizes(recalc(tot,'3,7'))
		splitter.stretchFactor(1)
		left.initPage()
		content.initPage()		
	}
	onTimer() { 
		if( this.firstCall ) {
			this.firstCall=false
			this.initPage()
		}
		if( this.var(strEditQuery) ) { 			
			content.setQuery(this.var(strEditQuery))
			left.popup.hide()
			this.var(strEditQuery,null)
		}
	}
	getLangCode() {
		cur=left.cbScreenIds.current() not(cur) return "all";
		return cur.LANGUAGE;
	}
}

class popupTableInfo {
	initClass() {
		class(this,'form')
		this.addButtons(
			'ApplyComment, MoveDown, MoveUp, MakeQuery, *, DeleteRow, NewTable', 
			'적용, 아래로, 위로, 선택 쿼리작성, *, 삭제, 새테이블 만들기', color('#8B3D53AA')
		)
		@grid=this.makeWidget('grid', 'gridTableInfo', 'rcGrid') class(grid, 'grid')
		input=this.makeWidget('input', 'inlineInfoEditor')
		input.parentWidget(this)
		input.setEvent('onKeyDown', this, this.keydown)
		grid.setInput(input)
		grid.var(targetForm, this)
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
		grid.var(bgColor, color('#528B3DE0'))
		grid.setEvent('onDraw', this, this.gridDraw)
		grid.setEvent('onMouseDown', this, this.gridMouseDown)
		grid.setEvent('onMouseWheel', this, this.gridMouseWheel)
		this.updateButtons=null
		this.initFormCheck()
		this.timer(500)
	}
	onTimer() {
		if( grid.var(editStartTick)) {
			grid.var(editStartTick,0)
			grid.inputFocus()
		}
	}
	updateForm(rc) {
		vbox(rc, '*,32').inject(rcBody, rcStatus)
		rcGrid=rcBody.incr(2)
		btnInfo=this.getButtonWidth()
		arr=hbox(rcStatus.incrYH(4,2), btnInfo)
		while(btn, buttons, idx ) {
			not(btn.rectId) continue;
			btn.rectClient=arr.get(idx)
		}
		this.setFormRect()
		this.with(rcBody, rcStatus)
	}
	drawForm(dc, rc) {
		this.inject(rcBody, rcStatus)
		dc.fill(rcBody, '#fff')
		dc.fill(rcStatus, '#ccc')
		dc.rectLine(rc, 34, '#999', 2)
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
				if(next) {
					grid.current(next)
					grid.edit(next,'COLUMN_DESCRIPTION')
				}
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
			grid.inputHide()
			return; 
		}
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
	
	setTable(node) {
		db=Baro.db('locknlock')
		root=grid.rootNode().removeAll()
		root.name=node.name
		table=root.name
		db.fetchAll(this.conf("tableDetail"), root, true)
		this.title("테이블: ${node.name} 컬럼수: ${root.childCount()}")
		while(cur, root) {
			comment=conf("columnComment.${table}:${cur.COLUMN_NAME}")
			if(comment) {
				cur.COLUMN_DESCRIPTION=comment
			}
		}
		grid.update()
	}
	
	buttonClick(id) {
		fnm="click$id"
		fc=this.get(fnm)
		if(typeof(fc,'func')) call(fc, this)
	}
	clickApplyComment() {		 
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
	clickMoveDown() {
		cur=grid.current()
		next=grid.nextNode(cur)
		if(next) grid.current(next)
	}
	clickMoveUp() {
		cur=grid.current()
		prev=grid.prevNode(cur)
		if(prev) grid.current(prev)
	}
	clickMakeQuery() {
		root=grid.rootNode()
		table=root.name
		ss='', num=0
		while(cur, root) { 
			if(cur.flag(NODE.check) ) { 
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
				${ss}
			FROM
				${table}
			WHERE 1=1
		];
		page('main').var(strEditQuery, sql)
	}
}

class leftPanel {
	initClass() { 
		class(this,'form')
		@db=Baro.db('locknlock')
		@popup=this.makeWidget('canvas','popupTableInfo') class(popup, 'popupTableInfo', true)
		@tree=this.makeWidget('tree','tableTree','rcTree') class(this,'tree')
		tree.var(treeMode, false)
		tree.setEvent('onDraw', this, this.treeDraw)
		tree.setEvent('onMouseDown', this, this.treeMouseDown)
		tree.setEvent('onMouseMove', this, this.treeMouseMove)
		tree.setEvent('onFilter', this, this.treeFilter)
		tree.var(treeMode, false)
		tree.model('name')
		@combo=this.makeWidget('combo', 'cbLang', 'rcLang')
		@input=this.makeWidget('input','inputFilter','rcInput')
		
		combo.addItem(_node('combo.langTypes'), 'LANGUAGE,TEXT')
		input.setEvent('onTextChange', this, this.inputFilterChange)
		this.initFormCheck()
		this.timer(500)
	}
	initPage() {
		node=_node('combo.langTypes')
		db.fetchAll(this.conf('langTypes'), node)
		combo.update()
		this.treeData()
	}
	onTimer() { 
		if( this.var(filterChangeTick)) {
			dist=System.tick() - this.var(filterChangeTick);
			if(dist>250) {
				this.var(filterChangeTick,0)
				tree.update()
			}
		}
	}
	inputFilterChange() {
		this.var(filterChangeTick, System.tick())
	}
	updateForm(rc) {
		vbox(rc,'30,*,34').inject(rcTitle, rcBody, rcStatus)
		hbox(rcStatus.margin(4,4,4,6), '40,180,*,80').inject(rcLabel, rcInput, rcSpace,rcLang)
		rcTree=rcBody.incr(2)
		this.with(rcTitle, rcBody, rcStatus, rcLabel, rcLang)
		this.setFormRect()
	}
	drawForm(dc, rc) {
		this.inject(rcTitle, rcBody, rcStatus, rcLabel, rcLang)
		bgColor=page('main').var(bgColor)
		dc.fill('#aaa')
		dc.fill(rcTitle.incr(1), bgColor.darkColor(100))
		dc.fill(rcStatus.incr(1), '#ddd')
		dc.pen(bgColor.lightColor(150)).text(rcTitle.incr(4), "락앤락 테이블 정보")
		dc.pen(bgColor.darkColor(100)).text(rcLabel,"필터 :")
	}
	popupTableInfo(node) {
		rc=tree.nodeRect(node)
		rcPopup=rc(rc.rb(),900,450)
		rcGlobal=tree.mapGlobal(rcPopup)
		popup.move(rcGlobal)
		popup.open()
		popup.active()
		popup.setTable(node)
	}
	treeDraw(dc, node, index, state, over) { 
		rc=this.drawSelect(dc, dc.rect(), node, col, state, over)
		node.rcIcon=rc.moveLeft(18,18,-2,0,true) 
		node.inject(name, comment)
		btnIcon='vicon:database_table'
		dc.font(10).pen('#223').text(rc, name)
		dc.textSize(name).inject(tw, th)
		dist=rc.width() - tw;
		if(dist.gt(60)) {
			tw+=4;
			rcIcon=rc.incrX(tw).leftCenter(20,20) 
			dc.rectLine(rcIcon,0,'#eee')
			dc.image(rcIcon.center(18,18), btnIcon)
			node.rcBtn=rcIcon
			if(comment) {
				tw+=20;
				rcComment=rc.incrXW(tw, 4) 
				dc.font(9).pen('#966').text(rcComment, "- $comment", "right")
			}
		}
	}
	treeData() {
		root=tree.rootNode()
		db.fetchAll(this.conf('tableInfo'), root)
		while(cur, root) {
			cur.comment=conf("tableComment.${cur.name}")
		}
		tree.update()
	}
	treeMouseMove(pos) {
		node=tree.at(pos)
		if(node ) {
			node.inject(rcBtn, rcCopy)
			if(rcBtn.contains(pos) ) {
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
	treeMouseDown(pos,key,btn) { 
		node=tree.at(pos)
		not(node) return;
		node.inject(rcIcon, rcBtn)
		if(rcIcon.contains(pos)) return;
		if( this.rcOver ) {
			if( rcBtn.contains(pos) ) {
				this.popupTableInfo(node)
				return 'ignore'
			}
		}
		rc=tree.nodeRect(node)
		if(this.currentTreeNode!=node) {
			this.currentTreeNode=node
			tree.current(node)
			tree.expand(node, true, true)
			page('main').var(currentTreeNode, node)
		}
		return 'ignore';
	}
	treeFilter(node) {
		text=input.value()
		not(text) return true;
		if(node.get('name').find(text,2)) return true;
		return false;
	}
}

class contentPage {
	initClass() {
		@splitter=findTag(this,'splitter')
		@editor=this.makeWidget('editor','editorMssql') class(editor,'editorSql')
		@form=this.makeWidget('canvas', 'contentForm') class(form,'contentForm', true)
		splitter.addPage(editor)
		splitter.addPage(form)
	}
	initPage() {
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'*,34'))
		splitter.stretchFactor(0)
		print("content init page ", splitter.sizes() )
		form.initPage()
	}
	setQuery(query) {
		db=Baro.db('locknlock')
		grid=form.grid
		editor.insertQuery(query)
		splitter=findTag(this,'splitter')
		hh=splitter.sizes().get(1)
		if(hh<100) {
			tot=splitter.sizes().sum()
			splitter.sizes(recalc(tot,'*,300'))
		} 
		root=grid.rootNode().removeAll()
		db.fetchAll(query, root, true)
		ss='', num=0
		while(field,root.var(fields), num) {
			if(num) ss.add(',')
			ss.add(field)
		}
		grid.fields(ss)
		if( num<10 ) {
			grid.size().inject(width)
			arr=recalc(width,num)
			grid.headerWidth(arr)
		} else {
			arr=_arr()
			while(n=0, num) {
				arr.add(100)
			}
			grid.headerWidth(arr)
		}
		grid.update()
		form.update()
	}
}

class contentForm {
	initClass() {
		class(this, 'form') 
		@grid=this.makeWidget('grid', 'gridDetail', 'rcGrid')
		@cbScreenIds=this.makeWidget('combo', 'cbScreenIds', 'rcScreenIds')
		@cbUxCodes=this.makeWidget('combo', 'cbUxCodes', 'rcScreenIds')
		cbScreenIds.setEvent('onChange', this, this.changeScreenId)
		grid.model('text')
		grid.is('sortEnable', true)
		this.initFormCheck()
	}
	initPage() {
		arr=this.conf('screenIds').split()
		node=_node('combo.screenIds').removeAll()
		while(code, arr) {
			text=code
			node.addNode().with(code, text)
		}
		cbScreenIds.addItem(node,'code,text','==스크린번호 선택==')
		cbUxCodes.addItems('UX254,UX251,UX249,UX250,UX257')
	}
	updateForm(rc) {
		not(rc) rc=this.rect()
		vbox(rc,'*,34').inject(rcBody, rcStatus)
		if( rcBody.height()>100) {
			rcGrid=rcBody.incr(2)
			num=grid.fields().childCount()
			if( num < 10 ) {
				tw=rc.width()-28;
				grid.headerWidth(recalc(tw,num))
			}
		} 
		hbox(rcStatus.margin(4,4,4,8), "75,180,*,80").inject(rcLabel, rcScreenIds, rcSpace, rcEtc)
		this.with(rcBody, rcStatus, rcLabel,  rcSpace, rcEtc ) 
		this.setFormRect()
	}
	drawForm(dc, rc) {
		this.inject(rcBody, rcStatus, rcLabel,  rcSpace, rcEtc)
		bgColor=page('main').var(bgColor)
		dc.fill('#fff') 
		dc.fill(rcStatus.incr(1), '#888')
		dc.pen(bgColor.lightColor(210)).text(rcLabel, "스크린번호")
	}
	changeScreenId() {
		cur=cbScreenIds.current()
		not(cur) return;
		lang=page('main').getLangCode()
		cur.SCREEN_ID=cur.code
		cur.LANGUAGE=when(lang.eq('all'),'',lang)
		sql=fmtQuery(this.conf('langDtlList'), cur)
		page('main').var(strEditQuery, sql)
	}
}

class func {
	tableCommentQuery(root) {
		ss=''
		while(cur,root, n) {
			if(n) ss.add("\n")
			ss.add("SELECT objname as table, value FROM ::fn_listextendedproperty (NULL, 'schema', 'dbo', 'table', '${cur.name}', default, default)")
		}
		return ss;
	}
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
	
	
	langTypes:
	<sql>
		SELECT CODE AS LANGUAGE ,TEXT1 AS TEXT, SORT_SEQ AS SEQ
		FROM SCODE
		WHERE 1=1
			AND LANGUAGE   = 'KO'
			AND USE_FLAG   = 'Y'                  
			AND TYPE       = 'AD001'              
		ORDER BY SORT_SEQ
	</sql>
	
	langDtlList:
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
				#{[LANGUAGE ? AND LANG.LANGUAGE = #{LANGUAGE}]}
		ORDER BY HOUSE_CODE, SCREEN_ID, CODE 	
	</sql>
	
	tableInfo: 
	<sql>
	  select a.name, a.crdate
	  FROM SYSOBJECTS A join SYSUSERS B on A.uid=B.uid and a.xtype='U'
	  where 1=1
		and a.name not like 'np_%'
	  order by A.name
	</sql>
	
	tableDetail: 
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

class layout {
	<page id="main" title="락앤락 DB 관리툴" margin="0">
		<splitter type="hbox">
	</page>
	
	<page id="content" title="쿼리 조회페이지">
		<splitter type="vbox">
	</page>
}
