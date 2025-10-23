<api>
	 btn3d() { return #[
		.btn3d {
			width: 140px;
			height: 50px;
			position: relative;
			background: none;
			outline: none;
			border: none;
			padding: 0;
			margin: 0;
		}
		.btn3d .top {
			width: 100%;
			height: 100%;
			background: rgb(255, 255, 238);
			font-family: poppins;
			font-size: 16px;
			color: rgb(36, 38, 34);
			display: flex;
			align-items: center;
			justify-content: center;
			border-radius: 7mm;
			outline: 2px solid rgb(36, 38, 34);
			transition: 0.2s;
			position: relative;
			overflow: hidden;
		}
		.btn3d .bottom {
			position: absolute;
			width: 100%;
			height: 100%;
			background: rgb(229, 229, 199);
			top: 10px;
			left: 0;
			border-radius: 7mm;
			outline: 2px solid rgb(36, 38, 34);
			z-index: -1;
		}
		.btn3d .bottom::before {
			position: absolute;
			content: "";
			width: 2px;
			height: 9px;
			background: rgb(36, 38, 34);
			bottom: 0;
			left: 15%;
		}
		.btn3d .bottom::after {
			position: absolute;
			content: "";
			width: 2px;
			height: 9px;
			background: rgb(36, 38, 34);
			bottom: 0;
			left: 85%;
		}
		.btn3d:active .top {
			transform: translateY(10px);
		}
		.btn3d::before {
			position: absolute;
			content: "";
			width: calc(100% + 2px);
			height: 100%;
			background: rgb(140, 140, 140);
			top: 14px;
			left: -1px;
			border-radius: 7mm;
			outline: 2px solid rgb(36, 38, 34);
			z-index: -1;
		}
		.btn3d .top::before {
			position: absolute;
			content: "";
			width: 15px;
			height: 100%;
			background: rgba(0, 0, 0, 0.1);
			transform: skewX(30deg);
			left: -20px;
			transition: 0.25s;
		}
		.btn3d:active .top::before {
			left: calc(100% + 20px);
		}
	]}
	3dText() {
		depth=param.get('depth') not(depth) depth=4
		type=param.get('type') not(type) type='left'
		if(type.eq('left')) {
			dx=-2, dy=2	
			sx=-1, sy=1, ex=-2, ey=2
		} else {
			dx=2, dy=2
			sx=1, sy=1, ex=2, ey=2
		}
		base = randomColor().lightColor(50)
		bgColor = getColor(base.lightColor(150))
		color=getColor(base), hoverColor=getColor(base.darkColor(80))
		ss=''
		while(n=1,depth) {
			if(n.gt(1)) ss.add(',')
			ss.add("${sx}px ${sy}px 0px ${color}, ${ex}px ${ey}px 0px ${color}")
			sx+=dx;
			sy+=dy;
			ex+=dx;
			ey+=dy;
		}
		classNm = 'letter'
	return #[
		.${classNm} {
			width: fit-content;
			height: fit-content;
			transform-style: preserve-3d;
			padding: 10px;
			color: ${bgColor};
			cursor: pointer;
		}
		.${classNm} span {
			display: block;
			font-size: 60px;
			font-weight: 800;
			text-shadow: ${ss};
		}
		.${classNm}:hover {
			color: ${hoverColor}; 
		}
	 ]}
</api>