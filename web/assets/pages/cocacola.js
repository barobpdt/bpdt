(function() {
	loadStyle(`

#cocacolaPage .card {
	position:relative;height:350px;transition-delay:0.5s;border-radius:20px;transition:0.5s;display:flex;align-items:center;justify_content:center;width:350px;background:#333;
}
#cocacolaPage .card:hover {
	transition-delay:0.5s;width:600px;
}
#cocacolaPage .card:hover #cocacolaPage .circle::before {
	width:100% height:100% rad:20px;background:#f33;
}
#cocacolaPage .card:hover #cocacolaPage .img.logo {
	transition-delay:0s;transition:scale(0);
}
#cocacolaPage .card:hover #cocacolaPage .img.prod {
	top:72%;height:500px;transition-delaleft:25%;transform:translate(-50%,-50%) scale(1) rotate(15deg);
}
#cocacolaPage .circle {
	width:100%;height:100%;top:0;border-radius:20px;left:0;position:absolute;display:flex;align-items:center;justify_content:center;
}
#cocacolaPage .circle:before {
	top:0;height:350px;border:8px solid #f33;border-radius:50%;content:'';left:0;filter:drop-shadow(0 10px #f33) drow-shadow(0 60px #5A4A4AFF);position:absolute;width:350px;background:#aaa;
}
	`)
	
	function initPage(page, content) {
		const cocacolaContainer=$('<div id="cocacolaPage" class="container"/>').css({width:'100%',height:'100%',width:'100%',height:'100%',display:'flex', alignItems:'center', justifyContent:'center'}).appendTo(content)
const card=$('<div class="card"/>').appendTo(cocacolaContainer)
const circle=$('<div class="circle"/>').appendTo(card)
const img=$('<img src="/images/coca.png" class="img logo"/>').css({position:'relative',transition:'0.5s',transitionDelay:'0.5s',width:'250px'}).appendTo(circle)
const box=$('<div class="box"/>').appendTo(card)
const h2=$('<h2 class="h2"/>').html(`test`).appendTo(box)
const img1=$('<img src="/images/coca-prod.png" class="img prod"/>').css({top:'50%',transition:'0.5s ease-in-out',position:'absolute',left:'50%',transform:'translate(-50%, -50%) scale(0) rotate(315deg)'}).appendTo(card)

	}
	makePage('cocacola', initPage)
})()