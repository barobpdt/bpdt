##> main {
	
}
	h1 test
	h2 aaa
	h3 bbb
	h4 ccc
end
<init>
	const ws = cf.websocket
	if( ws ) {
		ws.connectWebSocket()
		ws.setCallbackFunc((type,data) => {
			clog('@@ websocket callback ', type, data)
			switch(type) {
			case 'changePageScript':
				const app = cf.apps.currentApp
				if( app ) {
					app.loadPage(data.pageCode)
				}
				return true
			default:
			}
		})
	}
</init>
	
##> *coca {
	bgColor:#aaa
	baseColor:#f33
}
container {
	full, flexCenter
}
	card 
	{
		
		css(
			rel, flexCenter, w:350px, h:350px, 
			tr:0.5s, trdelay:0.5s, 
			bg:#333,rad:20px
		) 
		hover(
			w:600px, trdelay:0.5s
		) 
	}
		circle  
		{
			css(
				abs,full,flexCenter,x:0,y:0,rad:20px
			)
			before(
				content, abs, x:0,y:0,w:350px,h:350px,
				bd:8px solid @[baseColor],  bg:@[bgColor]
				rad:50%
				filter:drop-shadow(0 0 10px @[baseColor]) drop-shadow(0 0 60px @[baseColor.light(50)]), tr(0.5s, background 0.5s), trdelay(0.75s,1s)
			)
		}
			img {src=/images/cocacola_logo.png, class=logo } 
	end
	
##> *cocacola {
	bgColor:#aaa
	baseColor:#f33
}
container {
	full, flexCenter
}
	card 
	{
		css(
			rel, flexCenter, w:350px, h:350px, 
			tr:0.5s, trdelay:0.5s, 
			bg:#333,rad:20px
		) 
		hover(
			w:600px, trdelay:0.5s
		)
		hover(
			@[circle]::before => w:100% height:100% rad:20px, bg:@[baseColor]
		)
		hover(
			@[img.logo] => tr:scale(0), trdelay:0s 
		)
		hover(
			@[img.prod] => x:72%,y:25%, h:500px, 
			trdelay:0.75s, tf: translate(-50%,-50%) scale(1) rotate(15deg)
		)
	}
	
		circle 
		{
			css(
				abs,full,flexCenter,x:0,y:0,rad:20px
			)
			before(
				content, abs, x:0,y:0,w:350px,h:350px,
				bd:8px solid @[baseColor],  bg:@[bgColor]
				rad:50%
				filter:drop-shadow(0 0 10px @[baseColor]) drop-shadow(0 0 60px @[baseColor.light(50)]), tr(0.5s, background 0.5s), trdelay(0.75s,1s)
			)
		}
			img {src=/images/cocacola_logo.png, class=logo, rel, w:250px, tr:0.5s, trdelay:0.5s}
		box
			h2 test 
		img {src=/images/coca-prod.png, class=prod, abs, x:50%, y:50%, 			
			transform: translate(-50%, -50%) scale(0) rotate(315deg)
			tr:0.5s ease-in-out
		}
	end

##> *leftNavbar [메뉴바 에니메이션] {
	bgColor:#223f4d
}
container
	navbar {
		flexCenter,fixed,x:40,w:80,p:20
		bg:@[bgColor.light(50)]
		rad:50
	}
		ul {render:renderNavItems(item), col,gap:10,width:100}
			li {rel,listStyle,w:100%,h:60, p:0 10px}
				a {href="#", flexCenter,ai:center,jc:flex-start, onclick(){linkClick('link click')}}
					span {class="icon", rel,block,minw:65,h:65,rad:65,bg:@[bgColor],c:#fff,fs:1.75em}
						icon {class="vicon @[item.icon]", abs,w:24,h:24}
		end
		<init>
			clog('init page this==>', @[this])
			alert('init')
		</init>
	end
<js>
	const aaa = () => {
		clog('aaa function ', @[this])
		@[ul].html('')		
	}
	const linkClick = () => {
		clog('link click')
	}
</js>	

##> *form-test [폼입력창 라벨 에니메이션] {
	bgColor: #1c3354
}
container {flexCenter, bg:#1c3354}
	box {rel, w:400 }
		h2 login {fs:2em,fw:600,textAlign:center,w:100%,mb:40,ls:4}
		form {rel, w:100%}
			inputBox {rel, w:100%, h:100}
				input {required, css(
					abs, w:100%, p:20px 10px, pl:40px
					bd:2px solid @[bgColor.dark(80)]
					rad:8
					fs:1em, fw:400, ls:2
					bg:@[bgColor.light(50)]
					color:@[bgColor.light(150)]
					outline:none
				),focus(
					~ .icon => x:-25px, y:-30px, w:34, h:34
				),valid(
					~ .icon => x:-25px, y:-30px, w:34, h:34
				),focus(
					~ .label => x:-25px, y:-45px, c:#fff, fs:0.7em, pl:40px
				),valid(
					~ .label => x:-25px, y:-45px, c:#fff, fs:0.7em, pl:40px
				)}
				icon {class="ui-icon application_add", css(
					abs, pe, w:24, h:24
					rad:6px, fs:1.4em, flexCenter
					c:#fff, bg:@[randomColor()]
					tr:0.25s
				)}
				label user name {css(
					abs, pe, x:0,p:20px 0, c:rgba(255,255,255,0.5), pl:60px
					fs:1em,
					tr:0.25s
				)}
			inputBox
				input {required}
				icon {class="ui-icon application_cascade", bg:@[randomColor()]}
				label password
			line <>
				forget password?
				<a>click heare</a>
			<>
			inputBox
				button submit
		end
	<init>
		alert('init')
	</init>
<css>

</css>	
		
##> *wave [리스트 선택 물결표시 기능] {
	bgColor: randomColor()
}
container {flexCenter}
	box
	box
	box
	box
	box
	box
	box
	box
	box
	<js>
		function test() {
			@[box].each(el=>{
				const c = '@[bgColor.light()]'
				$(el).on('mouseover')
			})
		}
	</js>
end
<css>
	@[box] {
		display:flex;
		width: 250px;
		color: @[light(100)];
	}
	@[box].hovered {
		display:flex;
		width: 250px
	}
</css>
