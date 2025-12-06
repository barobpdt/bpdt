(function() {
	loadStyle(`

#testPage .circle {
	position:absolute;bottom:0;width:20px;aspect-ratio:1/1;background:#0f0;box-shadow:0 0 10px #0f0,
				0 0 20px #0f0,
				0 0 30px #0f0,
				0 0 50px #0f0;border-radius:50%;animation:glow 5s linear forwards;
}

#testPage .circle:before {
	positioncontent:'';top:25%;left:100%;height:100vh;opacity:0.5;background:linear-gradient(#0f0,transparent);
}
@keyframes glow{
0%{transition:translateY(0);opacity:1;}
50%{opacity:1;}
100%{transition:translateY(-100vh);opacity:0;}
}
	`)
	
	const line = () => {
		const circle = $('<div class="circle"/>').appendTo(getPageEl('.screen'))
		circle.css({
			width:parseInt(Math.random() *12),
			left:parseInt(Math.random() * $(window).width()),
			animationDuration: parseInt(Math.random()*3)+2+'s'
		})
		setTimeout(() => circle.remove(), 5000)
	}	

	function initPage(page, pageLayoutElement) {
		const root=$('<div id="testPage" class="screen"/>').css({width:'100%',minHeight:'100vh'}).appendTo(pageLayoutElement)
$('<div class="circle"/>').css({display:'hidden'}).appendTo(root)

	clog('init test page', line)
	setPageInterval(line,1000)

	}
	makePage('test', initPage)
})()