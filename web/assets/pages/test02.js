(function() {
	const pageImpl = {
		initPage: function() {
			clog('init page this >>',this) 
			const bar = $('<div/>').css({height:50}).appendTo(this.contentEl)
			$('<button class="btn">test1</button>').appendTo(bar)
			$('<button class="btn">test2</button>').appendTo(bar)
			$('<button class="btn">test3</button>').appendTo(bar)
			const body = $('<div/>').css({flex:1}).appendTo(this.contentEl)
		},
		test: function(a) {
			clog('test called', a)
		}
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContent')
		, children:[
			{tag:'div',style:getCss('full',{height:30,background:getRandomColor()}), className:'appTop'},
			{tag:'div',style:getCss('pageContent'), className:'appContents', content:true},
			{tag:'div',style:{height:30,background:getRandomColor()}, className:'appFooter'},
		]
	}
	const app = cf.apps.currentApp
	const pageInfo = {id:'test02', layout}
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()