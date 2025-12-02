(function() {
loadStyle(`
.login-box {
  position: absolute;
  top: 50%;
  left: 50%;
  width: 400px;
  padding: 40px;
  transform: translate(-50%, -50%);
  background: rgba(0,0,0,.5);
  box-sizing: border-box;
  box-shadow: 0 15px 25px rgba(0,0,0,.6);
  border-radius: 10px;
}

.login-box h2 {
  margin: 0 0 30px;
  padding: 0;
  color: #fff;
  text-align: center;
}

.login-box .user-box {
  position: relative;
}

.login-box .user-box input {
  width: 100%;
  padding: 10px 0;
  font-size: 16px;
  color: #fff;
  margin-bottom: 30px;
  border: none;
  border-bottom: 1px solid #fff;
  outline: none;
  background: transparent;
}
.login-box .user-box label {
  position: absolute;
  top:0;
  left: 0;
  padding: 10px 0;
  font-size: 16px;
  color: #fff;
  pointer-events: none;
  transition: .5s;
}

.login-box .user-box input:focus ~ label,
.login-box .user-box input:valid ~ label {
  top: -20px;
  left: 0;
  color: #03e9f4;
  font-size: 12px;
}

.login-box form a {
  position: relative;
  display: inline-block;
  padding: 10px 20px;
  color: #03e9f4;
  font-size: 16px;
  text-decoration: none;
  text-transform: uppercase;
  overflow: hidden;
  transition: .5s;
  margin-top: 40px;
  letter-spacing: 4px
}
`)
	const test = (a,b,c) => clog('test called ',a,b,c)
	function initForm(page, content) {	
		setCss(content, 'itemCenter')
		const map = new Map()
		const box = $('<div class="login-box"/>').appendTo(content)
		const title = $('<h2/>').html('로그인').appendTo(box)
		const form = $('<form class="form"/>').appendTo(box)
		for(const id of 'user_name, password'.splitComma()) {			
			const row = $('<div class="user-box"/>').appendTo(form)
			const type = id=='password' ? 'password': 'text'
			const input = $('<input type="'+type+'"/>').appendTo(row)
			$('<label/>').text(id).appendTo(row)
			map.set(id,input)
		}
		page.inputMap = map		
		setTimeout(()=> {
			for(const a of map) a[1].val('')
			mapAt(map,0).focus()
			test(1,2,3)
		}, 500)
	}
	makePage('form_login', initForm)
})()