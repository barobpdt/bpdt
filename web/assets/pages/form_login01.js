(function() {
loadStyle(`
.login-page {
  width: 360px;
  padding: 8% 0 0;
  margin: auto;
}
.divForm {
  position: relative;
  z-index: 1;
  background: #FFFFFF;
  max-width: 360px;
  margin: 0 auto 100px;
  padding: 45px;
  text-align: center;
  box-shadow: 0 0 20px 0 rgba(0, 0, 0, 0.2), 0 5px 5px 0 rgba(0, 0, 0, 0.24);
}
.divForm input {
  font-family: "Roboto", sans-serif;
  outline: 0;
  background: #f2f2f2;
  width: 100%;
  border: 0;
  margin: 0 0 15px;
  padding: 15px;
  box-sizing: border-box;
  font-size: 14px;
}
.divForm button {
  font-family: "Roboto", sans-serif;
  text-transform: uppercase;
  outline: 0;
  background: #4CAF50;
  width: 100%;
  border: 0;
  padding: 15px;
  color: #FFFFFF;
  font-size: 14px;
  -webkit-transition: all 0.3 ease;
  transition: all 0.3 ease;
  cursor: pointer;
}
.divForm button:hover,.divForm button:active,.divForm button:focus {
  background: #43A047;
}
.divForm .message {
  margin: 15px 0 0;
  color: #b3b3b3;
  font-size: 12px;
}
.divForm .message a {
  color: #4CAF50;
  text-decoration: none;
}
.divForm .register-form {
  display: none;
}
`)
	function initForm(page, content) {	
		setCss(content, 'itemCenter')
		const map = new Map()
		const box = $('<div class="login-page"/>').appendTo(content)
		const forms = $('<div class="divForm"/>').appendTo(box)
		for(let n=0;n<2;n++) {
			const cls=n==0?'register-form':'login-form'
			const aa=n==0?
				[
					{name:'name'},
					{name:'passwd',type:'password'},
					{name:'email'},
					{name:'create', type:'button'},
					{text:'이미 등록되었나요', link:'로그인', type:'message'},
				]:[
					{name:'name'},
					{name:'passwd',type:'password'},
					{name:'login', type:'button'},
					{text:'등록되었나요', link:'가입', type:'message'},
				]
			const form = $('<form class="'+cls+'">').appendTo(forms)
			for(const a of aa) {
				if(!a.type) a.type='input'
				if(a.type=='button') {
					$('<button/>').text(a.name).appendTo(form)
				} 
				else if(a.type=='message') {
					const msg = $('<p class="message"/>').html(a.text+' <a href="#">'+a.link+'</a>').appendTo(form)
					msg.find('a').click(()=> {
						$('form').animate({height: "toggle", opacity: "toggle"}, "slow");
					})
				}
				else {
					$('<input type="'+a.type+'"/>').prop('placeholder', a.name).appendTo(form)
				}
			}
		}
	}
	makePage('form_login01', initForm)
})()