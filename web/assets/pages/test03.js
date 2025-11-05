(function() {
	const pageImpl = {
		initPage: function() {
			clog('init page this >>',this) 
			const bar = $('<div/>').css({height:50}).appendTo(this.contentEl)
			const body = $('<div/>').css({flex:1, background:getRandomColor()}).appendTo(this.contentEl)
			$('<div>test03 PAGE</div>').appendTo(bar)
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
	const pageInfo = {id:'test03', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()