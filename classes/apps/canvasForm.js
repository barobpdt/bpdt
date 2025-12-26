/*
https://github.com/ImageMagick/ImageMagick/blob/main/scripts/txt2html
https://im.snibgo.com/

pp=@test.page()
x=pp.setLayout('tag:canvas, scroll:true')
pp.open()
pp.size(400, 250)
c=pp.get('canvas')
class(c,'widget')
c.var(scrollUse, false)

c[
	initClass() {
		class(this,'widget')
		@forms=this.addNode('node.forms')
		this.setEvent('onDraw', this.draw)
	}
	draw(dc, rc) {
		if(this.firstCall) {
			return;	
		}
		scroll=this.var(scroll)
		if(scroll) {
			if(this.var(scrollUse)) {
				if(this.rcCanvas) {
					print("xxxxx canvas rect ok xxxxxx", this.rcCanvas, rc)
				} else {
					this.updateCanvas(dc, rc)
					this.rcCanvas=rc
					scroll.widgetResizable(false)
				}
			} else {
				this.var(scrollUse, true)
				scroll.widgetResizable(true)
			}
		}
		if(forms.img) {
			dc.image(forms.img, rc)
		} else {
			dc.text("form 미설정 (영역:$rc)". "center")
		}
	}
	updateCanvas(dc, rc) {
		print("update canvas rc==$rc")	
	}
]
	
## grid header click ~~
pp=@test.page()
pp.setLayout('tag:grid, headerUse:true')
gg=pp.get('grid')
gg.model('check, id, text')
root=gg.rootNode()
root.addNode().with(id:id1,text:name1)
root.addNode().with(id:id2,text:name2)
gg.is('sortEnable', true)
gg.update()
class(gg,'grid')
pp.open()
 
clr=randomColor()
gg.var(bgColor, clr.lightColor(80))

~~
gg[
	gridDrawNode(dc,rc, node, field) {		
		if(field.eq('check')) {
			if(node.flag(NODE.add)) {
				dc.fill(rc.incr(2),'#eaa')
			} else if(node.flag(NODE.modify)) {
				dc.fill(rc.incr(2),'#aae')
			}
			if(node.flag(NODE.check)) {
				dc.image(rc.center(24,24), 'icons:roundCheck')
			} else {
				dc.image(rc.center(24,24), 'icons:roundCheck_blank')
			} 
		} else {
			dc.text(rc.incrX(2), node.get(field))
		}
	}
]
~~
gg[
	onHeaderClick() {
		grid=this
		idx=grid.var(clickIndex)
		print("idx==$idx")
		if(idx==0) {
			if(grid.var(checkedAll)) {
				bchk=false				
			} else {
				bchk=true
			}
			grid.var(checkedAll, bchk)
			while(cur, grid.rootNode()) {
				cur.flag(NODE.check, bchk)
			} 
		}
	}
	onDraw(dc, node, index, state) {
		grid=this
		field=grid.field(index)
		dc.mode()
		rc=grid.drawState(dc, node, state, index, field )
		this.gridDrawNode(dc, rc, node, field)
	}
	gridDrawNode(dc,rc, node, field) {		
		if(field.eq('check')) {
			if(node.flag(NODE.add)) {
				dc.fill(rc.incr(2),'#eaa')
			} else if(node.flag(NODE.modify)) {
				dc.fill(rc.incr(2),'#aae')
			}
			if(node.flag(NODE.check)) {
				dc.image(rc.center(24,24), 'icons:roundCheck')
			} else {
				dc.image(rc.center(24,24), 'icons:roundCheck_blank')
			} 
		} else {
			dc.text(rc.incrX(2), node.get(field))
		}
	}
	onMouseDown(p,a) {
		print("mouse down ", p, this.id)
	}
	onDrawHeader(dc, text, index, order) {
		rc=dc.rect(), fields=this.fields();
		last=fields.childCount()-1;
		if(last.eq(index)) {
			dc.rectLine(rc, 4,'#a0a0a0');
		} else {
			dc.rectLine(rc, 34,'#a0a0a0');
		}
		if( index.eq(sortIndex) ) {
			if(order) {
				icon="vicon:bullet_arrow_up";
			} else {
				icon="vicon:bullet_arrow_down";
			}
			rcIcon=rc.rightCenter(16,16,-5);
			dc.text(rc.incrX(10), text );
			dc.image(rcIcon, icon);
		} else {
			dc.text(rc, text, 'center');
		}
	}
	drawState(dc, node, state, index, field) {
		last=this.columnCount()-1;
		rc=dc.rect();
		clr=this.var(bgColor)
		not(clr) clr=color("#c96");
		ty=when(last.eq(idx), 24, 234);
		if( state & STYLE.Selected ) {
			if(field.eq('chk','status')) {
				dc.rectLine(rc,3, clr.lightColor(120), 1 );
			} else {
				dc.fill(rc, '#aaa');
				dc.rectLine(rc,3, clr.lightColor(120), 1 );
				dc.pen(clr.lightColor(220));
			}
		} else if( state & STYLE.MouseOver ) {
			dc.fill(rc,'#def' );
			dc.pen(clr.darkColor(150));
			dc.rectLine(rc,ty);
		} else {
			dc.fill(rc,'#ffffff');
			dc.rectLine(rc,ty,'#ddd');
			dc.pen(clr.darkColor(200));
		}
		return rc.incrX(4);
	}
]

~~
@draw.saveIcon('roundCheck')

 

~~

s=''
imgFile=Cf.val('c:\temp\icons\checkRound1.png')
s.add("magick ",
	Cf.jsValue(imgFile),
	" -fuzz 6% -transparent white ",
	Cf.jsValue(imgFile)
)
print("s==>$s")
cmd.run(s)


~~
cmd=cmd()
fo=Baro.file()
fo.list('C:\WORK\icons', func(info) {
	while(info.next()) {
		info.inject(type, name, fullPath, ext)
		e=ext.lower()
		if(e.eq('png')) {
			a=Cf.jsValue(fullPath)
			str="magick $a -fuzz 6% -transparent white $a"
			cmd.cmdAdd(str)
			print("xxxxxxx str==$str")
		}
	}
})

cmd.cmdRun()


~~
gg[
	
]



~~
p=@test.page('canvas')
c=p.get('canvas')
~~
c[
	test() {
		img=System.clipboard('image')
		aa=Cf.imageLoad(img)
		dc=mdc('aa', aa)
		temp=mdc('temp', 160, 160)

		dc.image(aa)
		aa.saveImage('c:/temp/aa.png')
	}
	onDraw(dc, rc) {
		img=mdc('aa')
		rcImg=img.rect()
		dc.image(img, rc)
		while(a, vbox(rcImg,2), row ) {
			while(b, hbox(a,3), col) {
				rcIcon=b.incrXY(20,14)
				dc.rectLine(rcIcon, 0, '#ea9', 1, 'dash')
				filePath="c:/temp/icons_aa_${row}_${col}.png"
				print("xxxxx filePath== $filePath ")
				not(isFile(filePath)) {					
					temp=mdc('temp')
					temp.fill()
					temp.image(img, rcIcon)
					aa=temp.var(img)
					print("xxxxxxxxxx", temp, rcIcon, aa)
					aa.saveImage(filePath)
				}
			}
		}
	}
]


p.open()




#!/bin/sh

# Known issues: does not support non-English characters
fbbkg_name=$(basename -a "$1")

echo "Enter post text."
read -r fbbkg_text
[ -z "$fbbkg_text" ] && echo "Please enter some text"
convert "$1" -gravity center -crop 2000x2000:0:0 -resize 1080x1080 fbbkg_background.jpg
convert -background transparent -gravity center -font Source-Sans-3-Black -size 915x600 -fill white -strokewidth 2 -stroke black caption:"$fbbkg_text" fbbkg_text.png
composite -gravity center fbbkg_text.png fbbkg_background.jpg "${fbbkg_name%.*}-fb.jpg"
rm fbbkg_background.jpg fbbkg_text.png

#!/bin/bash

# Quick and dirty broll generator. Requires ffmpeg.

echo "Enter start time:"
read -r start
echo "(Optional) Enter the length into the video:"
read -r stop
echo "Enter clip length:"
read -r length
echo "Enter name of clips:"
read -r name

if [ -z "$stop" ]
then
	case "$1" in
		*mkv)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" "${name}"%02d.mkv;;
		*mp4)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" "${name}"%02d.mp4;;
		*ogv)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" "${name}"%02d.ogv;;
		*webm)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" "${name}"%02d.webm;;
		*)
			echo "Please enter a video file.";;
	esac
else
	case "$1" in
		*mkv)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" -t "${stop}" "${name}"%02d.mkv;;
		*mp4)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" -t "${stop}" "${name}"%02d.mp4;;
		*ogv)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" -t "${stop}" "${name}"%02d.ogv;;
		*webm)
			ffmpeg -i "$1" -vcodec copy -an -f segment -segment_time "${length}" -reset_timestamps 1 -ss "${start}" -t "${stop}" "${name}"%02d.webm;;
		*)
			echo "Please enter a video file.";;
	esac
fi



파일포맷변경

convert image_org.gif  image_out.jpg
[설명] image_org.gif  이미지를 image_out.jpg로 바꾼다.
 
convert image_org.png  image_out.jpg
[설명] image_org.png  이미지를 image_out.jpg로 바꾼다. 

 

 

확대,축소,리사이즈

convert image_org.jpg -resize 25%x25% -quality 100 image_out.jpg
[설명] image_org.jpg  이미지를 1/4 축소한 image_out.jpg로 바꾼다.
 
convert image_org.jpg -resize 800x600 -quality 100 image_out.jpg
[설명] image_org.jpg  이미지를 800x600픽셀로 리사이즈 하지만 비율을 유지하며 큰사이즈 비율 기준으로 image_out.jpg를 생성한다.
 
convert image_org.jpg -resize 800x600\! -quality 100 image_out.jpg
[설명] image_org.jpg  이미지를 800x600픽셀로 강제적으로 바꾸어 image_out.jpg를 생성한다.
 
[설명] "-quality 100"이란 옵션을 주면 품질을 최대한 좋게한다.
 
 

 

회전

convert image_org.png -matte -background none -rotate 90  image_out.png
[설명] 이미지를 90도 회전하고 나머지 영역은 투명하게한다.

convert image_org.png -matte -background none -rotate -15  image_out.png
[설명] 이미지를 -15도 회전하고 나머지 영역은 투명하게한다.
 

 


좌우반전,상하반전

convert -flop image_org.jpg  image_out.jpg
[설명] image_org.jpg  이미지를 좌우반전시켜 image_out.jpg 이미지를 생성한다.

convert -flip image_org.jpg  image_out.jpg
[설명] image_org.jpg  이미지를 상하반전시켜 image_out.jpg 이미지를 생성한다.

 
 


흑백,갈색 효과

convert image_org.jpg -colorspace gray image_out.jpg
[설명] image_org.jpg  이미지를 흑백효과를 적용하고 image_out.jpg 이미지를 생성한다.

convert image_org.jpg -sepia-tone 80% image_out.jpg
[설명] image_org.jpg  이미지를 갈색효과를 적용하고 image_out.jpg 이미지를 생성한다.
 

 


밝게,어둡게

convert image_org.jpg -sigmoidal-contrast 3,0% image_out.jpg
[설명] image_org.jpg  이미지를 밝게하여 image_out.jpg 이미지를 생성한다.

convert image_org.jpg -sigmoidal-contrast 3,100% image_out.jpg
[설명] image_org.jpg  이미지를 어둡게하여 image_out.jpg 이미지를 생성한다.
 

 


자르기(crop)

convert image_org.jpg -crop 800x600+10+20  image_out.jpg
[설명] image_org.jpg를 Left 10픽셀 Top 20픽셀 부터 800x600픽셀까지  자르고 그 결과로 image_out.jpg 이미지를 생성한다.

convert image_org.jpg -crop 800x600+10-30  image_out.jpg
[설명] image_org.jpg를 Left 10픽셀 Top -30픽셀 부터 800x600픽셀까지  자르고 그 결과로 image_out.jpg 이미지를 생성한다.


 

 
캔버스생성

convert -size 800x600 xc:white image_out.jpg
[설명] 800x600픽셀인 흰색 image_out.jpg 이미지를 생성한다.

convert -size 800x600 xc:skyblue image_out.gif
[설명] 800x600픽셀인 하늘색 image_out.jpg 이미지를 생성한다.

convert -size 800x600 xc:none image_out.png
[설명] 800x600픽셀인 투명 image_out.png 이미지를 생성한다.


 
 

글자이미지생성

convert -background white -fill black -font batang.ttf -pointsize 36 label:"Test\n한글" image_out.png
[설명] "Test\n한글"이란 글자로 image_out.png 이미지를 생성한다. (이미지배경은 흰색, 글자색은 검정색, 폰트는 바탕, 폰트사이즈는 36pt)

 


 
이미지합성

composite -dissolve 60 -geometry +50+100 image_temp.png image_org.jpg image_out.jpg
[설명] image_temp.png 이미지를 투명도 60%로 하여 image_org.jpg의 50,100 픽셀 좌표에 올려 합성후 image_out.jpg 이미지를 생성한다.



*/
class canvasForm { 
	initClass() {
		@forms=this.addNode('node.forms')
		@formDc=null
		this.setEvent('onDraw', this.drawForm)
	}
	setForm(s) {
		forms.removeAll(true)
		forms.parseJson(s)
		this.var(formSet, true)
		this.update()
	}
	drawForm(dc, rc) {
		if( this.var(formSet)) {
			this.updateForm(dc, rc)
			this.var(formSet, false)
		}
	}
	updateForm(dc, rc) {
		rc.inject(cx,cy,cw,ch)
		while(cur, forms, n) {
			cur.inject(tag, space, w, h)
		} 
		if()
	}
	tagLabelInput(dc,rc) {
		not(this.rcBase) this.rcBase=rc
		not(rc.eq(this.rcBase)) return;
		 rcBox=rc.center(250,30)
		 dc.fill()
		 dc.font('size:10, weight:bold')
		 text='사용자 아이디'
		 dc.textSize(text).inject(tw)
		 tw+=30;
		 a=hbox(rcBox, "$tw,*")
		 while(rc,a,n) {
		 	if(n==0) {
		 		myLabel(dc, rc, text)
		 	} else {
		 		input.move(rc.incrYH(1) )
		 		input.show()
		 	}
		 }
	}
	roundLabel(dc, rc, text, bg, rad) {
		not(rad) rad=20
		lt=rc.lt(), lb=rc.lb().incrY(-rad)
		a=rc(lt,rad,rad), b=rc(lb,rad,rad)
		dc.drawPath(rc.rt(), a.tc(),
			'arc',a,90,90, b.lc(), 
			'arc',b,180,90, rc.rb(), 
			true, bg
		)
		dc.pen(bg.lightColor(200))
		dc.text(rc,text,'center')
	}
} 