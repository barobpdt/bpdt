(function() {
	loadStyle(`
 
	`)
	
	const aaa = () => {
		clog('aaa function ', getPageEl('.icon'))
		getPageEl('.ul').html('')		
	}
	const linkClick = () => {
		clog('link click')
	}


function renderNavItems(item) {
	const content = getRenderElement('ul')
	clog('render content =>', content, item)
	const li=$(`<div class="li"/>`).css({position:'relative',height:60,padding:'0 10px',listStyle:'none',width:'100%'}).appendTo(content)
const a=$(`<a href="#" class="a"/>`).css({tag:'a',width:'100%',height:'100%',display:'flex', alignItems:'center', justifyContent:'flex-start'}).appendTo(li)
a.on('click',()=>{})
const span=$(`<span class="span icon"/>`).css({position:'relative',height:65,borderRadius:65,tag:'span',display:'block',minWidth:65,color:'#fff',fontSize:'1.75em'}).appendTo(a)
clog(`<div class="icon ui-icon ${item}"/>`, item)
const icon=$(`<div class="icon ui-icon ${item}"/>`).appendTo(span)

}
	function initPage(page, content) {
		const leftNavbarContainer=$('<div id="leftNavbarPage" class="container"/>').appendTo(content)
const navbar=$('<div class="navbar"/>').css({position:'fixed',top:40,padding:20,borderRadius:50,width:80,height:'100%',display:'flex', alignItems:'center', justifyContent:'center',background:'#5A4A4AFF'}).appendTo(leftNavbarContainer)
const ul=$('<div class="ul"/>').css({display:'flex',flexDirection:'column',gap:10,width:100}).appendTo(navbar)
setRenderElement('ul',ul)

			clog('init page this==>', getPageEl('.icon'))
			fetch('http://localhost/api/dev/test/navIcons').then(res=>res.json()).then(a=>a.map(item=>renderNavItems(item)))
		
	}
	makePage('leftNavbar', initPage)
})()