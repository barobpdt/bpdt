(function() {
	function initForm(page, content) {
		content.addClass('form_buttons')
		content.html(`
		<div class="buttons-grid">
            <!-- Button 16 -->
            <div class="button-card">
                <h3 class="button-title">Particle Explosion</h3>
                <button class="myBtn btn-particle">Explode</button>
            </div>
            
            <!-- Button 17 -->
            <div class="button-card">
                <h3 class="button-title">Liquid Fill Effect</h3>
                <button class="myBtn btn-liquid">Liquid Fill</button>
            </div>
            
            <!-- Button 18 -->
            <div class="button-card">
                <h3 class="button-title">Holographic Effect</h3>
                <button class="myBtn btn-holographic">Hologram</button>
            </div>
            
            <!-- Button 19 -->
            <div class="button-card">
                <h3 class="button-title">Morphing Shape</h3>
                <button class="myBtn btn-morph">Morph</button>
            </div>
            
            <!-- Button 20 -->
            <div class="button-card">
                <h3 class="button-title">Sliding Panels</h3>
                <button class="myBtn btn-slide-panels">Slide Panels</button>
            </div>
            
            <!-- Button 21 -->
            <div class="button-card">
                <h3 class="button-title">Glitch Effect</h3>
                <button class="myBtn btn-glitch" data-text="Glitch">Glitch</button>
            </div>
            
            <!-- Button 22 -->
            <div class="button-card">
                <h3 class="button-title">Matrix Style</h3>
                <button class="myBtn btn-matrix">Matrix</button>
            </div>
            
            <!-- Button 23 -->
            <div class="button-card">
                <h3 class="button-title">Gravity Effect</h3>
                <button class="myBtn btn-gravity">Gravity</button>
            </div>
            
            <!-- Button 24 -->
            <div class="button-card">
                <h3 class="button-title">Confetti Animation</h3>
                <button class="myBtn btn-confetti">Confetti</button>
            </div>
            
            <!-- Button 25 -->
            <div class="button-card">
                <h3 class="button-title">Cyberpunk Style</h3>
                <button class="myBtn btn-cyberpunk">Cyberpunk</button>
            </div>
            
            <!-- Button 26 -->
            <div class="button-card">
                <h3 class="button-title">Jelly Animation</h3>
                <button class="myBtn btn-jelly">Jelly</button>
            </div>
            
            <!-- Button 27 -->
            <div class="button-card">
                <h3 class="button-title">Spotlight Effect</h3>
                <button class="myBtn btn-spotlight">Spotlight</button>
            </div>
            
            <!-- Button 28 -->
            <div class="button-card">
                <h3 class="button-title">Split Text Effect</h3>
                <button class="myBtn btn-split" data-text="Split Text"><span>S</span><span>p</span><span>l</span><span>i</span><span>t</span></button>
            </div>
            
            <!-- Button 29 -->
            <div class="button-card">
                <h3 class="button-title">Gradient Border</h3>
                <button class="myBtn btn-gradient-border">Gradient Border</button>
            </div>
            
            <!-- Button 30 -->
            <div class="button-card">
                <h3 class="button-title">Infinite Loop</h3>
                <button class="myBtn btn-infinite">Infinite</button>
            </div>
        </div>`)
		setFormEvent(page, content)
	}
	const pageImpl = {
		initPage: function() { initForm(this, this.contentEl) }
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContent', {overflow:'auto'})
		, content: true
	}
	const pageInfo = {id:'form_buttons', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()

function setFormEvent(page, contentEl) {
    $('.form_buttons .myBtn').each((i, btn)=>{
		$(btn).on('click', ()=>{
			$(btn).addClass('active').css({transform: 'scale(0.95)'});
			setTimeout(()=>{
				clog('timeout btn=>', btn)
				$(btn).css({transform:''}).removeClass('active')
			}, 200);
		})
	})
}

loadStyle(`
.form_buttons {
	background:#fff;
}
.buttons-grid {
	display: grid;
	grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
	gap: 30px;
	width: 100%;
}
.button-card {
	display: flex;
	flex-direction: column;
	justify-content: center;
	padding: 4px 40px
}
.myBtn {
	position: relative;
	padding: 15px 30px;
	border: none;
	border-radius: 8px;
	font-size: 16px;
	font-weight: 600;
	cursor: pointer;
	overflow: hidden;
	transition: all 0.4s ease;
	outline: none;
	color: white;
	text-transform: uppercase;
	letter-spacing: 1px;
	display: flex;
	align-items: center;
	justify-content: center;
	min-width: 180px;
}

/* 16. Particle Explosion Button */
.btn-particle {
	background: linear-gradient(45deg, #ff6b6b, #ffa500);
	position: relative;
}

.btn-particle::before {
	content: '';
	position: absolute;
	width: 20px;
	height: 20px;
	background: rgba(255, 255, 255, 0.7);
	border-radius: 50%;
	transform: scale(0);
	animation: particle 1.5s infinite;
}

@keyframes particle {
	0% {
		transform: scale(0);
		opacity: 1;
	}
	100% {
		transform: scale(4);
		opacity: 0;
	}
}

.btn-particle:hover::before {
	animation: none;
}

.btn-particle:hover::after {
	content: '';
	position: absolute;
	width: 100%;
	height: 100%;
	background: radial-gradient(circle, rgba(255,255,255,0.8) 0%, transparent 70%);
	border-radius: 50%;
	transform: scale(0);
	animation: explode 0.5s forwards;
}

@keyframes explode {
	0% {
		transform: scale(0);
		opacity: 1;
	}
	100% {
		transform: scale(4);
		opacity: 0;
	}
}

/* 17. Liquid Fill Button */
.btn-liquid {
	background: transparent;
	border: 2px solid #00b4db;
	color: #00b4db;
	position: relative;
	z-index: 1;
	overflow: hidden;
}

.btn-liquid::before {
	content: '';
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	background: #00b4db;
	clip-path: polygon(0 0, 100% 0, 100% 0, 0 0);
	transition: clip-path 0.6s ease;
	z-index: -1;
}

.btn-liquid:hover::before {
	clip-path: polygon(0 0, 100% 0, 100% 100%, 0 100%);
}

.btn-liquid:hover {
	color: white;
}

/* 18. Holographic Button */
.btn-holographic {
	background: linear-gradient(90deg, #ff00cc, #3333ff, #00ccff, #ffcc00, #ff00cc);
	background-size: 400% 400%;
	animation: hologram 4s linear infinite;
	border: none;
	color: white;
	text-shadow: 0 0 5px rgba(0,0,0,0.5);
}

@keyframes hologram {
	0% {
		background-position: 0% 50%;
	}
	100% {
		background-position: 400% 50%;
	}
}

/* 19. Morphing Button */
.btn-morph {
	background: #9c27b0;
	border-radius: 50px;
	animation: morph 3s infinite alternate;
}

@keyframes morph {
	0% {
		border-radius: 50px;
	}
	50% {
		border-radius: 10px;
	}
	100% {
		border-radius: 30px 5px 30px 5px;
	}
}

.btn-morph:hover {
	animation-play-state: paused;
}

/* 20. Sliding Panels Button */
.btn-slide-panels {
	background: transparent;
	border: 2px solid #ff6b6b;
	color: #ff6b6b;
	position: relative;
	overflow: hidden;
}

.btn-slide-panels::before,
.btn-slide-panels::after {
	content: '';
	position: absolute;
	width: 50%;
	height: 100%;
	background: #ff6b6b;
	transition: transform 0.5s ease;
	z-index: -1;
}

.btn-slide-panels::before {
	top: 0;
	left: 0;
	transform: translateX(-100%);
}

.btn-slide-panels::after {
	top: 0;
	right: 0;
	transform: translateX(100%);
}

.btn-slide-panels:hover::before {
	transform: translateX(0);
}

.btn-slide-panels:hover::after {
	transform: translateX(0);
}

.btn-slide-panels:hover {
	color: white;
}

/* 21. Glitch Effect Button */
.btn-glitch {
	background: #1a1a1a;
	color: #fff;
	position: relative;
}

.btn-glitch::before,
.btn-glitch::after {
	content: attr(data-text);
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	padding: 15px 30px;
	border-radius: 8px;
}

.btn-glitch::before {
	left: 2px;
	text-shadow: -2px 0 red;
	background: #1a1a1a;
	animation: glitch-1 2s infinite linear alternate-reverse;
	clip-path: polygon(0 0, 100% 0, 100% 35%, 0 35%);
}

.btn-glitch::after {
	left: -2px;
	text-shadow: -2px 0 blue;
	background: #1a1a1a;
	animation: glitch-2 3s infinite linear alternate-reverse;
	clip-path: polygon(0 65%, 100% 65%, 100% 100%, 0 100%);
}

@keyframes glitch-1 {
	0% {
		transform: translateX(0);
	}
	100% {
		transform: translateX(-2px);
	}
}

@keyframes glitch-2 {
	0% {
		transform: translateX(0);
	}
	100% {
		transform: translateX(2px);
	}
}

/* 22. Matrix Button */
.btn-matrix {
	background: #000;
	color: #0f0;
	border: 1px solid #0f0;
	font-family: 'Courier New', monospace;
	position: relative;
	overflow: hidden;
}

.btn-matrix::before {
	content: '';
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	background: linear-gradient(transparent 70%, rgba(0, 255, 0, 0.1) 100%);
	animation: matrix-rain 3s linear infinite;
}

@keyframes matrix-rain {
	0% {
		background-position: 0 0;
	}
	100% {
		background-position: 0 100%;
	}
}

/* 23. Gravity Button */
.btn-gravity {
	background: #ff4757;
	box-shadow: 0 10px 0 #c23636;
	transform: translateY(0);
	transition: all 0.2s ease;
}

.btn-gravity:hover {
	transform: translateY(5px);
	box-shadow: 0 5px 0 #c23636;
}

.btn-gravity:active {
	transform: translateY(10px);
	box-shadow: 0 0 0 #c23636;
}

/* 24. Confetti Button */
.btn-confetti {
	background: linear-gradient(45deg, #ff9a9e, #fad0c4);
	color: #333;
	position: relative;
	overflow: hidden;
}

.btn-confetti::before {
	content: '';
	position: absolute;
	width: 20px;
	height: 20px;
	background: #ff6b6b;
	border-radius: 50%;
	animation: confetti 1.5s infinite;
}

.btn-confetti::after {
	content: '';
	position: absolute;
	width: 15px;
	height: 15px;
	background: #4ecdc4;
	border-radius: 50%;
	animation: confetti 1.8s infinite 0.5s;
}

@keyframes confetti {
	0% {
		transform: translateY(0) rotate(0);
		opacity: 1;
	}
	100% {
		transform: translateY(100px) rotate(360deg);
		opacity: 0;
	}
}

/* 25. Cyberpunk Button */
.btn-cyberpunk {
	background: #000;
	color: #0ff;
	border: 1px solid #0ff;
	box-shadow: 0 0 10px #0ff, 0 0 20px #0ff;
	text-shadow: 0 0 5px #0ff;
	position: relative;
}

.btn-cyberpunk::before {
	content: '';
	position: absolute;
	top: -2px;
	left: -2px;
	right: -2px;
	bottom: -2px;
	background: linear-gradient(45deg, #ff00cc, #3333ff, #00ccff, #ffcc00);
	background-size: 400% 400%;
	z-index: -1;
	animation: cyber-glow 3s ease infinite;
	border-radius: 10px;
}

@keyframes cyber-glow {
	0% {
		background-position: 0% 50%;
	}
	50% {
		background-position: 100% 50%;
	}
	100% {
		background-position: 0% 50%;
	}
}

/* 26. Jelly Button */
.btn-jelly {
	background: #ff6b6b;
	animation: jelly 1.5s infinite;
}

@keyframes jelly {
	0%, 100% {
		transform: scale(1, 1);
	}
	25% {
		transform: scale(0.9, 1.1);
	}
	50% {
		transform: scale(1.1, 0.9);
	}
	75% {
		transform: scale(0.95, 1.05);
	}
}

/* 27. Spotlight Button */
.btn-spotlight {
	background: #333;
	color: white;
	position: relative;
	overflow: hidden;
}

.btn-spotlight::before {
	content: '';
	position: absolute;
	top: 0;
	left: -100%;
	width: 100%;
	height: 100%;
	background: linear-gradient(90deg, transparent, rgba(255,255,255,0.4), transparent);
	transition: left 0.7s ease;
}

.btn-spotlight:hover::before {
	left: 100%;
}

/* 28. Split Text Button */
.btn-split {
	background: transparent;
	border: 2px solid #4ecdc4;
	color: #4ecdc4;
	position: relative;
	overflow: hidden;
}

.btn-split span {
	display: inline-block;
	transition: transform 0.5s ease;
}

.btn-split:hover span:nth-child(odd) {
	transform: translateY(-100%);
}

.btn-split:hover span:nth-child(even) {
	transform: translateY(100%);
}

.btn-split::before {
	content: attr(data-text);
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	display: flex;
	align-items: center;
	justify-content: center;
	transform: translateY(100%);
	transition: transform 0.5s ease;
	color: white;
}

.btn-split:hover::before {
	transform: translateY(0);
}

/* 29. Gradient Border Button */
.btn-gradient-border {
	background: transparent;
	color: white;
	position: relative;
	z-index: 1;
}

.btn-gradient-border::before {
	content: '';
	position: absolute;
	top: 0;
	left: 0;
	right: 0;
	bottom: 0;
	border-radius: 8px;
	padding: 2px;
	background: linear-gradient(45deg, #ff6b6b, #4ecdc4, #45b7d1, #96ceb4, #ffeaa7);
	background-size: 400% 400%;
	-webkit-mask: linear-gradient(#fff 0 0) content-box, linear-gradient(#fff 0 0);
	mask: linear-gradient(#fff 0 0) content-box, linear-gradient(#fff 0 0);
	-webkit-mask-composite: xor;
	mask-composite: exclude;
	animation: gradient-border 3s ease infinite;
	z-index: -1;
}

@keyframes gradient-border {
	0% {
		background-position: 0% 50%;
	}
	50% {
		background-position: 100% 50%;
	}
	100% {
		background-position: 0% 50%;
	}
}

/* 30. Infinite Loop Button */
.btn-infinite {
	background: #6a11cb;
	position: relative;
	overflow: hidden;
}

.btn-infinite::before {
	content: '';
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
	transform: translateX(-100%);
	animation: infinite-loop 1.5s infinite;
}

@keyframes infinite-loop {
	0% {
		transform: translateX(-100%);
	}
	100% {
		transform: translateX(100%);
	}
}

/* Micro Banner */
.micro-banner {
	background: rgba(0, 0, 0, 0.7);
	padding: 15px 30px;
	border-radius: 50px;
	margin-top: 40px;
	display: flex;
	align-items: center;
	justify-content: center;
	gap: 10px;
	font-size: 1.1rem;
	box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
}

.micro-banner .heart {
	color: #ff6b6b;
	animation: heartbeat 1.5s infinite;
}

@keyframes heartbeat {
	0%, 100% {
		transform: scale(1);
	}
	50% {
		transform: scale(1.2);
	}
}

@media (max-width: 768px) {
	.buttons-grid {
		grid-template-columns: 1fr;
	}
	
	h1 {
		font-size: 2.2rem;
	}
}
`)