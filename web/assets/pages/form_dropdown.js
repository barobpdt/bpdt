(function() {
	function initForm(page, content) {
		content.addClass('form_dropdown')
		const bar = $('<div class="searchBar"/>').css({height:75, alignItems:'center'}).appendTo(content)
		const body = $('<div class=""/>').css({flex:1}).appendTo(content)
		addDropdown(bar)
		const dropdown = $('<div class="dropdown"/>').appendTo(bar)
		setFormEvent(page, content)
	}
	const pageImpl = {
		initPage: function() { initForm(this, this.contentEl) }
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContent', {overflow:'auto'})
		, content: true
	}
	const pageInfo = {id:'form_dropdown', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()

function addDropdown(bar) {
	const dropdown = $('<div class="dropdown"/>').appendTo(bar)
	const input = $('<div class="input-box"/>').appendTo(bar)
	const list = $('<div class="list"/>').appendTo(bar)
	list.html(`
		<div class="search-box">
			<input type="search" class="search" placeholder="Search Options"/>
		</div>
	`);
	const data = [
		{key:'id11', icon:'', text:'Walk'},
		{key:'id12', icon:'', text:'Road'},
		{key:'id13', icon:'', text:'Train'},
		{key:'id14', icon:'', text:'Flight'},
		{key:'id15', icon:'', text:'Ship'},
	]
	data.map(cur=>cur.icon=cur.icon||cur.text.toLowerCase())
	data.map(cur=>(`
		<input type="radio" name="drop1" id="" class="radio" />
		<label for="id11">
			<span class="material-icons-outlined"> directions_walk </span>
			<span class="name">Walk</span>
		</label>
	`))
	<div class="dropdown">
		<div class="title">Title</div>
		<div class="input-box"></div>
		<div class="list">
			<div class="search-box">
				<input
				type="search"
				name=""
				class="search"
				placeholder="Search Options"
				/>
			</div>
			

			<input type="radio" name="drop1" id="id12" class="radio" />
			<label for="id12">
				<span class="material-icons-outlined"> directions_bike </span>
				<span class="name"></span>
			</label>

			<input type="radio" name="drop1" id="id13" class="radio" />
			<label for="id13">
				<span class="material-icons-outlined"> train </span>
				<span class="name"></span>
			</label>

			<input type="radio" name="drop1" id="id14" class="radio" />
			<label for="id14">
				<span class="material-icons-outlined"> flight </span>
				<span class="name">Flight</span>
			</label>

			<input type="radio" name="drop1" id="id15" class="radio" />
			<label for="id15">
				<span class="material-icons-outlined"> directions_boat </span>
				<span class="name">Ship</span>
			</label>

			<input type="radio" name="drop1" id="id16" class="radio" />
			<label for="id16">
				<span class="material-icons-outlined"> local_shipping </span>
				<span class="name">Delivery</span>
			</label>
		</div>
	</div>
}
function setFormEvent(page, content) { 
	
}

loadStyle(`
.dropdown {
	width: 300px;
	height: fit-content;
	box-sizing: border-box;
	position: relative;        
}
.dropdown .input-box {
	width: 100%;
	height: 40px;
	box-sizing: border-box;
	outline: 0.3mm solid rgba(0, 0, 0, 0.15);
	border-radius: 2mm;
	padding: 10px 15px;
	font-family: poppins;
	font-size: 14px;
	cursor: pointer;
	display: flex;
	align-items: center;
	justify-content: flex-start;
	position: relative;
	background: #fff;
}
.dropdown .input-box::before {
	content: "expand_more";
	font-family: "Material Icons";
	position: absolute;
	font-size: 18px;
	top: 50%;
	right: 15px;
	transform: translate(0, -50%);
	width: fit-content;
	height: fit-content;
}
.dropdown .input-box.open::before {
	content: "expand_less";
}
.dropdown .input-box:empty::after {
	content: "Select Title";
	color: rgba(0, 0, 0, 0.5);
}
.dropdown .list {
	position: absolute;
	top: 100%;
	left: 0;
	width: 100%;
	height: fit-content;
	background: white;
	margin-top: 10px;
	border-radius: 2mm;
	overflow: hidden;
	display: flex;
	align-items: center;
	justify-content: space-between;
	flex-direction: column;
	max-height: 0;
	transition: 0.25s ease-out;
}
.dropdown .list input {
	display: none;
}
.dropdown .list label {
	display: flex;
	width: 100%;
	align-items: center;
	justify-content: flex-start;
	font-family: poppins;
	font-size: 14px;
	padding: 10px 15px;
	box-sizing: border-box;
	cursor: pointer;
	position: relative;
}
.dropdown .material-icons-outlined{
	margin-right: 5px;
	font-size: 22px;
}
.dropdown .list label:hover {
	background: rgba(0, 0, 0, 0.08);
}
.dropdown input:checked + label {
	color: rgb(20, 117, 213);
	background: rgb(238, 245, 252);
}
.dropdown input:checked + label::before {
	content: "done";
	font-family: "Material Icons";
	position: absolute;
	top: 50%;
	right: 15px;
	transform: translate(0, -50%);
	font-size: 18px;
}
.dropdown .open {
	outline: 0.7mm solid rgb(20, 117, 213);
}
.dropdown .title {
	font-family: poppins;
	font-size: small;
	font-weight: 500;
	margin-bottom: 10px;
}
.dropdown .search-box {
	width: 100%;
	box-sizing: border-box;
	padding: 10px 8px;
}
.dropdown .list .search {
	display: block;
	width: 100%;
	box-sizing: border-box;
	padding: 8px;
	border-radius: 1mm;
	border: none;
	outline: 0.3mm solid rgba(0, 0, 0, 0.15);
	font-family: poppins;
}
.dropdown .list .search:focus {
	outline: 0.5mm solid rgba(0, 0, 0, 0.35);
}
`)