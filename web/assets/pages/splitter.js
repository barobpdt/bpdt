(function() {
loadStyle(`
/* 드래그 중일 때 적용할 스타일 */
.splitter.dragging .drag-handle {
	background-color: #4a90e2;
}

.splitter.dragging .drag-handle-button {
	opacity: 0;
}
.splitter .drag-handle {
	position: absolute;
	left: 0;
	width: 10px;
	height: 100%;
	background-color: #ddd;
	cursor: ew-resize;
	z-index: 20;
	transition: transform 0.45s ease, background-color 0.2s;
	display: flex;
	align-items: center;
	justify-content: center;
	cursor: col-resize;
}

.splitter .drag-handle:hover {
	background-color: #aaa;
}

.splitter .drag-handle-button {
	position: absolute;
	width: 16px;
	height: 40px;
	background-color: #fff;
	border: 1px solid #ddd;
	border-radius: 8px;
	cursor: pointer;
	display: flex;
	align-items: center;
	justify-content: center;
	transition: background-color 0.2s;
	z-index: 21;
}

.splitter .drag-handle-button:hover {
	background-color: #f0f0f0;
}

.splitter .drag-handle-button i {
	font-size: 12px;
	color: #666;
	transition: transform 0.3s ease;
}

.splitter .side-panel.collapsed + .drag-handle i {
	transform: rotate(180deg);
}

.splitter .side-panel.collapsed .drag-handle {
	transform: translateX(300px);
	border-left: 1px solid #ddd;
	cursor: pointer;
}
`)	
	function initLayout() {
		const left = $('<div class="left"/>').css(getCss('vbox',{width:200})).appendTo(this.contentEl)
		const handle = $(`
		<div class="drag-handle">
			<button class="drag-handle-button">
				<i class="fas fa-chevron-left"></i>
			</button>
		</div>
		`).appendTo(this.contentEl)
		const right = $('<div class="right"/>').css(getCss('vbox',{flex:1})).appendTo(this.contentEl)
		// setTimeout(()=>initSplitter().setDragPos(),50)
		// container, left, right, handle, handleBtn
		initSplitter(
			this.contentEl,
			this.findEl('.left'),
			this.findEl('.right'),
			this.findEl('.drag-handle'),
			this.findEl('.drag-handle-button')
		).setDragPos()
	}
	const initSplitter = (container, left, right, handle, handleBtn) => { 
		const dragHandleSize = 10
		let isDragging = false
		let startX = 0
		let startWidth = 0		
		let prevSize = 0
		let mousedownTick = 0
		container.addClass('splitter')
		function setLeftPanelPos(leftWidth) {
			const cw = container.width()
			left.width(leftWidth)
			right.width(cw - leftWidth - dragHandleSize)
			handle.css('left', leftWidth+'px') 
		}
		// 패널 접기 함수
		function collapsePanel() {
			left.addClass('collapsed')
			setLeftPanelPos(0)
		}
		
		// 패널 펼치기 함수
		function expandPanel() {
			left.removeClass('collapsed')
			console.log('expand ', prevSize)
			setLeftPanelPos(prevSize<200 ? 300: prevSize)
		}
		
		
		// 드래그 핸들 버튼 클릭 이벤트
		handleBtn.on('click', e => {
			mousedownTick = 0
			// e.stopPropagation();
			if ( left.hasClass('collapsed') ) {
				expandPanel();
			} else {
				collapsePanel()
			}
			e.preventDefault();
		});
		
		// 드래그 시작
		handle.on('mousedown', e => {
			mousedownTick = new Date().getTime()
			setTimeout(() => {
				if( mousedownTick==0 ) {
					return
				}
				if (!isDragging) {  // 패널이 펼쳐져 있을 때만 드래그 가능
					isDragging = true;
					container.addClass('dragging')
					startX = e.clientX;
					startWidth = left.width()
					
					document.addEventListener('mousemove', handleMouseMove);
					document.addEventListener('mouseup', handleMouseUp);
				}
			}, 250)
			e.preventDefault();
		});
		
		// 마우스 이동 처리
		function handleMouseMove(e) {
			if (!isDragging) return
			const width = startWidth + (e.clientX - startX)
			setLeftPanelPos(width)
			
			if( width < 150 ) {
				handleMouseUp()
				collapsePanel()
			}
		}
		
		// 마우스 업 처리
		function handleMouseUp() {
			isDragging = false;
			container.removeClass('dragging');
			document.removeEventListener('mousemove', handleMouseMove)
			document.removeEventListener('mouseup', handleMouseUp)
			setDragPos()
		}
		function setDragPos() {			
			if( right.css('flex')!=='none') {
				right.css('flex','none')
			}
			const width = left.width()
			setLeftPanelPos(width) 
			prevSize = width
			
			console.log('@@ set pos ', width, right[0])
		}
		return {setDragPos}
	}
	const pageImpl = {
		initPage: function() {
			clog('## splitter init page this >>',this) 
			initLayout.call(this)
		}
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContent')
		, children:[
			{tag:'div',style:getCss('full',{height:30,background:getRandomColor()}), className:'appTop'},
			{tag:'div',style:getCss('pageContent'), className:'appContents', content:true},
			{tag:'div',style:{height:30,background:getRandomColor()}, className:'appFooter'},
		]
	}
	const pageInfo = {id:'splitter', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()