(function() {
	loadStyle(`

	#wavePage .box {
		display:flex;
		width: 250px;
		color: #827676FF;
	}
	#wavePage .box.hovered {
		display:flex;
		width: 250px
	}

	`)
	
	function initPage(page, content) {
		const waveContainer=$('<div id="wavePage" class="container"/>')
		clog('@@ initPage', page, content, waveContainer);
		const aa= waveContainer.css({width:'100%',height:'100%',display:'flex',flexDirection:'column'})
		clog('@@ aa==',aa)
		aa.appendTo(content)
		clog("init page ==> ", page, content, waveContainer)
const box=$('<div class="box"/>').appendTo(waveContainer)
const box1=$('<div class="box"/>').appendTo(waveContainer)
const box2=$('<div class="box"/>').appendTo(waveContainer)
const box3=$('<div class="box"/>').appendTo(waveContainer)
const box4=$('<div class="box"/>').appendTo(waveContainer)
const box5=$('<div class="box"/>').appendTo(waveContainer)
const box6=$('<div class="box"/>').appendTo(waveContainer)
const box7=$('<div class="box"/>').appendTo(waveContainer)
const box8=$('<div class="box"/>').appendTo(waveContainer)

	}
	makePage('wave', initPage)
})()