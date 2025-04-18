/* 메뉴관리 */
<script module="@topMenu">
    /* 
    [사용예]
        - tabItems 화면갱신시 자동생성
        - menuTabs 메뉴변경시 탭추가

    1. mainPage 페이지에 아래 함수 등록
        onAction(action) { topMenu.changeMenu(action.id) } 
        changeAction(action, tab) { this.changeMenuForm(actionId, tab) }
    */
	init(target, menu, height) {
		@tabItems = []
		if(typeof(menu,'node') ) {
			this.initMenu(target, menu, height)
		}
	}
	initMenu(target, menu, height) {
		@targetPage = target
		@menuInfo = menu
		@tabItems = []
		not(menu.actionList) menu.addArray("actionList")
        not(menu.menuTabs) menu.addNode("menuTabs")
		not(height) height=28
		setEvent('onDraw', this.drawMenuInfo)
		setEvent('onMouseDown', this.mouseDown)
		setEvent('onMouseUp', this.mouseUp)
		setEvent('onMouseMove', this.mouseMove)		
		this.fixedHeight(height)
	}
	findAction(id) {
		while(a, menuInfo.actionList ) {
			if(a.cmp('id',id)) return a;
		}
		return;
	} 
	drawMenuInfo(dc,rc) {
		if( rc.eq(this.rect()) ) {
			not( rc.eq(this.rcBase) ) {
				this.rcBase=rc
                this.updateMenu(dc, rc)
			}
		}
		this.drawMenu(dc,rc)
	}
   
	updateMenu(dc, rc) {
		dc.textSize('메뉴').inject(tw)
		tabItems.reuse().add(tw+38) 
		while(tab, menuInfo.menuTabs) {
			if(tab.actionId) {
				not(tab.action) {
					tab.action=this.findAction(tab.actionId)
					tab.action.inject(text, menuText)
					not(menuText) menuText=text
					tab.menuText=menuText
				}
				tab.inject(id, menuText, action, useClose)
				dc.textSize(menuText).inject(tw)
				if(action.icon) tw+=25;
				if(useClose) tw+=22;
				tab.tabWidth=tw+20;
			} else if(tab.widget) {
				tab.tabWidth=tab.width
			} else if(tab.cmp('type','label') ) {
				dc.textSize(tab.text).inject(tw)
				tab.tabWidth=tw+10;
			} else {
				continue;
			}
			tabItems.add(tab)
		}
		info=''
		while(a,tabItems,n) {
			if(n) info.add(',')
			if(typeof(a,'num')) info.add(a) else info.add(a.tabWidth)
		}
		info.add(',*')
		arr=hbox(rc,info)
		while(rc, arr, n) {
			if(n.eq(0)) this.rcMenu=rc 
			else tabItems.get(n).set('rect',rc)
		} 
	}
 	drawMenu(dc,rc) { 
		bgColor=this.bgColor
		not(bgColor) bgColor=color('#D4D4B7A0')
		dc.fill(bgColor) 
		while(tab, tabItems, n) {
			if(n.eq(0)) {
				rect=this.rcMenu
				fg=bgColor.lightColor(200)
				if( rect.eq(this.overRect)) {
					bg=bgColor.darkColor(120)
				} else {
					bg=bgColor.darkColor(180)
				}
				rcIcon=rect.leftCenter(16,16,4,2)
				rcText=rect.incrX(25)
				dc.fill(rect.incr(1), bg)
				dc.image(rcIcon, 'icons:menu1')
				dc.pen(fg).text(rcText, '메뉴')
				dc.rectLine(rect, 3, bgColor.darkColor(50), 2)
				dc.rectLine(rect.incrW(1), 3, bgColor.lightColor(80))
				continue;
			}
			tab.inject(rect, menuText, action, useClose)
			fg=bgColor.darkColor(200)
			rcText=rect.margin(1,2,2,0)
			dc.save()
			if( tab==this.currentMenuTab ) {
				dc.fill(rect, '#fff')
				dc.rectLine(rcText, 23, '#404030aa', 2 )
				dc.font('weight:bold')
			} else if( rect.eq(this.overRect)) {
				bg=bgColor.darkColor(80)
				dc.fill(rect, bg)
				dc.rectLine(rect, 23, bg.darkColor(120), 2 )
			} else {
				bg=bgColor
				dc.fill(rect, bg)
				dc.rectLine(rcText, 23, bg.darkColor(120) )
			}
			if(action.icon) {
				dc.image(rcText.leftCenter(16,16,4), action.icon)
				rcText.incrX(24)
			}
			dc.pen(fg).text(rcText, menuText)
			if(useClose) {
				rcClose=rect.rightCenter(16,16,-8)
				dc.fill(rcClose.incr(-2), '#ee604055')
				dc.image(rcClose, 'icons:close_gray')
			}
			dc.restore()
		}
	}
	mouseDown(pos) {
		while(tab, tabItems, n) {
			if(n.eq(0)) {
				actionId='menu' 
				rect=this.rcMenu;
			} else {
				tab.inject(actionId, rect)
			}
			if(rect.contains(pos)) {
				this.downId=actionId
				this.downRect=rect
				return true;
			}
		}
	}
	mouseUp(pos) {
		not(this.downRect) return;
		if( this.downRect.contains(pos)) {
			id=this.downId
			not(targetPage) {
				alert("메뉴 $id 클릭 메인페이지를 등록하세요")
			} 
			if( id.eq('menu')) {
				this.showMenu()
			} else {
				this.changeMenu(id)
			}
			this.downRect=null
			this.update()
		}
	}
	mouseMove(pos) {
		while(tab, tabItems, n) {
			if(tab==this.currentMenuTab ) continue;
			if(n.eq(0)) {
				id='menu'
				rect=this.rcMenu;
			} else {
				tab.inject(id, rect)
			}
			if(rect.contains(pos)) {
				eq=eq(rect,this.overRect)
				not(eq) {
					targetPage.var(menuOverTick, System.tick())
					this.overRect=rect
					this.cursor(CURSOR.PointingHandCursor)
					this.update()
				}
				return true;
			}
		}
		if( this.overRect ) {
			this.overRect=null
			this.cursor(0)
			this.update()
		}
	}
	mouseOverCheck(pos) {
		not(pos) pos=this.cursorPos()
		rc=this.mapGlobal(this.rect()) 
		not( rc.contains(pos) ) { 
			this.overRect=null
			this.cursor(0)
			this.update()
			targetPage.var(menuOverTick, null)
		}
	}	
	showMenu() {
		lb=this.rcMenu.lb()
		this.menu(targetPage, menuInfo.menus, lb)
	}
	makeMenu(menuCode, listData, reload) {
		menus=menuInfo.get(menuCode)
		if( typeof(menus,'node')) {
			not(reload) return menus;
			menus.removeAll(true)
		} else {
			menus=menuInfo.addNode(menuCode)
		}
		if(listData.ch('[')) {
			s=listData.match(1)
			return parse(menus, s)
		} else {
			return parse(menus,listData)
		}
		// menus=a,b,c[aa,bb,cc[dd,ee]],d...
		parse = func(node,&s) {
			while(s.valid()) {
				c=s.ch()
				not(c) break;
				if(subCheck(s)) {
					menuText=s.findPos('[').trim()
					cur=node.addNode().with(menuText)
					parse(cur, s.match(1) )
				} else {
					actionId=s.findPos(',').trim()
					if(actionId.eq('-')) {
						node.addNode().with(type:separator)
					} else {
						node.addNode().with(actionId)
					}
				}
			} 
			return node;
		};
		subCheck = func(&s) {
			left=s.findPos(',')
			if(left.find('[')) return true;
			return false;
		};
		return this.setMenuAction(menus)
	}

	changeMenu(id) {
		action=this.findAction(id)
        not(action ) {
            alert("메뉴변경 오류 - $id 메뉴액션를 찾을수 없습니다")
			return false;
        }
        tab=findField(menuInfo.menuTabs,'actionId',id)
        not(tab) {
            tab=menuInfo.menuTabs.addNode()
            tab.actionId=id
            tab.action=action
			tab.menuText=action.text
			tab.useClose=true;
            this.rcBase=null
            this.update()
        }
		this.currentMenuTab=tab
		if( typeof(targetPage.changeAction,'func') ) {
			targetPage.changeAction(action, tab)
			return true;
		}
		return false;
	}
}

</script>


<script>
getTestMenu() {
	menu = {
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
	return menu
}
</script>


<func>
/*
## drag & drop 처리
g.is('drag', true)
g.is('drop', true)
g[
	onDrag(type,data) {
		print("drag $data =>",type )
		return 'accept'
	}
	onDrop(a,b) {
		print("drop=>",a,b)
	}
	onDragMove(pos, data) {
		print("move=> $data", pos)
		return 'accept'
	}
]

*/
	@menu.makeMenu() {
		db=Baro.db('ggs')
		not(db.open()) db.open('D:\APP\ggs_local\data\config.db')
		menus=db.fetchAll(#[
			SELECT seq, code as id, pcode as pid, value as text, depth, url, icon, ref1, ref5 as type, sort 
			FROM comm_tree WHERE  ref='MN' and depth>0 
			ORDER BY depth, sort, pcode, seq
		], true)
		this.menuTree(menus)
		this.var(menuText, this.makeMenuText(this))
		this.var(toolbarText, this.makeToolbar(menus))
		return this;
	}
	@menu.makeTree(menus ) {
		root=this
		root.removeAll()
		while(cur, menus) {
			parent=root.findOne('id',cur.pid)
			not(parent) parent=root
			parent.addNode(cur,true)
		}
		return root;
	}
	@menu.makeMenuText(node, depth) {
		not(depth) dept=0
		ss='';
		text=node.text
		if(text.eq('-')) {
			ss.add('-,')
			return s;
		}
		pid=node.pid
		print("pid==$pid", text, pid)
		
		if( pid.eq('ROOT') ) {
			ss.add("{id:ROOT, ")
		} else {
			node.inject(id, text,icon)
			ss.add("{id: $id, text:$text, icon:$icon, ")
		}
		if( node.size()) {
			ss.add("type:menu, actions:[")
			while(sub, node) {
				ss.add( this.makeMenuText(sub, depth+1) )
			}
			ss.add("]}")
		} else {
			ss.add("}")
		}
		if(depth) ss.add(",")
		return ss
	}
	@menu.makeToolbar(menus) {
		ss=''
		prev=''
		while(cur, menus) {
			not(cur.ref1.eq('tool')) continue
			not( prev.eq(cur.pid) ) {
				if(prev) ss.add("-,")
				prev=cur.id
			}
			ss.add("${cur.pid}.${cur.id},")
		}
		return ss;
	}
</func>