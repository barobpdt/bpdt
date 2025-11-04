(function() {
	function initForm(page, content) {
		content.addClass('form_btn3d')
		content.html(``)
		setFormEvent(page, content)
	}
	const pageImpl = {
		initPage: function() { initForm(this, this.contentEl) }
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContainer', {overflow:'auto'})
		, content: true
	}
	const pageInfo = {id:'form_btn3d', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()

function setFormEvent(page, content) { 
	const top = $('<div class="page-title"/>').css({height:80, padding:4}).appendTo(content)
	const body = $('<div class="page-body"/>').css({flex:1}).appendTo(content)
	tagBtn3d(top, 'test01')
	tagBtn3d(top, 'test02')
	tagBtn3d(top, 'test03')
}

loadStyle(`
.letter { width: fit-content; height: fit-content; transform-style: preserve-3d; padding: 10px; color: ; cursor: pointer; } 
.letter span { display: block; font-size: 60px; font-weight: 800; text-shadow: -1px 1px 0px , -2px 2px 0px ,-3px 3px 0px , -4px 4px 0px ,-5px 5px 0px , -6px 6px 0px ; } .letter:hover { color: ; }
.btn3d { width: 140px; height: 50px; position: relative; background: none; outline: none; border: none; padding: 0; margin: 0; } 
.btn3d .top { width: 100%; height: 100%; background: rgb(255, 255, 238); font-family: poppins; font-size: 16px; color: rgb(36, 38, 34); display: flex; align-items: center; justify-content: center; border-radius: 7mm; outline: 2px solid rgb(36, 38, 34); transition: 0.2s; position: relative; overflow: hidden; } 
.btn3d .bottom { position: absolute; width: 100%; height: 100%; background: rgb(229, 229, 199); top: 10px; left: 0; border-radius: 7mm; outline: 2px solid rgb(36, 38, 34); z-index: -1; } 
.btn3d .bottom::before { position: absolute; content: ""; width: 2px; height: 9px; background: rgb(36, 38, 34); bottom: 0; left: 15%; } 
.btn3d .bottom::after { position: absolute; content: ""; width: 2px; height: 9px; background: rgb(36, 38, 34); bottom: 0; left: 85%; } 
.btn3d:active .top { transform: translateY(10px); } 
.btn3d::before { position: absolute; content: ""; width: calc(100% + 2px); height: 100%; background: rgb(140, 140, 140); top: 14px; left: -1px; border-radius: 7mm; outline: 2px solid rgb(36, 38, 34); z-index: -1; } 
.btn3d .top::before { position: absolute; content: ""; width: 15px; height: 100%; background: rgba(0, 0, 0, 0.1); transform: skewX(30deg); left: -20px; transition: 0.25s; } 
.btn3d:active .top::before { left: calc(100% + 20px); }
`)