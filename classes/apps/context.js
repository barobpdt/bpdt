class context { 
	buttons=mdc('buttons', 'btn-bg02.png')
	drawButton(dc) {
		not(buttons) return;
		not(checkMember('sx')) {
			sx=0,sy=0;
		}
		rcImg=buttons.var(rect)
		dc.image(rcImg, buttons, sx, sy)
	}
}
