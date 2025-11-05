function onLoad(page, content) {
	content.addClass('baro')
	const bar = $('<div/>').css({height:50}).appendTo(content)
	$('<button class="btn">test1</button>').appendTo(bar)
	$('<button class="btn">test2</button>').appendTo(bar)
	$('<button class="btn">test3</button>').appendTo(bar)
	const body = $('<div/>').css({flex:1}).appendTo(content)
}
(function() {
loadStyle(`
.baro .btn {
  margin: 10px;
  padding: 15px 40px;
  border: none;
  outline: none;
  color: #FFF;
  cursor: pointer;
  position: relative;
  z-index: 0;
  border-radius: 12px;
}
.baro .btn::after {
  content: "";
  z-index: -1;
  position: absolute;
  width: 100%;
  height: 100%;
  background-color: #333;
  left: 0;
  top: 0;
  border-radius: 10px;
}
/* glow */
.baro .btn::before {
  content: "";
  background: linear-gradient(
	45deg,
	#FF0000, #FF7300, #FFFB00, #48FF00,
	#00FFD5, #002BFF, #FF00C8, #FF0000
  );
  position: absolute;
  top: -2px;
  left: -2px;
  background-size: 600%;
  z-index: -1;
  width: calc(100% + 4px);
  height:  calc(100% + 4px);
  filter: blur(8px);
  animation: glowing 20s linear infinite;
  transition: opacity .3s ease-in-out;
  border-radius: 10px;
  opacity: 0;
}

@keyframes glowing {
  0% {background-position: 0 0;}
  50% {background-position: 400% 0;}
  100% {background-position: 0 0;}
}

/* hover */
.baro .btn:hover::before {
  opacity: 1;
}

.btn:active:after {
  background: transparent;
}

.baro .btn:active {
  color: #000;
  font-weight: bold;
}
`)	
	const app = cf.apps.currentApp
	const layout = {
		tag:'div'
		, style: getCss('pageContent')
		, children:[
			{tag:'div',style:getCss('full',{height:30,background:getRandomColor()}), className:'appTop'},
			{tag:'div',style:{flex:1}, className:'appContents', content:true},
			{tag:'div',style:{height:30,background:getRandomColor()}, className:'appFooter'},
		]
	} 
	clog('####> ', app)
	const pageImpl = {
		initPage: function() {
			clog('main page init ==> ', this)
			onLoad(this, this.contentEl)
		}
	}
	const pageInfo = {id:'main', layout}
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()