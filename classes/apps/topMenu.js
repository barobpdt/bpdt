/* 메뉴관리 */
class topMenu {
    /* 
    [사용예]
        - tabItems 화면갱신시 자동생성
        - menuTabs 메뉴변경시 탭추가

    1. mainPage 페이지에 아래 함수 등록
        onAction(action) { topMenu.changeMenu(action.id) } 
        changeAction(action, tab) { this.changeMenuForm(actionId, tab) }
    */
	setMenuInfo(mainPage, menuInfo, height) {
        not(typeof(menuInfo,'node') ) return print("상위메뉴(topMenu) 메뉴정보 노드오류")
		@mainPage=mainPage
		@menuInfo=menuInfo
        not(menuInfo.menuTabs) menuInfo.addNode("menuTabs")
		not(height) height=28
		this.setEvent('onDraw', this.drawLayout)
		this.setEvent('onMouseDown', this.mouseDown)
		this.setEvent('onMouseUp', this.mouseUp)
		this.setEvent('onMouseMove', this.mouseMove)
		this.fixedHeight(height)
	}
	findAction(id) {
		while(a, menuInfo.actionList ) {
			if(a.cmp('id',id)) return a;
		}
		return;
	}
	addAction(param) {

    }    
	drawLayout(dc,rc) {
		not(typeof(sender,'widget')) {
			print("draw layout 오류 sender 객체오류")
			return;
		}
		if( rc.eq(sender.rect()) ) {
			not( rc.eq(this.rcBase) ) {
				this.rcBase=rc
                this.updateMenu(dc, rc)
			}
		}
		this.drawMenu(dc,rc)
	}
   
	updateMenu(dc, rc) {
		tabItems=this.addArray('@tabItems',true);
		dc.textSize('메뉴').inject(tw)
		tabItems.add(tw+38) 
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
			if(n.eq(0)) this.rcMenu=rc else tabItems.get(n).set('rect',rc)
		} 
		this.setFormRect()
	}
 	drawMenu(dc,rc) { 
		bgColor=this.bgColor
		not(bgColor) bgColor=color('#D4D4B7A0')
		dc.fill(bgColor)
		arr=this.var(tabItems)
		not(typeof(arr,'array')) {
			dc.text(rc, "메뉴 아이템정보가 없습니다")
			return;
		} 
		while(tab, arr, n) {
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
		arr=this.var(tabItems)
		while(tab, arr, n) {
			if(n.eq(0)) {
				actionId='menu', rect=this.rcMenu;
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
			not(mainPage) {
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
		arr=this.var(tabItems)
		while(tab, arr, n) {
			if(tab==this.currentMenuTab ) continue;
			if(n.eq(0)) {
				id='menu', rect=this.rcMenu;
			} else {
				tab.inject(id, rect)
			}
			if(rect.contains(pos)) {
				eq=eq(rect,this.overRect)
				not(eq) {
					mainPage.var(menuOverTick, System.tick())
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
			mainPage.var(menuOverTick, null)
		}
	}	
	showMenu() {
		lb=this.rcMenu.lb()
		this.menu(mainPage, menuInfo.menus, lb)
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
        mainPage.changeAction(action, tab)
		return true;
	}
}
