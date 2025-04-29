##> 파이션 실행하기

	c=cmd()
	in = logWriter('runcmd-in')
	out = logReader('runcmd-out')

	pythonPath=conf('python.path')
	pythonPath.add('/python.exe')

	sourcePath = conf('include.path')
	sourcePath.add('/classes/pyapps')

	cmd=fv('#{pythonPath} "#{sourcePath}/run_cmd.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
	
	pip=fv('#{pythonPath} -m pip install zipfile')
	pip=fv('#{pythonPath} -m pip install pywin32==306')
    cmdInstall = fv('#{pythonPath} "#{conf("python.path")}/Scripts/pywin32_postinstall.py" -install')
~~
	c.run(pip)
	c.run(cmd)

	in.write('##>quit:')
    
##> 소스실행

src = #[
import zipfile
with zipfile.ZipFile('${path}') as zip:
    arr = zip.infolist()
    for cur in arr:
        cur.filename = cur.filename.encode('cp437').decode('euc-kr')
    log(arr)

]
ss=fv('##> exec:#{src}')
in.write(ss)    
    

##> 웹브라우저 실행 mywebview

c=cmd('web')
in = logWriter('webcmd-in')
out = logReader('webcmd-out')
cmd=fv('#{pythonPath} "#{sourcePath}/mywebview.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
~~
c.run(cmd)

in.write('##>quit:')
    
##> 웹브라우저 실행 webview
    c=cmd('web')
 	cmd=fv('#{pythonPath} "#{sourcePath}/webview.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
    c.run(cmd)

##> 로그보기
Cf.sourceApply(#[
    <widgets>
        <page id="logViewer">
            <editor id="e">
            <hbox>
                <button id="clear" text="clear">
                <space>
            </hbox>
        </page>
    </widgets>
])
    p=page('test:logViewer')
    p[
        init() {
            @out = logReader('runcmd-out')
            @e=widget('e')
            setEvent(widget('clear'), 'onClick', this.clickClear, this)
            this.timer(200)
            this.open()
        }
        onTimer() {
            s=out.timeout()
            if(s) {
                e.append(s)
            }
        }
        clickClear() {
            e.value('')
        }
    ]
    p.init()

##> 클립보드 캡쳐
root=Cf.rootNode()
System.watcherClipboard(
setEvent(root, 'onChanageClipboard', @test.clipZipFileCopy)
~~
<func>
	@test.clipZipFileCopy(a,b,c) {
		not(a.eq('text')) return;
		not(b.start('file:///')) return;
		path = b.value(8)
		ext=right(path,'.').lower()
		print("copy file =>", path, ext)
		not(ext.eq('zip')) return;
	}
</func>


##> 
