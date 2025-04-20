include('classes/common/etc')
c=cmd()
in=logWriter('runcmd-in')
out=logReader('runcmd-out')


pp=conf('python.path')
pp.add('/python.exe')

target = System.path()
target.add('/pyapps')

p=fv('#{pp} "#{target}/run_cmd.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
c.run(p)

~~
<widgets>
	<page id="p1">
		<editor id="e">
		<hbox>
			<button id="clear" text="clear">
			<space>
		</hbox>
	</page>
</widgets>
~~
page=page('test:p1')
page.open()
page[
	init() {
		@e=widget('e')
		@out=logReader('webview-out')
		@in = logWriter('webview-in')
		setEvent(widget('clear'), 'onClick', this.clickClear, this)
		this.moveTick = 0
		this.timer(100)
	}
    movePos() {
        rc=e.geo()
        this.mapGlobal(rc).inject(x,y,w,h)
        x=x.toInt()
        y=y.toInt()
        w=w.toInt()
        h=h.toInt()
        in.append("##>geo:$x,$y,$w,$h")
    }
	onTimer() {
        if(this.topMode) {
            this.movePos()
            this.topMode = false
        }
        if(this.moveTick) {
            dist = System.tick() - this.moveTick
            if(dist>500) {
                this.movePos()
                this.moveTick = 0
            }
            return
        }
        if(this.activationChange) {
			a=this.is('min')
			if(a) {
				this.minMode = true
			} else if(this.minMode) {
				in.append('##>show:')
				this.minMode = false
			}
			b=this.is('active')
            if(b) {
                in.append('##>top:')
                this.topMode = true
            }
			this.activationChange = false
			return
		}
		if(this.minMode) {
			in.append('##>hide:')
			this.minMode=false
		}
		s=out.timeout()
		if(s) e.append(s)
	}
	clickClear() {
		print("click clear")
		e.value('')
	} 
	onMove() {
		not(this.moveTick) {
			in.append('##>hide:')
		}
		this.moveTick = System.tick()
	}
]