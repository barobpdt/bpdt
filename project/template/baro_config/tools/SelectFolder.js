##> config { name=SelectFolder }
	@eval { 
		/*
		runSrc('pageOpen') 
		runSrc('make-icons')
		*/
		print('SelectFolder Load')
	}
	pageOpen = @eval {
		src=cv('layout.pages')
		// page('SelectFolder:main').close()
		page = pageLoad(src,'SelectFolder1')
		if(page) {
			page.open()
		}
	}
	make-icons = @eval {
		icons=object('icon.extIcons')
		if(icons.var(loadComplet)) {
			print("icons==$icons")
		}
		s=cv('data.icons')
		s.ref()
		cate=''
		print(">> icons", s)
		while(s.valid()) {
			line=s.findPos("\n")
			c=line.ch() not(c) break;
			if(c=='#') {
				a=line.incr().trim()
				splitSep(a,':').inject(cate, cateNm)
				cur=icons.addNode().with(cate, cateNm)
				continue;
			}
			if(c.eq()) {
				ext=line.match(), url=''
				c=line.ch()
				if(c.eq(':')) {
					c=line.incr().ch()
					url=line.match()
				}
				if(url) {
					cur.addNode().with(ext, url)
				}
			}
		}
		print('>> icons:', json(icons,'children',true) ) 
		icons.var(loadComplet, true)
	}
	
##> layout {name=player}
pages {
	<page id="main" initPage() {
		splitter = widget('splitter')
		splitter.addPage(page('TreePage'))
		splitter.addPage(page('GridPage'))		
	} 
	setSelectFolder() {
		print("set select folder")
	}>
		<hbox>
			<label id="title" height=30>
		</hbox>
		<splitter>
		<hbox>
			<button id="selectFolder" text="폴더선택" onClick() { page().setSelectFolder() }>
			<space>
			<button id="cancle" text="취소" onClick() {page().close()}>
		</hbox>
	</page>
	
	<page id="TreePage" margin=0 initPage() {
		widget('tree').initTree()
	}>
		<tree id="tree" module="tree,SelectFolderTree">
		<hbox>
			<button id="ok" text="ok">
			<space>
		</hbox>
	</page>
	
	<page id="GridPage" margin=0 initPage() {
		widget('grid').initGrid()
	}>
		<grid id="grid" module="grid,SelectFolderGrid">
		<hbox>
			<button id="ok" text="ok">
			<space>
		</hbox>
	</page>
} 
 
##> module {name=SelectFolderTree}
	initTree() {
		tree=this
		event(tree,'onDraw', tree.draw)
		event(tree,'onFilter', tree.filter)
		event(tree,'onChange', tree.changeNode)
		event(tree,'onMouseDown', tree.mouseDown)
		event(tree,'onMouseUp', tree.mouseUp)
		event(tree,'onTimer', tree.treeTimer)
		tree.model('name')
		tree.timer(500)

		root=object('baro.SelectFolders').removeAll()
		while(path, System.driveList() ) {
			drive=root.addNode()
			drive.fullPath = path
			drive.type='drive'
		}
		cur=root.child(0)
		getFolderList(cur.path, cur)
		while(sub, cur) {
			getFolderList(sub.fullPath, sub)
		}
		this.rootNode(root)
	}
	treeTimer() {
		if(this.childLoadTick ) {
			print("timer >> ${this.childLoadTick}")
			node = this.currentNode
			this.expand(node, true)
			this.childLoadTick=0
			return;
		}
	}
	changeNode(node) {
		if(this.childLoadTick) {
			return true;
		}
		print("xxxxxxxx current node xxxxxxxxxx", node)
		this.currentNode = node
		not(node.childLoad) {
			getFolderList(node.fullPath, node)
			while(sub,node) {
				getFolderList(sub.fullPath, sub)
			}
			this.childLoadTick = System.tick()
			this.update()
		}
		if(node.cmp('type','drive')) {
			this.expand(node, true)
		}
		grid=page('GridPage').get('grid')
		if(grid) {
			grid.setFolderNode(node)
		}
	}
	filter(node) {
		not(node.type.eq('drive','folder')) return false;
		p=node.parentNode()
		name=node.name
		if(p.var(rootPath) && node.name.eq('Program Files','Program Files (x86)','Windows') ) {
			return false;
		}
		return true;
	}
	draw(dc, node, index, state, over)	{
		rc=this.drawSelect(dc, dc.rect(), state, over)
		node.rcIcon=rc.moveLeft(18,18,-2,0,true)
		if(node.cmp('type','drive')) {
			c=node.fullPath.ch()
			text="$c: DRIVE"		
			rcBase = rc.incrXW(-20,-20)
			dc.save().font(object('font.bold'))
			dc.rectLine(rcBase, 4, '#ccc', 1, 'dot')
			dc.fill(rcBase, '#ddddee77')
			dc.text(rc, text)
			dc.restore()
		} else {		
			text=node.get('name')
			dc.text(rc, text)
		}	
	}
	mouseDown(pos, check) {
		print(">> mouse down", pos, check)
	}
	mouseUp(pos, check) {
		print(">> mouse up", pos, check)
	}

##> module {name=SelectFolderGrid}
	initGrid() {
		grid=this 
		event(grid,'onDraw', grid.draw)
		event(grid,'onFilter', grid.filter)
		event(grid,'onChange', grid.change)
		event(grid,'onMouseDown', grid.mouseDown)
		event(grid,'onMouseUp', grid.mouseUp)
		event(grid,'onMouseMove', grid.mouseMove)
		event(grid,'onTimer', grid.gridTimer)
		fields=[
			{ field:chk, text:선택, width:40}
			{ field:name, text: 파일명, width:200}
			{ field:size, text: 파일크기, width:90}
			{ field:modifyDt, text: 수정일시, width:150}
			{ field:edit, text: 편집, width:*}
		]
		grid.model(fields)
		grid.is('stretchLast', true)
		grid.is('sortEnable', true)
		grid.timer(500)
	}
	setFolderNode(node) {
		root=this.rootNode().removeAll()
		getFileList(node.fullPath, root)
		this.update()
	}
	gridTimer() {
		overNode = this.mouseOverNode
		if(overNode) {
			if( overNode != this.tooltipNode) {
				dist = System.tick() - this.mouseOverTick;
				if(dist>1000) {
					this.tooltip(overNode.name, true)
					this.tooltipNode = overNode
				}
			}
		} else if( this.tooltipNode) {
			this.tooltip('', false)
		}
	}
	draw(dc, node, index, state) {
		field=this.field(index)
		rc=this.drawState(dc, node, state, index, field )
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
		} else if(field.eq('size')) {
			dc.text(rc.incrW(-4),node.size, 'right')
		} else if(field.eq('modifyDt')) {
			dc.text(rc,node.modifyDt, 'center')
		} else if(field.eq('edit')) {
			this.inject(colorBtn, colorBtnText)
			dark=colorBtn.darkColor(100)
			dc.save().font(colorBtnText)
			while(rcBtn, hbox(rc,'80,80,*',true), n) {
				if(n==2) break;
				rcCur=rcBtn.incr(4)
				roundRect(dc, rcCur, dark, true, 'gradient', colorBtn, dark)
				switch(n) {
				case 0:
					node.rcOpenFolder=rcCur
					dc.text(rcCur, '열기', 'center')
				case 1: 
					node.rcEdit=rcCur
					dc.text(rcCur, '편집', 'center')
				}
			}
			dc.restore()
		} else {
			dc.text(rc.incrX(2), node.get(field))
		}
	}
	change(node) {
		
	}
	filter(node) {
		return true
	}
	mouseDown(pos, check) {
		hh=this.headerHeight()
		node=this.at(pos.incrY(hh))
		field=node.var(code)
		if(field=='chk') {
			chk=when(node.flag(NODE.check), false, true)
			node.flag(NODE.check, chk)
			this.update()			
		}
		else if(field=='edit') {
			if(node.rcOpenFolder.contains(pos)) {
				print("OpenFolder down") 
			} 
			else if(node.rcEdit.contains(pos)) {
				print("Edit down") 
			}
		}
		
	}
	mouseUp(pos, check) {
	}
	mouseMove(pos) {
		hh=this.headerHeight()
		node=this.at(pos.incrY(hh)) not(node) return;
		if( node.var(code)=='name') {
			this.mouseOverNode=node
			this.mouseOverTick=System.tick()
		} else {
			this.mouseOverNode=null
		}
	}

##> data
icons {
	# doc: 문서 파일
	'.txt': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_text.svg',
	'.md': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_markdown.svg',
	'.pdf': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_pdf.svg',
	'.doc': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_word.svg',
	'.docx': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_word.svg',
	'.xls': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_excel.svg',
	'.xlsx': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_excel.svg',
	'.ppt': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_powerpoint.svg',
	'.pptx': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_powerpoint.svg',
	
	# source:소스 파일
	'.py': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_python.svg',
	'.js': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_js.svg',
	'.jsx': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_reactjs.svg',
	'.ts': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_typescript.svg',
	'.tsx': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_reactts.svg',
	'.html': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_html.svg',
	'.css': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_css.svg',
	'.scss': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_scss.svg',
	'.less': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_less.svg',
	'.json': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_json.svg',
	'.xml': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_xml.svg',
	'.yaml': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_yaml.svg',
	'.yml': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_yaml.svg',
	'.sql': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_sql.svg',
	'.php': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_php.svg',
	'.java': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_java.svg',
	'.c': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_c.svg',
	'.cpp': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_cpp.svg',
	'.h': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_h.svg',
	'.hpp': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_hpp.svg',
	'.cs': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_csharp.svg',
	'.go': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_go.svg',
	'.rb': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_ruby.svg',
	'.swift': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_swift.svg',
	'.kt': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_kotlin.svg',
	'.rs': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_rust.svg',
	'.sh': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_shell.svg',
	'.bat': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_shell.svg',
	'.ps1': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_powershell.svg',
	
	# image:이미지 파일
	'.jpg': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	'.jpeg': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	'.png': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	'.gif': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	'.bmp': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	'.svg': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_svg.svg',
	'.ico': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	'.webp': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_image.svg',
	
	# audio: 오디오 파일
	'.mp3': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_audio.svg',
	'.wav': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_audio.svg',
	'.ogg': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_audio.svg',
	'.flac': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_audio.svg',
	'.aac': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_audio.svg',
	
	# video: 비디오 파일
	'.mp4': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_video.svg',
	'.avi': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_video.svg',
	'.mov': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_video.svg',
	'.wmv': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_video.svg',
	'.flv': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_video.svg',
	'.mkv': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_video.svg',
	
	# zip: 압축 파일
	'.zip': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_zip.svg',
	'.rar': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_archive.svg',
	'.7z': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_archive.svg',
	'.tar': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_archive.svg',
	'.gz': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file_type_archive.svg',
	
	# folde: 폴더
	'folder': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/folder.svg',
	'folder_open': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/folder_opened.svg',
	
	# etc: 기본 아이콘
	'default': 'https://raw.githubusercontent.com/vscode-icons/vscode-icons/master/icons/file.svg',
}