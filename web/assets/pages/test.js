(function() {
	loadStyle(`

	`)
		
	function renderTree(node, parentEl, depth) {
		if(!parentEl) parentEl = getPageEl('.tree')
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
				return getPageEl('.top').html(cf.selectFolder.map(cur=>cur.text).join(', '))
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

	function initPage(page, pageLayoutElement) {
		const root=$('<div id="testPage" class="container"/>').css({display:'flex',flexDirection:'column',height:'100%'}).appendTo(pageLayoutElement)
$('<div class="top"/>').css({height:40,width:'100%'}).html(`TOP`).appendTo(root)
$('<div class="tree"/>').css({background:'#eee',overflow:'auto',width:'100%',flex:1}).appendTo(root)
$('<div class="bottom"/>').css({height:35,width:'100%'}).html(`BOTTOM`).appendTo(root)
fetch('http://localhost/api/folders?path=C:/temp')
		.then(res=>res.json())
		.then(data=>makeTree(data.children))
	}
	makePage('test', initPage)
})()