/* API 관리 */
class main {
	initClass() { 
		global("host", 'http://106.246.249.162:38055')
		@page.spacing(this,0)
		@splitter=findTag(this,'splitter')
		@left=this.makeWidget('div','leftPanel') 			class(left,'leftPanel', true)
		@content=this.getWidget('content') 					class(content,'contentPage', true)
		@apiInputDialog=this.getWidget('apiInputDialog', true) 
		@topMenu=this.getWidget('topMenu', true)
		@keyMapMng=this.makePage('keyMapMng','margin:0', 'div')
		@formInfoMng=this.makePage('formInfoMng','margin:0', 'div')
		splitter.addPage(left)
		splitter.addPage(content)

		formInfoConf('promptComment', this)
		formInfoConf('promptEdit', this) 
		this.timer(250)
	}
	
	initPage() {
		appMenus=this.conf('topMenu',true)
		this.positionLoad() 
		this.action(appMenus.actionList )
		topMenu.setMenuInfo(this, appMenus)
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'3,7'))
		topMenu.changeMenu('menu.apiForm')
	} 
	onClose() {
		this.positionSave()
	}
	onTimer() {
		if( this.firstCall ) {
			this.firstCall=false
			this.initPage()
		} 
		if( this.var(menuOverTick) ) {
			dist=System.tick() - menuOverTick;
			if(dist.gt(500)) {
				topMenu.mouseOverCheck()
			}
		}
		if( this.var(strEditQuery) ) { 			
			left.popup.hide()
			curPage=content.div.current()
			curPage.setQuery(this.var(strEditQuery))
			this.var(strEditQuery,null)
		}
		if( this.var(apiResultText) ) {
			content.apiPage.editor.append(this.var(apiResultText), true); 
			this.var(apiResultText,null)
		}
		if( this.var(apiSendInfo) ) {
			text=this.var(apiSendInfo)
			form=left.current() 
			form.setApiSendInfo(text)
			content.apiPage.editor.append("\n전송시작: $text")
			apiInputDialog.hide()
			this.var(apiSendInfo,null)
		}
		if( this.var(logMessage) ) {
			logTick=this.var(logMessageTick)
			if(logTick) {
				dist = System.tick() - logTick;
				if(dist<5000 ) return;
				this.var(logMessage,'')
				this.var(logMessageTick,0)
			} else {
				this.var(logMessageTick, System.tick())
			}
			page=content.div.current()
			if( page.form ) page.form.setLogText(this.var(logMessage))
		}
		if( this.var(focusWidget) ) {
			this.var(focusWidget).focus()
			this.var(focusWidget,null)
		}
		if( this.var(templateFilterTick) ) {
			dist=System.tick() - this.var(templateFilterTick)
			if(dist>500) {
				this.var(templateFilterTick, null)
				this.var(templateFilterText, inputTemplateFilter.value() )
				gridTemplate.update()
			}
		}
	}
	onAction(action) {
		if(action.cmp('type','page')) {
			return topMenu.changeMenu(action.id)
		}
		print("action trigger action=${action.id}")
		switch(action.id) {
		case 'sourceEditor':
			classLayout("app/sourceEditor").open()
			Cf.debug(true, "data/logs")
		case 'locknlock':
			classLayout("app/app_locknlock")
		case 'manu.close':
			this.close()
		default:
			target=this.currentMenuTarget
			print("menu target ==> ${target.id}")
			target.execAction(action)
		}
	}
	changeAction(action, tab) {
		not(tab) return alert("$actionId 메뉴를 찾을수 없습니다")
		actionId=action.id
		prev=this.var(currentMenuTab)
		if(prev==tab) {
			print("이미 선택된 메뉴입니다 (메뉴:$tab)")
			return;
		}
		if( prev ) {
			if( prev.useLeftPanel) {
				prev.addArray('splitterSizes',true).copy(splitter.sizes())
			}
		}
		this.var(currentMenuTab, tab)
		if( tab.useLeftPanel) {
			sizes=tab.splitterSizes
			if(typeof(sizes,'array') ) {
				splitter.sizes(sizes)
			}
			left.show()
			left.setCurrent(actionId)
		} else {
			left.hide()
		}
		content.setCurrent(actionId)
		topMenu.update()
	}
	addTopMenu() {
		vbox=this.child(0)
		vbox.addChild(topMenu,this,0) 
		return topMenu;
	}
	removeTopMenu() {
		vbox=this.child(0)
		vbox.removeChild(topMenu)
	}
	openPage(page, title, w, h) {
		page.title(title)
		page.size(w,h)
		page.parentWidget(null)
		page.flag('window')
		page.open()
		page.active()
	}
	onKeyDown(k,a) {
		bctrl=a&KEY.ctrl;
		not(bctrl) return;
		if(k.eq(81) ) {
			input=left.current().get('input') not(input) return;
			input.focus()
			return true;
		} 
		if(a&KEY.shift) {
			if(k.eq(KEY.K) ) {
				this.openPage(keyMapMng, '에디터 키템플릿 관리', 900, 650 )
				div=keyMapMng.get('div') 
				not(div.current()) {					
					div.addPage(@form.get('keyTemplateSearch',this), true)
				}
			}
			if(k.eq(KEY.F) ) {
				form=@form.get('formInfoEdit',this)
				this.openPage(form,'폼정보 관리', 900, 650 )
			}	
		}
	}
	showMenu(menuCode, menuData, pos, target) {
		menus=topMenu.makeMenu(menuCode, menuData)
		if( typeof(menus,'node')) {
			this.currentMenuTarget=target;
			target.menu(this, menus, pos)
		}
		return menus;
	}
	popupClosed(popup) {

	}
	openComment(node ) {
		form=@form.get('inputComment', this) 
		form.openTool(this)
		form.setData("${node.text} 상세설명 입력", node, this)
	}
	openPromptEdit(node, tree) {
		// node props => tag, text, note, comment, template
		form=@form.getVisible('pageTemplateEdit', this, true) 
		form.openTool(this, title)
		form.setData('${node.text} 페이지 템플릿 작성', node, this, tree)
	}
	addTemplateDiv(root) { 
		form=@form.target('pageTemplateEdit')
		ctt=content.div.current().get('divPage') not(ctt) return;
		not(form) {
			form=@form.load('pageTemplateEdit', this) 
			ctt.get('div').addPage(form,true)
			return;
		}
		list=form.formInfo.var(targetList)
		while(cur, list, idx) {
			if(root) {
				find=findField(root,'text', cur.pageId)
				if(find) find.flag(NODE.set, true)
			}
			ctt.get('div').addPage(cur, idx.eq(0))
		}
	}
	addTemplateForm(node, tree) {
		form=@form.target('pageTemplateEdit')
		not(form) return; 
		ctt=content.div.current().get('divPage') not(ctt) return;
		list=form.formInfo.var(targetList)
		form=null;
		while(cur, list) {
			if(cur.cmp('pageId',node.text) ) {
				form=cur
				break;
			} 
		}
		not(form) {
			form=@form.load('pageTemplateEdit', this) 
			form.setData('${node.text} 페이지 템플릿 작성', node, this, tree)
		}
		ctt.get('div').addPage(form,true)
		node.flag(NODE.set, true)
		tree.udate()
	}
}

class leftPanel {
	initClass() {
		@apiForm=this.makeWidget('apiForm')
		@htmlTemplateForm=this.makeWidget('htmlTemplateForm')
		@dbTableForm=this.makeWidget('dbTableForm')
	}
	setCurrent(name) {
		switch(name) {
		case 'menu.apiForm': form=apiForm
		case 'menu.htmlTemplateForm': form=htmlTemplateForm
		case 'menu.dbTableForm': form=dbTableForm
		default: form=null
		}
		not(form) {
			return alert("leftPanel $name 폼 추가오류")
		}
		this.setCurrentForm(form)
	}
	setCurrentForm(form) {
		div=this
		if( div.isPage(form)) {
			div.current(form)
		} else {
			div.addPage(form, true)
			form.initForm()
		}
	}
}

class contentPage {
	initClass() {
		@div=this.getWidget('div')
		@apiPage=this.getWidget('apiPage', true)
		@htmlTemplatePage=this.getWidget('htmlTemplatePage', true)
		@dbTablePage=this.getWidget('dbTablePage', true)
	}
	setCurrent(name) {
		switch(name) {
		case 'menu.apiForm': page=apiPage
		case 'menu.htmlTemplateForm': page=htmlTemplatePage
		case 'menu.dbTableForm': page=dbTablePage
		default: 
			page=this.getWidget(right(name), true)
		}
		not(page) {
			return alert("content 페이지추가 오류 $name 페이지를 찾을수 없습니다")
		}
		this.setCurrentPage(page)
	}
	setCurrentPage(page) {
		if( div.isPage(page)) {
			div.current(page)
		} else {
			div.addPage(page, true)
			page.initPage()
		}
	}
}

/* API 관리 페이지 */
class apiInputDialog {
	initClass() {
		@combo=this.getWidget('comboApi')
		@comboMethod=this.getWidget('comboMethod')
		@inputApiAddr=this.getWidget('inputApiAddr')
		@editor=this.getWidget('data') class(editor,'editorSql') @template.editorKeyMap(editor)
		combo.setEvent('onChange', this, this.comboChange)
		this.getWidget('btnSend').setEvent('onClick', this, this.clickApiSend) 
	}
	openDialog(parent, url) {
		if(this.firstCall ) {
			data=_node('comboApi');
			comboMethod.addItems('GET,POST')
			combo.addItem(data, 'url, text','== 선택 ==')
			combo.popupSize('expanding', 400);
			combo.update()
			this.firstCall=false
		}
		if(url) this.setComboUrl(url)
		this.open(parent)
	}
	comboChange() {		 
		doc=_node('apiDoc')
		cur=combo.current() 
		not(cur) {
			inputApiAddr.value('')
			return;
		}
		host=global('host')
		comboMethod.value(cur.method)
		inputApiAddr.value("${host}${cur.url}")
		method=cur.method.lower()
		nl="\r\n"
		ss="## ${cur.text}"
		if(method.eq('get')) {
			path=Cf.val('paths.',cur.url,'.',method,'.parameters')
			params=findPath(doc, path)
			if(params) {
				ss.add(nl,"{")
				num=0
				while(cur, params) {
					if(cur.required) {
						if(num) ss.add(", ")
						ss.add(nl,"\t", Cf.jsValue(cur.name),': ""')
						num++;
					}
				}
				ss.add(nl,"}")
			}
		} else {
			path=Cf.val('paths.',cur.url,'.',method,'.requestBody.content.application/json.schema.$ref')
			print("post path==$path")
			ref=findPath(doc, path)
			if(ref) {
				objName=right(ref,'/')
				req=findPath(doc,"components.schemas.$objName")
				ss.add(nl,"{")
				while(name, req.required, num) {
					if(num) ss.add(", ")
					ss.add(nl,"\t", Cf.jsValue(name),': ""')
				}
				ss.add(nl,"}",nl, toString(req.properties, true, true))
			}
		}		

		editor.value(ss)
		page('main').var(apiResultText, "\r\n* API 전송 정보 >>\r\n$ss")
	}
	clickApiSend() {
		main=page('main')
		if(main.var(apiSendInfo)) return alert("실행중인 API가 있습니다");

		_data=func(&s) {
			s.findPos('##')
			s.findPos('{',1,1)
			not(s.ch()) return;
			ss=s.match(1)
			return "{$ss}";
		}
		str=Cf.val(inputApiAddr.value(),'<sep>',comboMethod.value(),'<sep>',_data(editor.value()) )
		main.var(apiSendInfo, str)
	} 
	setComboUrl(url) {
		data=_node('comboApi')
		cur=findField(data,'url',url);
		combo.current(cur)
	}
}
class apiForm {
	initClass() { 
		print("init tree form")
		@web=web('api')
		@tree=this.makeWidget('tree','treeApi') class(tree,'tree')
		@input=this.makeWidget('input','inputFilter')
		tree.var(treeMode, false)
		tree.model('text')
		tree.setEvent('onDraw', this, this.treeDraw, true)
		tree.setEvent('onChange', this, this.treeChange)
		tree.setEvent('onMouseDown', this, this.treeMouseDown)
		tree.setEvent('onMouseUp', this, this.treeMouseUp)
		tree.setEvent('onMouseMove', this, this.treeMouseMove)
		tree.setEvent('onFilter', this, this.treeFilter)
		input.setEvent('onTextChange', this, this.filterTextChange)
		this.setFormInfo('apiFormInfo')		
	}
	initForm() {
		root=tree.rootNode()
		root.addNode().with(tag:api, text:API 정보) 
		root.addNode().with(tag:result, text:API 호출결과 처리)
		this.update()
		this.treeApiDoc(this.callApi("/v3/api-docs"))
		this.timer(500)
	} 
	onTimer() {
		if( this.firstCall ) {
			this.initData()
			this.firstCall=false
		}
		if( this.var(filterChangeTick)) {
			dist=System.tick() - this.var(filterChangeTick);
			if(dist>500) {
				this.filterValue=input.value()
				tree.update() 
				this.var(filterChangeTick, 0)
			}
		}	
	}
	initData() { 
		this.treeApiResult(this.callApi(), true)
	} 
	callApi(url,data, post) {
		not(url) url="/api/main/getMainInfo?langCd=EN&currentDay=20240"
		if(url.ch('/')) {
			host=global('host')
			apiUrl="${host}${url}"
		} else {
			apiUrl=url
		}
		web.header('Content-Type','application/json')
		web.header('Authorization','Bearer eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJNMjQwNjA2MDAwMDAxMDIxIiwiTWVtYk5vIjoiTTI0MDYwNjAwMDAwMTAyMSIsIk1lbWJObSI6Ikd1ZXN0IiwiRW1haWwiOiIiLCJEZXZpSWQiOiIwMTIzNDU2Nzg5IiwiTWVtYkNkIjoiMjAiLCJpYXQiOjE3MTc2NTcxNDEsImV4cCI6MjMzMjgyNTE0MX0.TgNP8zgxnorOE6pX-Tgud05NI-s8rvNZirIb0w-E3RM')
		if( data || post ) {
			web.callPost(apiUrl,data)
		} else {
			web.call(apiUrl)
		}
		return web.result()
	}
	setApiSendInfo(&s) {
		url=s.findPos('<sep>').trim()
		method=s.findPos('<sep>').trim()
		data=s.findPos('<sep>').trim()
		if(method.eq('POST')) {
			result=this.callApi(url,data,true)
		} else {
			if(data) {
				node=_node().parseJson(data)
				while(key, node.keys(), n) {
					if(n) url.add('&') else url.add('?')
					val=node.get(key)
					url.add("$key=$val")
				}
			}
			result=this.callApi(url)
		}
		if(result) {
			this.treeApiResult(result)
		} else {			
			page('main').var(apiResultText, "\r\n$url\r\n호출오류")
		}
	}
	treeDraw(dc, node, index, state, over) {
		rc=tree.drawSelect(dc, dc.rect(), node, col, state, over) 
		rcText=rc.incrX(4)
		node.rcIcon=rc.moveLeft(18,18,-2,0,true)
		text=node.text
		dc.save()
		switch(node.tag) {
		case apiItem:
			node.rcBtn=rcText.rightCenter(18,18,-4)
			dc.html(rcText, text)
			dc.image(node.rcBtn,'vicon:script_lightning')
		case resultItem: 			
			dc.font('weight:bold')
			dc.text(rcText, text) 
			if(node.comment) {
				dc.textSize(text).inject(tw)
				dc.font('size:11px,color:#88a,weight:0').text(rcText.incrX(tw+6), node.comment )
			} else {
				dc.font('weight:0')
			}
			rcModify=null
			if(node.flag(NODE.modify)) {
				rcModify=rcText.rightCenter(24,0)
			}
			if(rcModify ) {
				dc.font('color:#f00').text(rcModify, '*', 'center')
			}
			if(node.note) {
				if(rcModify) {
					rcNote=rcModify.moveLeft(100,0)
				} else {
					rcNote=rcText.rightCenter(124,0)
				}
				dc.fill(rcNote, '#ffffffaa')
				dc.font('size:12px, elided:right, color:#a23').text(rcNote.incrY(-4), node.note)
			} else {
				if(rcModify) {
					rcNote=rcModify.moveLeft(24,0)
				} else {
					rcNote=rcText.rightCenter(24,0)
				}
				dc.image(rcNote.center(18,18), 'vicon:brick_edit')
			}
			node.rcNote=rcNote
			
		case resultValue:
			if(node.value) text.add("- ${node.value}")
			idx=node.parentNode().index()	
			if(idx.eq(0)) {
				rcCheck=rc.leftCenter(20,20,2)
				rcText= rc.incrX(24)
				if(node.flag(NODE.check)) {				
					dc.image(rcCheck, 'icons:check1')
				} else {
					dc.rectLine(rcCheck.center(16,16), 0, '#888', 2)
				}
				node.rcCheck=rcCheck
			}
			dc.text(rcText, text)
		default:
			dc.text(rcText, text)
		}
		dc.restore()
	}
	treeApiDoc(&result) {
		apiDoc=_node('apiDoc')
		apiDoc.parseJson(result)
		root=tree.rootNode()
		api=findTag(root,'api')
		while(url, apiDoc.paths.keys()) {
			cur=apiDoc.paths.get(url)
			ss=''
			if(cur.isVar('post')) {
				ss.add('<font color="blue">[POST]</font> ')
				comment=cur.post.summary
			} else {
				ss.add('<font color="red">[GET]</font> ')
				comment=cur.get.summary
			}
			ss.add(url)
			sub=api.addNode()
			sub.tag='apiItem'
			sub.text=ss
			sub.url=url
			sub.comment=comment
			sub.response="${cur.response}"
		}
		tree.expand(api, true, true)
		tree.update()

		comboData=_node('comboApi')
		while(key, apiDoc.paths.keys()) {
			cur=apiDoc.paths.get(key)
			if(cur.post) {
				method='POST'
				name=cur.post.summary
			} else {
				method='GET'
				name=cur.get.summary
			}
			url=key
			text="$method $name"
			comboData.addNode().with(url, method, text)
		}
		page('main').var(apiResultText, "\r\nAPI문서정보 >> $result")
	}
	treeApiResult(&result, first) {
		this.addArray('arrCheckParentNodes', true)
		dataNode=_node().parseJson(result)
		parent=findTag(tree.rootNode(),'result')
		not(parent) return;
		if(first) {
			templateResultItem(parent)
		}
		while(name, dataNode.keys()) {
			if(name.eq('code','message','status','error','path')) continue;
			c=findField(parent,'text',name)
			not(c) {
				c=parent.addNode()
				c.text=name
				c.comment=conf("resultItem.$name:comment")
				c.note=conf("resultItem.$name:note")
				c.tag='resultItem'
			}
			list=dataNode.get(name) 
			while(sub, list, row) {
				cc=c.addNode()
				cc.tag="resultRow"
				cc.text="ROW$row"
				cc.type=name
				while(key,sub.keys()) {
					ccc=cc.addNode()
					ccc.tag="resultValue"
					ccc.text=key
					ccc.value=sub.get(key)
				}
			}
		}
		tree.update()
		url=web.getUrl()
		page('main').var(apiResultText, "\r\n$url\r\n호출결과 >> $result")
	}
	treeMouseDown(pos) {
		this.mouseDownNode=tree.at(pos)
	}
	treeMouseUp(pos) {
		node=tree.at(pos) not(node) return;
		if( node!=this.mouseDownNode ) {
			return;
		}
		if(node.rcIcon.contains(pos)) {
			return;
		} 
		if( this.arrCheckParentNodes.find(node)) {
			this.currentMenuNode=node
			tree.current(node)
			page('main').showMenu('apiTreetMenu','apiTest,copyFields,makeTemplate', pos, this )
			return true;
		} 
		if(node.tag.eq('api','result') ) {
			tree.expand(node, true, true)
			return true;
		}
		if(node.tag.eq('resultItem') ) {
			this.currentMenuNode=node
			tree.current(node)
			page('main').showMenu('resultItem','addItemComment,makeTemplate', pos, this )
			return true;
		}
		if(node.rcBtn) {
			if( node.rcBtn.contains(pos) ) {
				dlg=page('main').get('apiInputDialog')
				dlg.size(1025,500)
				dlg.openDialog(this, node.url) 
				return true;
			}
		}
		if( node.rcCheck ) {
			if(node.rcCheck.contains(pos)) {
				arr=this.arrCheckParentNodes
				parent=node.parentNode()
				not( arr.find(parent) ) {
					arr.add(parent)
				}
				chk=when(node.flag(NODE.check), false, true)
				node.flag(NODE.check, chk)
				tree.update()
				return true;
			}
		}
	}
	treeMouseMove(pos) {
		node=tree.at(pos)
		not(node) return;
		not(node.rcBtn) return;
		if( node.rcBtn.contains(pos) ) {
			not(this.treeTooltipNode) {				
				this.tooltip(node.comment, true)
				this.treeTooltipNode=node
			}
			return;
		}
		if(this.treeTooltipNode ) {
			this.treeTooltipNode=null
			this.tooltip('', false)
		}
	}
	treeChange(node) {
		print("tree change => $node")
	}
	treeFilter(node) {
		val=this.filterValue not(val) return true
		node.inject(tag, text, comment) if(tag.ne('apiItem')) return true;
		if(comment ) {
			not(comment.find(val,2)) {
				not(text.find(val,2)) return false;
			}
			return true;
		} else {			
			not(text.find(val,2)) return false;
		}
		return true;		
	}
	filterTextChange() {
		this.var(filterChangeTick, System.tick())
	}
	click_btnApiRun(item) {
		dlg=page('main').get('apiInputDialog')
		dlg.size(1025,500)
		dlg.openDialog(this)
		this.update()
	}
	execAction(action) { 
		print("exec action start ", action)
		node=this.currentMenuNode
		not(node) return this.alert("액션메뉴 실행오류 - 선택된 데이터가 없습니다")
		switch(action.id) {
		case apiTest: 
			this.alert("API 실행창을 오픈 - 기능구현중")
		case copyFields:
			ss=''
			while(cur, node, n) {
				not(cur.flag(NODE.check)) continue;
				cur.inject(text, value)
				if(ss) ss.add(",\r\n");
				ss.add(Cf.jsValue(text),':',Cf.jsValue(value) )
			}
			System.copyText(ss)
			this.alert('선택된 항목을 복사했습니다')
		case makeTemplate:
			text=toString(node,true,true)
			not(node.template) node.template=conf("resultItem.${node.text}:template")
			page('main').var(apiResultText, "\r\n$text" ) 
			page('main').openPromptEdit(node, tree)
		case addItemComment:
			page('main').openComment(node)
		default:
		}
	}
}
class apiPage {
	initClass() { 
		@splitter=this.get('splitter')
		@editor=this.makeWidget('editor','apiContentEditor') class(editor,'editorSql') @template.editorKeyMap(editor)
		@form=this.makeWidget('apiContentForm')
		splitter.addPage(editor)
		splitter.addPage(form)
		splitter.stretchFactor(0)
		this.bgColor=randomColor() 
	}
	initPage() {
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'*,40'))
	}  
}
class apiContentForm {
	initClass() {
		this.setFormInfo('apiContentFormInfo')
	}
	
}

/* HTML템플릿 페이지 */
class htmlTemplateForm {
	initClass() { 
		@tree=this.makeWidget('tree','treeTemplate') class(tree,'tree')
		@input=this.makeWidget('input','inputTemplateFilter')
		tree.var(treeMode, false)
		tree.model('id')
		tree.setEvent('onDraw', this, this.treeDraw)
		tree.setEvent('onChange', this, this.treeChange)
		tree.setEvent('onMouseDown', this, this.treeMouseDown)
		tree.setEvent('onFilter', this, this.treeFilter)
		input.setEvent('onTextChange', this, this.filterTextChange)
		this.setFormInfo('htmlTemplateFormInfo')
	}
	initForm() { 
		this.timer(800)
	}
	onTimer() {
		if( this.firstCall ) {
			this.initData()
			this.firstCall=false
		}
		if( this.var(filterChangeTick)) {
			dist=System.tick() - this.var(filterChangeTick);
			if(dist>500) {
				this.filterValue=input.value()
				this.var(filterChangeTick, 0)
				tree.update() 
			}
		}	
	}
	initData() {
		root=tree.rootNode() 
		cur=root.addNode().with(tag:result, text:HTML 템플릿정보)
		templateResultItem(cur,true)		
		tree.expand(cur, true, true)
		page('main').addTemplateDiv(cur)
		tree.update()
	}
	treeDraw(dc, node, index, state, over) {
		rc=tree.drawSelect(dc, dc.rect(), node, col, state, over) 
		rcText=rc.incrX(4)
		node.rcIcon=rc.moveLeft(18,18,-2,0,true)
		text=node.text
		dc.save()
		switch(node.tag) { 
		case resultItem: 			
			dc.font('weight:bold')
			dc.text(rcText, text) 
			if(node.comment) {
				dc.textSize(text).inject(tw)
				dc.font('size:11px,color:#88a,weight:0').text(rcText.incrX(tw+6), node.comment )
			} else {
				dc.font('weight:0')
			}
			rcModify=null
			if(node.flag(NODE.modify)) {
				rcModify=rcText.rightCenter(50,0)
			} else if(node.flag(NODE.set)) {
				rcModify=rcText.rightCenter(24,0)
			}			
			if( rcModify ) {
				if(node.flag(NODE.modify)) {
					dc.font('color:#44d').text(rcModify, '[수정]', 'center')
				} else {
					dc.font('color:#f00').text(rcModify, '*', 'center')
				}
			}
			if(node.note) {
				if(rcModify) {
					rcNote=rcModify.moveLeft(100,0)
				} else {
					rcNote=rcText.rightCenter(124,0)
				}
				dc.fill(rcNote, '#ffffffaa')
				dc.font('size:12px, elided:right, color:#a23').text(rcNote.incrY(-4), node.note)
			} else {
				if(rcModify) {
					rcNote=rcModify.moveLeft(24,0)
				} else {
					rcNote=rcText.rightCenter(24,0)
				}
				dc.image(rcNote.center(18,18), 'vicon:brick_edit')
			}
			node.rcNote=rcNote 
		default:
			dc.text(rcText, text)
		}
		dc.restore()
	}
	treeChange(node) { 
		page('main').addTemplateForm(node, tree) 
	}
	treeMouseDown(pos) {

	}
	filterTextChange() {
		this.var(filterChangeTick, System.tick())
	}
	treeFilter(node) {
		val=this.filterValue
		not(val) return true;
		if(node.text.find(val,2)) return true;
		if(node.note.find(val,2)) return true;
		if(node.template.find(val,2)) return true;
		return false;
	}
}
class htmlTemplatePage {
	initClass() { 
		@splitter=this.get('splitter')
		@divPage=this.makePage('htmlTemplateDivPage','margin:0','div')
		// @editor=this.makeWidget('editor','htmlTemplateContentEditor') class(editor,'editorSql') @template.editorKeyMap(editor)
		@form=this.makeWidget('htmlTemplateContentForm')
		splitter.addPage(divPage)
		splitter.addPage(form)
		splitter.stretchFactor(0)
		this.bgColor=randomColor() 
	}
	initPage() {
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'*,34'))
	}  
}
class htmlTemplateContentForm {
	initClass() {
		this.setFormInfo('htmlTemplateContentFormInfo')
	}
}

/* DB테이블 조회 페이지 */
class dbTableForm {
	initClass() {
		@tree=this.makeWidget('tree','dbTableTree') class(tree,'tree')
		@combo=this.makeWidget('combo','dbTableFilterKind')
		@filter=this.makeWidget('input','dbTableFilter')
		@popup=this.makeWidget('popupTableInfo')
		tree.setEvent('onDraw', this, this.treeDraw)
		tree.setEvent('onMouseDown', this, this.treeMouseDown)
		tree.setEvent('onMouseMove', this, this.treeMouseMove)
		tree.setEvent('onFilter', this, this.treeFilter)
		tree.model('TABLE_NAME')
		tree.var(treeMode, false)		
		combo.setEvent('onChange', this, this.filterKindChange)
		filter.setEvent('onTextChange', this, this.filterTextChange)
		
		this.var(filterKind,'all')
		this.expandFlag=false
		this.setFormInfo('dbTableFormInfo')
	}
	initForm() {
		combo.addItem(this.conf('dbTableFilter',true), 'code,value')
		this.treeData() 
		this.timer(500)
		this.firstCall=false
	} 
	onTimer() {
		if( this.var(filterChangeTick)) {
			dist=System.tick() - this.var(filterChangeTick);
			if(dist>500) {
				cur=combo.current()
				this.filterValue=filter.value()
				this.filterKind=when(cur, cur.code, 'all')
				this.var(filterChangeTick, 0)
				tree.update() 
			}
		}
	}
	treeData() {
		root=tree.model()
		db=Baro.db('muk')
		db.fetchAll(this.conf('tableInfo'), root, true)
		while(cur, root) {
			cur.type='table' 
		}
		tree.update()
	}
	treeDraw(dc, node, index, state, over) { 
		rc=tree.drawSelect(dc, dc.rect(), node, col, state, over)
		node.rcIcon=rc.moveLeft(18,18,-2,0,true)
		btnIcon=''
		dc.save()
		switch(node.type) {
		case table:
			text= node.TABLE_NAME
			comment= node.TABLE_COMMENT
			dc.font('size:11,color:#445')
			btnIcon='vicon:database_table'
		case column:
			text = node.COLUMN_NAME
			comment=node.COLUMN_COMMENT
			dc.font('size:11,color:#99a')
		default:
		}
		dc.textSize(text).inject(tw, th)
		dist=rc.width() - tw;
		dc.text(rc, text)
		if(dist.gt(60)) {
			tw+=4;
			rcIcon=rc.incrX(tw).leftCenter(20,20) 
			if(btnIcon) {
				dc.rectLine(rcIcon,0,'#eee')
				dc.image(rcIcon.center(16,16), btnIcon)
				node.rcBtn=rcIcon
				rcIcon=rcIcon.moveRight(20,20,4)
			}
			dc.rectLine(rcIcon,0,'#eee')
			dc.image(rcIcon.center(16,16), "vicon:comment_edit")
			node.rcCopy=rcIcon
			if(comment) {
				tw+=20;
				rcComment=rc.incrXW(tw, 4) 
				dc.font(9,color('#966'))
				dc.text(rcComment, "- $comment", "right")
			}
		} 
		dc.restore()
	}
	treeMouseMove(pos) {
		node=tree.at(pos)
		if(node ) {
			node.inject(rcBtn, rcCopy)
			if(rcBtn.contains(pos) || rcCopy.contains(pos)) {
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
		node.inject(rcIcon, rcBtn, rcCopy)
		if(rcIcon.contains(pos)) return;
		if( this.rcOver ) {
			if( rcBtn.contains(pos) ) {
				this.popupTableInfo(node)
				return 'ignore'
			}
			if( rcCopy.contains(pos) ) {
				if(node.type.eq('table')) {
					text=node.TABLE_NAME
				} else {
					text=node.COLUMN_NAME
				}
				System.copyText(text)
				page('main').var(logMessage, "$text 가 복사되었습니다")
				return 'ignore'
			}
		}
		rc=tree.nodeRect(node)
		if(this.currentTreeNode!=node) {
			tree.current(node)
			tree.expand(node, true, true)
			this.currentTreeNode=node
			page('main').dbTableTreeChange(node)
		}
		return 'ignore';
	}
	treeFilter(node) {
		val=this.filterValue not(val) return true;
		switch(this.filterKind) {
		case all:
			not(node.TABLE_NAME.find(val,2)) {
				not(node.TABLE_COMMENT.find(val,2)) return false;
			}
		case table:
			not(node.TABLE_NAME.find(text,2)) return false; 
		case comment: 
			not(node.TABLE_COMMENT.find(text,2)) return false; 
		default:
			not(node.TABLE_NAME.find(text,2)) return false; 
		}
		return true;
	}	
	filterTextChange() { 
		this.var(filterChangeTick, System.tick())
	}	
	filterKindChange(idx) {
		this.var(filterChangeTick, System.tick())
		filter.focus()
	}
	popupTableInfo(node) {
		rc=tree.nodeRect(node)
		rcPopup=rc(rc.rb(),900,450)
		rcGlobal=tree.mapGlobal(rcPopup)
		main=page('main')
		popup.parentWidget(main)
		popup.flags('tool', true)
		popup.move(rcGlobal)
		popup.open()
		popup.active()
		popup.setTable(node)
	}
}

class dbTablePage {
	initClass() { 
		@splitter=this.get('splitter')
		@editor=this.makeWidget('editor','dbTableContentEditor') class(editor,'editorSql') @template.editorKeyMap(editor)
		@form=this.makeWidget('dbTableContentForm') 
		splitter.addPage(editor)
		splitter.addPage(form)
		splitter.stretchFactor(0)
		editor.setEvent('onKeyDown', this, this.editorKeydown)
	}
	initPage() {
		tot=splitter.sizes().sum()
		splitter.sizes(recalc(tot,'*,34'))
	}
	onKeyDown(k,a) {
		ctrl=a&KEY.ctrl;
		if(k.eq(81) && ctrl ) {
			main=page('main')
			combo=main.left.current().get('combo') not(combo) return;
			combo.showPopup()
			return true;
		} 
		return this.editorKeydown(k,a);
	} 
	editorKeydown(k,a) {
		ctrl=a&KEY.ctrl;
		if( ctrl && k.eq(KEY.Return, KEY.Enter) ) {
			this.runQuery()
			return true;
		}
	}
	setQuery(&query, skip) {
		db=Baro.db('muk')
		grid=form.grid
		if(skip) {
			if( query.find(';')) query=query.findPos(';')
		} else {
			editor.insertQuery(query)
		} 
		hh=splitter.sizes().get(1)
		if(hh<100) {
			tot=splitter.sizes().sum()
			splitter.sizes(recalc(tot,'*,300'))
		} 
		root=grid.rootNode().removeAll()
		db.fetchAll(query, root, true)
		ss='', num=0
		while(field, root.var(fields), num) {
			if(num) ss.add(',')
			ss.add(field)
		}
		grid.fields(ss) 
		if( form.isCheck('CheckSameSize') ) {
			grid.fullWidth(true)
		} else {
			grid.fullWidth('resizeToContent')
		}
		grid.selectClear()
		grid.update()
		form.update()
	}
	runQuery() {
		if( editor.is('select')) {
			query=editor.text('select')
			if( query.size()>5 ) {
				return this.setQuery(query, true)
			}
			pos=editor.pos('selectStart')
			editor.pos(pos)
		}
		pos=editor.pos()
		val=editor.text(pos-1, pos)
		if( val.eq(';')) {
			editor.pos(pos-1)
		}
		if( editor.searchPrev(';',0) ) {
			sp=editor.pos()
		} else {
			sp=0
		}
		editor.pos(sp+1)
		if( editor.searchNext(';',0) ) {
			ep=editor.pos() - 1;
		} else {
			ep=editor.pos('end')
		}
		if(sp<ep) {
			query=editor.text(sp,ep);
			this.setQuery(query, true); 
			if( form.isCheck('CheckRunSelect')) {
				editor.select(sp,ep)
			}
		}
	} 
	runSelect(select) {
		if(editor.is('select')) {
			pos=editor.pos('selectStart')
			editor.pos(pos)
			return;
		}
		not(select) return;
		pos=editor.pos()
		val=editor.text(pos-1, pos)
		if( val.eq(';')) {
			editor.pos(pos-1)
		}
		if( editor.searchPrev(';',0) ) {
			sp=editor.pos()
		} else {
			sp=0
		}
		editor.pos(sp+1)
		if( editor.searchNext(';',0) ) {
			ep=editor.pos() - 1;
		} else {
			ep=editor.pos('end')
		}
		if(sp<ep) {
			editor.select(sp,ep)
		}
	}
	
}
class dbTableContentForm {
	initClass() {
		@grid=this.makeWidget('grid', 'gridTableDetail')  class(grid,'grid')
		grid.model('text')
		grid.var(bgColor, color('#3D718BDD'))
		grid.setEvent('onDraw',this, this.drawGrid) 
		this.setFormInfo('dbTableContentFormInfo')
	}  
	drawGrid(dc, node, index, state) {
		field=grid.field(index)
		rc=grid.drawState(dc, node, state, index, field );
		text=node.get(field) 
		dc.text(rc,text);
	}
	drawDetail(dc,rc) {
		node=this.currentLogNode not(node) return;
		dc.save().font('size:11, weight:bold')
		dc.pen('#5566aacc').text(node.rect.incr(10,2), node.logText)
		dc.restore()
	}
	setLogText(text) {
		node=this.findTag(1,'space')
		if(text) {
			node.logText=text
			this.currentLogNode=node
			this.update()
		} else {
			this.currentLogNode=null
		}
	}
	click_RunQuery() {
		page().runQuery();
	}
	click_CheckRunSelect(item) {
		page().runSelect(item.checked)
	}
	click_CheckSameSize(item) { 
		not(grid) return;
		if( item.checked ) {
			grid.fullWidth(true)
		} else {
			grid.fullWidth('resizeToContent')
		}
	}
}

/* 테이블 컬럼정보 조회 */
class popupTableInfo {
	initClass() { 
		@grid=this.makeWidget('grid', 'gridTableInfo') class(grid, 'grid')
		fields=[
			{ field:chk, text:선택, width:40}
			{ field:column_name, text: 컬럼명, width:200}
			{ field:column_type, text: 데이터타입, width:100}
			{ field:column_comment, text: 설명, width:200}
			{ field:is_nullable, text: NULL여부, width:80}
			{ field:column_default, text: 기본값, width:80}
			{ field:column_key, text: 키정보, width:80}
			{ field:ordinal_position, text: 정렬}
		]
		input=grid.setInput()
		grid.model(fields)
		grid.is('stretchLast', true)
		grid.is('sortEnable', true)
		grid.var(bgColor, color('#528B3DE0'))
		grid.setEvent('onDraw', this, this.gridDraw)
		grid.setEvent('onMouseDown', this, this.gridMouseDown)
		grid.setEvent('onMouseWheel', this, this.gridMouseWheel)
		this.setFormInfo('popupTableForm')
		input.setEvent('onKeyDown', this, this.keydown)
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
			grid.inputHide()
			return; 
		}
		field=node.var(code)
		if(field.eq('chk')) {
			chk=when(node.flag(NODE.check), false, true)
			node.flag(NODE.check, chk)
			grid.update()
		} else if(field.eq('column_comment') ) {
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
		db=Baro.db('muk')
		root=grid.rootNode().removeAll()
		root.TABLE_NAME=node.TABLE_NAME
		db.fetchAll(this.conf("columnDetail"), root, true)
		this.title("테이블: ${node.TABLE_NAME} 컬럼수: ${root.childCount()}")
		print("popup set table node==>$node, root:$root")
		grid.update()
	}	 
	click_AddField() {
		field=grid.fields().child(1).get('field')
		root=grid.rootNode()
		cur=root.addNode()
		cur.flag(NODE.add, true)
		grid.update()
		grid.current(cur)
		grid.edit(cur, field)
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
		table=root.TABLE_NAME
		ss='', num=0
		while(cur, root) { 
			if(cur.flag(NODE.check) ) { 
				if(ss) ss.add(', ')
				ss.add(cur.column_name)
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
				muknoori.${table}
			WHERE 1=1
		];
		print("make query : ", sql)
		page('main').var(strEditQuery, sql)
		this.hide()
	}

}

/* 다국어처리 페이지 */
class langMngPage {
	initClass() {
		@db=Baro.db('muk')
		@form=this.getWidget('form')
		@grid=this.makeWidget('grid', 'gridTest') class(grid, 'grid') 
		input = grid.setInput()
		input.setEvent('onKeyDown', this, this.keydown)
		grid.setEvent('onDraw', this, this.gridDraw)
		grid.setEvent('onFilter', this, this.gridFilter)
		grid.setEvent('onMouseDown', this, this.gridMouseDown)
		grid.setEvent('onMouseWheel', this, this.gridMouseWheel)
		
		@comboUseYn=this.makeWidget('combo','langUseYn' )
		@comboKind=this.makeWidget('combo','langFilterKind' )
		@filter=this.makeWidget('input','langFilter' )

		grid.model(#[
			chk:선택 #40px,
			status:상태 #60px,
			APP_LANG_KEY 	: 다국어키 	#200,
			APP_LANG_NM_KR	: 한글		#200,
			APP_LANG_NM_EN	: 영문		#200,
			APP_LANG_NM_CH	: 중문		#200,
			APP_LANG_NM_JP	: 일문		#200,
			USE_YN			: 사용여부	#80,
			ADD_DT			: 등록일시	#100
		])
		@allFields=splitArray('APP_LANG_KEY,APP_LANG_NM_KR,APP_LANG_NM_EN', 'langFilter') 
		comboUseYn.addItem(@data.useYn(), 'code,value') 
		comboUseYn.setEvent('onChange', this, this.comboUseYnChange)
		comboKind.addItem(_node('data.langFilterKind').with([
			{code:all,value:전체}
			{code:langKey,value:키조회}
			{code:langKr,value:한글조회}
		]), 'code,value')
		comboKind.current(1)
		comboKind.setEvent('onChange', this, this.comboFilterChange)
		filter.setEvent('onTextChange', this, this.inputFilterChange)
		this.setEvent('onMouseDown', func() { grid.inputHide() })
		this.filterKind='langKey'
		extendFunc(form,'formInfo')
		form.setFormInfo("langMngForm", this)
	}

	initPage() {
		this.gridData()
		this.timer(500)
	}
	onTimer() {
		if( this.firstCall ) {
			grid.fullWidth()
			this.firstCall=false
		}
		if( grid.var(editStartTick)) {
			grid.var(editStartTick,0)
			grid.inputFocus()
		}
		if( this.var(filterChangeTick)) {
			dist=System.tick() - this.var(filterChangeTick);
			if(dist>250) {
				this.var(filterChangeTick,0)
				grid.update()
			}
		}
	} 
	inputFilterChange() {
		this.var(filterChangeTick, System.tick())
	}
	comboUseYnChange() {
		cur=comboUseYn.current()
		this.filterUseYn=when(cur,cur.code)
		grid.update()
	}
	comboFilterChange() {
		cur=comboKind.current()
		this.filterKind=when(cur,cur.code)
		grid.update()		
	}
	keydown(k,a,b) {
		if(k.eq(KEY.Escape)) {
			return grid.inputHide();
		}
		if(k.eq(KEY.Enter, KEY.Return, KEY.Tab)  ) {
			node=grid.var(editNode)
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
		}
	}
	gridDraw(dc, node, index, state) {
		field=grid.field(index)
		rc=grid.drawState(dc, node, state, index, field )
		this.gridDrawNode(dc, rc, node, field)
	}
	gridDrawNode(dc, rc, node, field) { 
		if(field.eq('chk')) {
			if(node.flag(NODE.check)) {				
				dc.image(rc.center(20,20), 'icons:check1')
			} else {
				dc.rectLine(rc.center(16,16), 0, '#888', 2)
			} 
		} else if(field.eq('status')) {
			if(node.flag(NODE.add)) {
				dc.pen('#33e').text(rc,'신규','center')
			} else if(node.flag(NODE.modify)) {
				dc.pen('#d3e').text(rc,'수정','center')
			}		
		} else {
			dc.text(rc.incrX(2), node.get(field))
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
		if(field.eq('status')) return;
		if(field.eq('chk')) {
			chk=when(node.flag(NODE.check), false, true)
			node.flag(NODE.check, chk)
			grid.inputHide()
			grid.update()
		} else {
			grid.edit(node, field)
		}
	}
	gridFilter(node) {
		if(node.flag(NODE.add)) {
			return true;
		}
		text=filter.value()		
		not(text) {
			if(this.filterUseYn) {
				return node.cmp('USE_YN',this.filterUseYn);
			}
			return true;
		}
		if(this.filterUseYn) {
			ok=node.cmp('USE_YN',this.filterUseYn)
			not(ok) return false;
		}
		kind=this.filterKind
		not(kind) kind='all'
		switch(kind) {
		case all:
			while(field, allFields) {
				if(node.get(field).find(text,2)) return true;
			}
		case langKey:
			if(node.get('APP_LANG_KEY').find(text,2) ) return true;
		case langKr:
			if(node.get('APP_LANG_NM_KR').find(text,2) ) return true;
		default:
		}
		return false;
	}
	gridData() {
		root=grid.rootNode().removeAll()
		grid.inputHide()
		grid.selectClear()
		db.fetchAll(this.conf('langSelect'), root)
		grid.update()
		grid.fullWidth()
	}
	/* 버튼클릭 이벤트 처리 */
	click_LangAdd() {
		field=grid.fields().child(2).get('field')
		root=grid.rootNode()
		cur=root.addNode()
		cur.flag(NODE.add, true)
		grid.update()
		grid.current(cur)
		grid.edit(cur,field)
	}
	click_LangApply() {
		Cf.error(true)
		not(this.confirm("변경내용을 적용 하시겠습니까?")) return;
		cntInsert=0, cntModify=0
		root=grid.rootNode()
		while(cur, root) {
			if(cur.flag(NODE.add)) {
				db.exec(this.conf('langInsert'),cur)
				cntInsert++;
			} else if(cur.flag(NODE.modify)) {
				db.exec(this.conf('langModify'),cur)
				cntModify++;
			}
			err=Cf.error()
			if(err) return this.alert("저장중 오류가 발생했습니다 오류:$err")
		}
		cnt=cntInsert+cntModify;
		if(cnt) {
			this.gridData()
		} else {
			this.alert("변경된 내용이 없습니다")
		}
	}
	click_LangDelete() {
		not(this.confirm("선택내용을 삭제 하시겠습니까?")) return;
		cnt=0
		arr=_arr()
		root=grid.rootNode()
		while(cur, root) {
			if(cur.flag(NODE.check)) {
				not(cur.flag(NODE.add)) {
					db.exec(this.conf('langDelete'),cur)
				}
				arr.add(cur)
				cnt++;
			}
		}
		while(cur, arr.revers()) {
			root.remove(cur)
		}
		if(cnt) {
			this.gridData()
		} else {
			this.alert("변경된 내용이 없습니다")
		}
	} 
	click_Reload() {
		this.gridData()
	}
}

class conf:menu {
	topMenu: {
		actionList: [
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
			{id:apiTest, text:API매개변수생성, icon:'vicon:add_default'}
			{id:copyFields, text:선택복사, icon:'vicon:application_edit', tooltip:선택된 필드 클립보드에 복사}
			{id:makeTemplate, text:템플릿만들기, icon:'vicon:application_form_add', tooltip:지정된 폴더에 템플릿생성}				
			{id:addItemComment, text:커멘트달기, icon:'ficon:balloon-quotation', tooltip:'API결과항목 커멘트달기'}
		]
		menuTabs:[
			{actionId:menu.apiForm, useLeftPanel:true }
			{actionId:menu.htmlTemplateForm, useLeftPanel:true }
			{actionId:menu.dbTableForm, useLeftPanel:true }
			{actionId:menu.langMngPage }
		]
		menus:[
			{actionId:menu.apiForm }
			{actionId:menu.htmlTemplateForm }
			{actionId:menu.dbTableForm }
			{actionId:menu.langMngPage }
			{actionId:sourceEditor }
			{actionId:locknlock }
			{type:separator }
			{text:설정관리, menus:[
				{actionId:menu.codeMng }
				{actionId:menu.appMenuMng }
				{actionId:menu.dbConnectMng }
			]}
			{type:separator }
			{actionId:menu.close }
		]
	}

}
class conf:form {
	apiFormInfo: { rows: {
		vbox:'30,*,34', rowMargin:[4,2,4,6]
		{
			{tag:label, text:'API호출 테스트', style:'size=12,weight=bold', width:200}
			{tag:space}
			{tag:btn, id:btnApiRun, icon:'vicon:script_lightning', tip:API실행창 열기 }
		}, {
			margin:2, widget:@tree
		}, { 
			{tag:label, text:필터 }
			{tag:input, widget:@input, width:150}
			{tag:space}
		}
	}}

	dbTableFormInfo: { rows: {
		vbox:'30,*,34', rowMargin:[4,2,4,6] 
		{
			{tag:label, text:'DB테이블 정보'}
		}, {
			widget:@tree, margin:2
		}, { 
			{tag:combo, widget:@combo, width:80 }
			{tag:input, widget:@filter, width:150}
			{tag:space}
			{tag:btn, id:btnTableInfo, icon:'vicon:database_table', tip:테이블정보 보기, margin:2 }
			{tag:btn, id:btnFoldToggle, icon:'icons:arrow_down', tip:테이블 펼치기/접기, margin:2 }
		}
	}}

	htmlTemplateFormInfo: { rows: {
		vbox:'30,*,34', rowMargin:[4,2,4,6]
		{
			{tag:label, text:'화면 HTML 템플릿 생성'}
		}, {
			widget:@tree, margin:2
		}, { 
			{tag:label, text:필터 }
			{widget:@input, width:150}
			{tag:space}
		}
	}}

	popupTableForm: { rows: {
		vbox:'*,34', rowMargin:[4,2,4,6], cellMargin:[4,2]
		{
			widget:@grid, margin:2
		}, {		 
			{tag:btn, id:AddField, text:필드추가}
			{tag:btn, id:MoveDown, text:아래로}
			{tag:btn, id:MoveUp, text:위로}
			{tag:btn, id:MakeQuery, text:선택 쿼리작성}
			{tag:space}
			{tag:btn, id:DeleteRow, text:삭제}
			{tag:btn, id:NewTable, text:새테이블 만들기}
		}
	}}

	apiContentFormInfo: { rows: {
		vbox:'*,34', rowMargin:[4,2,4,6], cellMargin:[4,2]
		{
			widget:@grid, margin:2
		}, {		  
			{tag:btn, id:EditorClear, text:초기화, icon:'vicon:comment_delete'}
			{tag:space}
		}
	}}

	htmlTemplateContentFormInfo: { rows: {
		vbox:'*,34', rowMargin:[4,2,4,6], cellMargin:[4,2] 
		{
			widget:@grid, margin:2
		}, {		  
			{tag:btn, id:EditorClear, text:초기화, icon:'vicon:comment_delete'}
			{tag:space}
		}
	}}

	dbTableContentFormInfo: { rows: {
		vbox:'*,34', rowMargin:[4,2,4,6], cellMargin:[4,2]
		{
			widget:@grid, margin:2
		}, {		  
			{tag:btn, id:RunQuery, text:쿼리실행, icon:'vicon:database_table'}
			{tag:check, id:CheckRunSelect, text:쿼리선택, tip:조회후 쿼리선택  }
			{tag:check, id:CheckSameSize, text:같은폭, tip:그리드 필드를 같은폭으로 맞추기 }
			{tag:space}			
		}
	}}

	langMngForm: { rows: {
		vbox:'*,34', rowMargin:[4,2,4,6], cellMargin:[4,2]
		{
			widget:@grid, margin:2
		}, {		 
			{tag:label, text:필터}
			{tag:input, widget:@filter, width:150 }
			{tag:btn, id:LangAdd, text:신규}
			{tag:btn, id:LangApply, text:적용}
			{tag:btn, id:LangDelete, text:삭제}
			{tag:space}
			{tag:combo, widget:@comboUseYn, width:60}
			{tag:combo, widget:@comboKind, width:120}
			{tag:btn, id:Reload, text:새로고침}
		}
	}}

	dbTableFilter: {
		{ code:all, 	value:전체 }
		{ code:table, 	value:테이블명 }
		{ code:comment,	value:설명 }
		{ code:column, 	value:컬럼명 }
	}
}

class conf:sql {
	tableInfo: <sql>
		SELECT TABLE_NAME, TABLE_COMMENT, CREATE_TIME
		FROM
			INFORMATION_SCHEMA.TABLES
		WHERE
			TABLE_SCHEMA = 'muknoori' and TABLE_TYPE ='BASE TABLE'
	</sql>

	columnInfo: <sql>
		SELECT
			TABLE_NAME, COLUMN_NAME, COLUMN_COMMENT
		FROM
			INFORMATION_SCHEMA.COLUMNS
		WHERE
			TABLE_SCHEMA = 'muknoori' AND TABLE_NAME = #{TABLE_NAME}    
	</sql>
	columnDetail: <sql>
		SELECT
			column_name
			, column_comment
			, ordinal_position
			, column_type
			, is_nullable
			, column_default
			, column_key
		FROM
			information_schema.columns
		WHERE
			table_schema = 'muknoori' AND table_name =#{TABLE_NAME} 
	</sql>
	
	langSelect: <sql>
		select 
			APP_LANG_KEY,
			APP_LANG_NM_KR,
			APP_LANG_NM_EN,
			APP_LANG_NM_CH,
			APP_LANG_NM_JP,
			USE_YN,
			ADD_DT
		from muknoori.muk_app_lang
	</sql>

	langInsert: <sql>
		insert into muknoori.muk_app_lang (
			APP_LANG_KEY,
			APP_LANG_NM_KR,
			APP_LANG_NM_EN,
			APP_LANG_NM_CH,
			APP_LANG_NM_JP,
			USE_YN
		) values (
			#{APP_LANG_KEY},
			#{APP_LANG_NM_KR},
			#{APP_LANG_NM_EN},
			#{APP_LANG_NM_CH},
			#{APP_LANG_NM_JP},
			#{USE_YN}
		)
	</sql>

	langModify: <sql>
		update muknoori.muk_app_lang set 
			APP_LANG_NM_KR = #{APP_LANG_NM_KR},
			APP_LANG_NM_EN = #{APP_LANG_NM_EN},
			APP_LANG_NM_CH = #{APP_LANG_NM_CH},
			APP_LANG_NM_JP = #{APP_LANG_NM_JP},
			USE_YN = #{USE_YN}
		where APP_LANG_KEY=#{APP_LANG_KEY}
	</sql>

	langDelete: <sql>
		delete from muknoori.muk_app_lang where APP_LANG_KEY=#{APP_LANG_KEY}
	</sql>
}

class layout {
	<page id="main" title="모꼬지 API테스트 페이지" margin="0"> 
		<canvas id="topMenu" height="28">
		<splitter type="hbox">
	</page>
	
	<page id="content" title="상세페이지" margin="0,0,4,0">
		<div id="div">
	</page>

	<page id="langMngPage" title="다국어관리 기능구현" margin="0">
		<canvas id="form">
	</page>	

	<page id="apiPage" margin="0">
		<splitter type="vbox">
	</page>

	<page id="htmlTemplatePage" margin="0">
		<splitter type="vbox">
	</page>

	<page id="dbTablePage" margin="0">
		<splitter type="vbox">
	</page>

	<dialog id="apiInputDialog" title="API전송 페이지">
		<editor id="data">
		<hbox>
			<combo id="comboApi" width="200">
			<combo id="comboMethod">
			<input id="inputApiAddr" stretch="1">
			<button id="btnSend" text="전송">
			<button id="btnClose" text="닫기" onClick() { page().close() }>
		</hbox>
	</dialog>
}
