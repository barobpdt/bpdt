##>config { name=vite_project }
	RUN_PATH = c:/temp/vite
	USE_RESET = true
	@eval {
		src=cv('source.runCommand')
		print("path===", src.size())
		@vite.test()
	}
	player {
<widgets>
	<page id="p1">
		<player id="player">
		<hbox>
			<button id="ok" text="ok">
		</hbox>
	</page>
</widgets>

pp=p.get('player')
pp.start('c:/temp/aa.m4a')
pp.stop()

~~
<widgets>
	<page id="p2">
		<video id="v">
		<hbox>
			<button id="ok" text="ok">
			<space>
		</hbox>
	</page>
</widgets>
~~
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
	 