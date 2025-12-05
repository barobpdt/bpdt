(function() {
	loadStyle(`

	`)
	
	function initPage(page, content) {
		const mainContainer=$('<div id="mainPage" class="container"/>').css({width:'100%',height:'100%',width:'100%',height:'100%',display:'flex', alignItems:'center', justifyContent:'center'}).appendTo(content)
const h1=$('<h1 class="h1"/>').html(`test`).appendTo(mainContainer)
const h2=$('<h2 class="h2"/>').html(`aaa`).appendTo(h1)
const h3=$('<h3 class="h3"/>').html(`bbb`).appendTo(h1)
const h4=$('<h4 class="h4"/>').html(`ccc`).appendTo(h1)

	}
	makePage('main', initPage)
})()