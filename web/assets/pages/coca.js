(function() {
	loadStyle(`

#cocaPage .card {
	position:relative;height:350px;transition-delay:0.5s;border-radius:20px;transition:0.5s;display:flex;align-items:center;justify-content:center;width:350px;background:#333;
}
#cocaPage .card:hover {
	transition-delay:0.5s;width:600px;
}
#cocaPage .circle {
	width:100%;height:100%;top:0;border-radius:20px;left:0;position:absolute;display:flex;align-items:center;justify-content:center;
}
#cocaPage .circle:before {
	top:0;height:350px;border:8px solid #f33;border-radius:50%;content:'';left:0;filter:drop-shadow(0 0 10px #f33) drop-shadow(0 0 60px #5A4A4AFF);position:absolute;width:350px;background:#aaa;
}
	`)
	
	function initPage(page, content) {
		const cocaContainer=$('<div id="cocaPage" class="container"/>').css({width:'100%',height:'100%',width:'100%',height:'100%',display:'flex', alignItems:'center', justifyContent:'center'}).appendTo(content)
const card=$('<div class="card"/>').appendTo(cocaContainer)
const circle=$('<div class="circle"/>').appendTo(card)
const img=$('<img src="/images/cocacola_logo.png" class="img logo"/>').appendTo(circle)

	}
	makePage('coca', initPage)
})()