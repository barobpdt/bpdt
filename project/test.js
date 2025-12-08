##> test {
	
}
container
	menuBar
	mainContent
		sidePanel {
			width: 280px;
			border-right: 1px solid #dee2e6;
			display: flex;
			flex-direction: column;
		}
			tabs
				tab 탭1
				tab 탭2
				tab 탭3
			tabContent
				tabPanel {}
		dragHandle
			dragHandleArrow
		mainArea
	end -- main
	
	menuRender {render:renderMenus(menuBar, menu), css(
		background-color: #f8f9fa;
		border-bottom: 1px solid #dee2e6;
		padding: 5px;
		display: flex;
		gap: 10px;
	)}
		menuItem {css(
			padding: 5px 10px;
			cursor: pointer;
			border-radius: 3px;
			position: relative;
		) active(
			@[submenu] => display: block;
		)}
			menuName {tag:span} 파일
			submenu {css(
			    position: absolute;
				top: 100%;
				left: 0;
				background-color: white;
				border: 1px solid #dee2e6;
				border-radius: 3px;
				box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
				min-width: 150px;
				display: none;
				z-index: 1000;
			)}
				submenuItem {class=has-submenu, css(
					display: flex;
					align-items: center;
				) hover(
					@[subsubMenu] => display: block;
				)}
					subName {css(flex:1)} 새로만들기
					subArrow {css(
						font-size: 10px;
						margin-left: 5px;
						color: #6c757d;
					)}
					subsubMenu {css(
						position: absolute;
						top: 0;
						left: 100%;
						background-color: white;
						border: 1px solid #dee2e6;
						border-radius: 3px;
						box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
						min-width: 150px;
						display: none;
						z-index: 1001;
					)}
						submenuItem 소스
						submenuItem 스타일
						submenuItem 스크립트
				submenuItem 열기
				submenuItem 저장
				submenuDivider {css(
					height: 1px;
					background-color: #dee2e6;
					margin: 5px 0;
				)}
				submenuItem 종료
			end
		end ------------------------- menuItem
		menuItem
			menuName {tag:span} 편집
			submenu {css(
				padding: 8px 15px;
				cursor: pointer;
				position: relative;
			) hover (
				background-color: #e9ecef;
			)}
				submenuItem 실행취소
				submenuItem 다시실행
				submenuDivider
				submenuItem 복사
				submenuItem 붙여넣기
			end
		end ------------------------- menuItem
		helpIcon {style{
			position: absolute;
			right: 0;
			top: 8px;
			width: 24px;
			height: 24px;
			background-color: #007bff;
			color: white;
			border-radius: 50%;
			display: flex;
			justify-content: center;
			align-items: center;
			cursor: pointer;
			font-weight: bold;
			margin-right: 10px;
			transition: background-color 0.3s;
			
		}} <>?<>
	end -- menuRender
end


##> *test-tree {
	bg:#eee
}
container {vbox}
	top 	{h:40} TOP
	tree 	{bg:@[bg], overflow:auto}
	bottom 	{h:35} BOTTOM
end


<js>	
	function renderTree(node, parentEl, depth) {
		if(!parentEl) parentEl = @[tree]
		if(!depth) depth = 0 
		if( node.type==='root') {
			cf.selectFolder = []
			const ul = $('<ul/>').appendTo(parentEl)
			if(node.children && node.children.length) {	
				node.children.forEach(cur=>renderTree(cur,ul,depth+1)) 
			}
			return;
		}
		const el = $('<li class="'+node.type+'"/>').appendTo(parentEl)
		const line = $('<div/>').css({fontSize:'1em', cursor:'pointer'}).html(node.text).appendTo(el)
		line.data('node',node)
		line.on('click', e=>{
			e.preventDefault()
			const line = $(e.target)
			const node = line.data('node')
			if(e.ctrlKey ) {				
				const idx = cf.selectFolder.indexOf(node)
				if( idx==-1 ) {
					cf.selectFolder.push(node)
				} else {
					cf.selectFolder.splice(idx,1)
				}
				return @[top].html(cf.selectFolder.map(cur=>cur.text).join(', '))
			}
			if(node.children && node.children.length) {
				const ul = line.parent().find('ul')				
				if( node.expand ) {
					ul.hide()
					node.expand = false
				} else {
					ul.show()
					node.expand = true
				}
			}
			cf.clickTick = tm()
			clog('click node=>',node)
		})
		if(node.children && node.children.length) {			
			const ul = $('<ul/>').css({transition:'0.5s'}).appendTo(el)
			node.children.forEach(cur=>renderTree(cur,ul,depth+1)) 
			node.expand = true
		}
	}
	function makeTree(list) {
		const root={type:'root',children:[]}
		const parentMap = {}
		list.map(item=>{
			const parentNode = parentMap[item.parent]|| root
			const cur = {
				id:item.id, pid:item.parent, 
				type:item.type||'file', 
				text:item.text
			}
			if(!parentNode.children) parentNode.children=[]
			parentNode.children.push(cur)
			parentMap[cur.id] = cur
		})
		clog('root=>',root)
		renderTree(root)
	}	
</js>

<init>
	fetch('http://localhost/api/folders?path=C:/temp')
		.then(res=>res.json())
		.then(data=>makeTree(data.children))
</init>	

##> *test-form {
	bg:#aaa
	bgLabel:#459
}
container {flexCenter, full}
	form {column, w:300, h:250, p:40px 20px, gap:10, 
		bg:@[bg.light(120)],
		bd:2px solid @[bg.dark(20)]
		rad:50
	}
		inputBox {
			css(row,ai,mb:4px, gap:4px)
		}
			label {css(
				bg:@[bgLabel],color:#fff				
				w:80px, h:30px, p:8px
				flex, ai, jc:start
			)}
				labelText 아이디
			input {class=lineEdit, css(
				flex:1, h:30px, p:8px
			)}
		inputBox
			label 
				labelText 비밀번호
			input {type=password, class=lineEdit}
		space
		formButtons {row, gap:10}
			button ok {css(
				w:80px, h:35px
			), onclick(e) {
				e.preventDefault(); 
				alert('ok click')
			}}
			button cancel {onclick(e) {
				e.preventDefault(); 
				alert('cancle click')
			}}
end
<init>
	clog('form test')
</init>

##> *test_02 {
	bg:#0f0
}
screen {full}
	circle {hidden, 
		css(
			abs, bottom:0,w:20px,aspect-ratio:1/1, bg:@[bg]
			shadow(
				0 0 10px @[bg],
				0 0 20px @[bg],
				0 0 30px @[bg],
				0 0 50px @[bg]	
			)
			rad: 50%
			ani: glow 5s linear forwards
			kf( glow =>
				0% { tf:translateY(0); opacity:1 }
				50% { opacity:1}
				100% { tf:translateY(-100vh); opacity:0; }
			)
		) 
		before(
			abs,ctt,x:25%,y:100%,h:100vh,opacity:0.5,bg:linear-gradient(@[bg],transparent)
		)
	}
end
<js>
	const line = () => {
		const circle = $('<div class="circle"/>').appendTo(@[screen])
		circle.css({
			width:parseInt(Math.random() *12),
			left:parseInt(Math.random() * $(window).width()),
			animationDuration: parseInt(Math.random()*3)+2+'s'
		})
		setTimeout(() => circle.remove(), 5000)
	}	
</js>

<init>
	clog('init test page', line)
	setPageInterval(line,1000)
</init>

##> *test_01 {
	bgColor: @[randomColor()]
}

container {col, full, bg:@[bgColor.light(180)]}
	top {row, h:30, bg:@[bgColor], bb:1px solid @[bgColor.dark(100)]}
		log	{w:100}
		space {flex:1}
		buttos {w:180, row}
			button b1 {onclick() {alert('b1')}}
			button b2 {onclick() {alert('b3')}}
			input {type=checkbox }
	content {flexCenter}
		form {w:400, h:300, bd:1px solid @[bgColor], p:20}
			inputBox {css(h:50,p:8)}
				label aaa
				input 
			inputBox {css(h:50,p:8)}
				label aaa
				input			
	bottom {h:30, bt:1px solid @[bgColor.light(100)]}

<init>
	clog('test playground init ')
</init>