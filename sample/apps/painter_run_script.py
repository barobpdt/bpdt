##
c[
	onDraw(dc) {
		canvas = this
		rc=this.rect()
		not( rc.eq(canvas.rcCanvas) ) {
			this.updateCanvas(rc)
		}
		dc.fill('#fff')
		this.inject(rcCanvas, gap, @sx, @sy, @wa, @ha)
		startX = sx
		while(h, ha, a) {
			sx=startX
			while(w,wa,b) {
				rcBox=rc(sx,sy,w,h)
				dc.rectLine(rcBox, 0, '#555', 1, 'dot')
				sx+=w+gap;
			}
			sy+=h+gap;
		}
	}
	updateCanvas(rc) {
		print("@@ update canvas rect:$rc")
		not( this.gap ) this.gap=2
		this.inject(rcCanvas, gap )
		rows=4
		cols=2
		rc.inject(sx,sy,w,h)
		if( gap ) {
			start=gap/2;
			tw = X[w-(gap*rows)]
			th = X[h-(gap*cols)]
		} else {
			start=0
			tw=w
			th=h
		}
		print("xxxxxxx", gap, start, tw, th)
		this.addArray('@wa').recalc(tw,rows)
		this.addArray('@ha').recalc(th,cols)
		this.set('@sx', start)
		this.set('@sy', start)
		this.rcCanvas = rc
	}
]
##
py=_s('${@python.path}/python')
sp = _s('${@sample.path}/apps')

console = @job.cmdRun('cd')
@job.cmdRun(console, _s('$py "$sp/painter02.py"'))


## 이미지 배경제거 라이브러리 설치
@python.cmdPip('pip install rembg')
@python.cmdPip('pip install onnxruntime')

## 이미지 배경제거 스크립트 실행
c=@python.cmdExec(#[##> exec:
from rembg import remove, new_session 
from PIL import Image
img_path = "c:/bpdt/data/sprites/ani02.jpg"
out_path = 'c:/bpdt/data/sprites/ani02.png'
img = Image.open(img_path)
model_name = "isnet-general-use"
session = new_session(model_name)
out = remove(img, session=session)
out.save(out_path)
log(f'print: saveImagePath={out_path}')
])

## sprite 이미지 조회 및 소스열기 
@python.cmdExec(#[##> exec:
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.action_chains import ActionChains
global driver
options = Options()
options.add_experimental_option("detach", True)
options.add_argument("--window-size = x,y")
options.add_argument('--disable-popup-blocking')
 
driver = webdriver.Chrome(options=options)
driver.get('https://www.google.com/') #?q=[search text]
log(f'print: driver=>{driver}')
])

@python.cmdExec(#[##> exec:
driver.get('https://www.freepik.com/search?format=search&img=1&last_filter=img&last_value=1&query=2d+sprites')
log(f'openEditor: {driver.page_source}')
])

<func>
	@parse.openEditor(&s) {
		node=this
		node.fullPath='c:/TEMP/sprites.html'
		fileWrite(node.fullPath, s)
		@job.addPost('openEditor',node)
}
	@job.openEditor#post(node) {
		if(node.fullPath ) {
			cmd= _s('notepad "${node.fullPath}"')
			print("openEditor POST: $node CMD:$cmd")
			@job.cmdRun(cmd)
		}
	}
</func>

## 이미지 다운로드 
w=Baro.web()
w.download('https://img.freepik.com/free-photo/8-bits-characters-gaming-assets_23-2151143769.jpg?semt=ais_hybrid&amp;w=740&amp;q=80'
, 'c:/bpdt/data/sprites/ani02.jpg', @proc.print)
print("w=>$w")