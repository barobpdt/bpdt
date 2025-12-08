(function() {
	loadStyle(`

	`)
	
	function initPage(page, pageLayoutElement) {
		const root=$('<div id="mainPage"/>').css({display:'flex',flexDirection:'column', alignItems:'center',justifyContent:'center', width:'100%',height:'100%'}).appendTo(pageLayoutElement)
$('<h1 class="h1"/>').html(`test`).appendTo(root)
$('<h2 class="h2"/>').html(`aaa`).appendTo(root)
$('<h3 class="h3"/>').html(`bbb`).appendTo(root)
$('<h4 class="h4"/>').html(`ccc`).appendTo(root)
const ws = cf.websocket
	if( ws ) {
		ws.connectWebSocket()
		ws.setCallbackFunc((type,data) => {
			clog('@@ websocket callback ', type, data)
			switch(type) {
			case 'changePageScript':
				const app = cf.apps.currentApp
				if( app ) {
					app.loadPage(data.pageCode)
				}
				return true
			default:
			}
		})
	}
	}
	makePage('main', initPage)
})()