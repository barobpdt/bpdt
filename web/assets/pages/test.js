(function() {
	loadStyle(`

#testPage .circle {
	box-shadow:0 0 10px #6A8B3DFF,
			0 0 20px #6A8B3DFF,
			0 0 30px #6A8B3DFF,
			0 0 50px #6A8B3DFF;border-radius:50%;animation:glow 5s linear forwards;position:absolute;bottom:0;width:20px;aspect-ratio:1/1;background:#6A8B3DFF;
}@keyframes glow{
0%{opacity:1;transition:translateY(0);;}
50%{opacity:1;}
100%{opacity:0;;transition:translateY(-100vh);;}
}
	`)
	
	const line = () => {
		const circle = $('<div class="circle"/>').appendTo(getPageEl('.screen'))
		setTimeout(() => circle.remove(), 5000)
	}	

	function initPage(page, pageLayoutElement) {
		const root=$('<div id="testPage" class="screen"/>').css({minHeight:'100vh',width:'100%'}).appendTo(pageLayoutElement)
$('<div class="circle"/>').css({display:'hidden'}).appendTo(root)

	clog('init test page', line)
	setPageInterval(line,1000)

	}
	makePage('test', initPage)
})()