##>config { name=player }
	RUN_PATH = c:/temp/player
	
	@eval {
		src=cv('layout.test')
		page = pageLoad(src,'test','p1')
		print("src==$src", page)
	}
	
##> layout {name=player}
test {
	<page id="p1" module="player#test, player#impl">
		<player id="player">
		<hbox>
			<button id="load" text="동영상열기">
			<button id="stop" text="멈춤">
			<space>
		</hbox>
	</page>
	<page id="selectFile" module="selectFile">
		<splitter type="hbox">
			<tree id="tree">
			<grid id="grid">
		</splitter>
	</page>
}
##> module {name=selectFile}
	init() {
		tree=
	}

##> module {name=player#test}
	init() {
		@player = this.player
		@btnLoad = this.load
		@btnStop = this.stop
		event(btnLoad,'onClick', this.clickLoad)
		event(btnStop,'onClick', this.clickStop)
	}
	initPage() {
		print("xxxxxxxxx init page xxxxxxxx", player, btnLoad)
	}
##> module {name=player#imple}
	clickLoad() {
		
	}
	clickStop() {
		player.stop()
	}

p2=page('test:p2')
p2.open()
v=p2.get('v')
v.start('c:/temp/aa.m4a')
v.stop()

event(v, 'onStateChange', @player.stateChange)
event(v, 'onMessage', @player.message)

~~
<func>
	@player.stateChange() {
		state=this.playState()
		print("state change ....",state)
	}
	@player.message(line) {
		print("player message >> $line")
	}
</func>


~~
x=conf('path.mplayer', 'C:/app/mplayer/mplayer.exe', true)
	}
	
##> func { name=test }
	@vite.test() {
		p=cv('RUN_PATH')
		print("runpath == $p")
	}
	 