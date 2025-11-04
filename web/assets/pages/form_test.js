(function() {
loadStyle(`
.formTest .form {
    width: 350px;
    padding: 20px;
    background-color: whitesmoke;
    border-radius: 4px;
    font-size: 12px;
}

.formTest .form h1 {
    color: #0f2027;
    text-align: center;
}

.formTest .form button {
    padding: 10px;
    margin-top: 10px;
    width: 100%;
    color: white;
    background-color: rgb(41, 57, 194);
    border: none;
    border-radius: 4px;
}

.formTest .input-control {
    display: flex;
    flex-direction: column;
}

.formTest .input-control input {
    border: 2px solid #f0f0f0;
	border-radius: 4px;
	display: block;
	font-size: 12px;
	padding: 10px;
	width: 100%;
}

.formTest .input-control input:focus {
    outline: 0;
}

.formTest .input-control.success input {
    border-color: #09c372;
}

.formTest .input-control.error input {
    border-color: #ff3860;
}

.formTest .input-control .error {
    color: #ff3860;
    font-size: 9px;
    height: 13px;
}
`)
function setFormEvent(form, map) {
	const username = map.get('UserName')
	const email = map.get('Email')
	const password = map.get('Password')
	const password2 = map.get('PasswordCheck')
	
	password.attr('type','password')
	password2.attr('type','password')
	// clog('##',username, email, password, password2 )
	form.on('submit', e => {
		e.preventDefault();
		validateInputs();
	});
	const setError = (element, message) => {
		const inputControl = element.parent();
		const errorDisplay = inputControl.find('.error');
		errorDisplay.html(message);
		inputControl.addClass('error');
		inputControl.removeClass('success')
	}

	const setSuccess = element => {
		const inputControl = element.parent();
		const errorDisplay = inputControl.find('.error');
		errorDisplay.html('');
		inputControl.removeClass('error');
		inputControl.addClass('success')
	};

	const isValidEmail = email => {
		const re = /^(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
		return re.test(String(email).toLowerCase());
	}

	const validateInputs = () => {
		const usernameValue = username.val().trim();
		const emailValue = email.val().trim();
		const passwordValue = password.val().trim();
		const password2Value = password2.val().trim();

		if(usernameValue === '') {
			setError(username, 'Username is required');
		} else {
			setSuccess(username);
		}

		if(emailValue === '') {
			setError(email, 'Email is required');
		} else if (!isValidEmail(emailValue)) {
			setError(email, 'Provide a valid email address');
		} else {
			setSuccess(email);
		}

		if(passwordValue === '') {
			setError(password, 'Password is required');
		} else if (passwordValue.length < 8 ) {
			setError(password, 'Password must be at least 8 character.')
		} else {
			setSuccess(password);
		}

		if(password2Value === '') {
			setError(password2, 'Please confirm your password');
		} else if (password2Value !== passwordValue) {
			setError(password2, "Passwords doesn't match");
		} else {
			setSuccess(password2);
		}

	};
}

	function initForm(page, content) {
		content.addClass('formTest')		
		setCss(content, 'itemCenter')
		const form = $('<form class="form"/>').appendTo(content)
		$('<h1/>').html('test form').appendTo(form)
		const labels='UserName, Email, Password, PasswordCheck'
		const map = new Map()
		for(const key of labels.splitComma()) {			
			const row = $('<div class="input-control">').appendTo(form)
			$('<label/>').html(key).appendTo(row)
			map.set(key, $('<input/>').appendTo(row) )
			$('<div class="error"/>').appendTo(row)
		}
		const btn = $('<button type="submit">Sign Up</button>').html('등 록').appendTo(form)
		map.set('submit', btn)
		map.set('form', form)
		setFormEvent(form, map)
		page.inputMap = map		
	}
	const pageImpl = {
		initPage: function() {
			initForm(this, this.contentEl)
		}
	}
	const layout = {
		tag:'div'
		, style: getCss('flexCenter')
		, content: true
	}
	const pageInfo = {id:'form_test', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()