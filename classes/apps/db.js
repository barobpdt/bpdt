class dbFields { 
	initPage() {
		print("db field page init", this.member());
	}
	clickButton(param) {
		id=when(typeof(param,'node'),param.id, param)
		switch(id) {
		case ok:
			this.alert("ok click")
		default:
		}
	}
}
