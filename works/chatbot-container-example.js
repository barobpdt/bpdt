#{script[ready]
	fetch() [result=>changeState('chat-list', result.children)]
	fetch() [result=>changeState('emoji-list', result.chidren)]
}
#{innerHTML[chat-container]
	<div class="chat-header">echo[chat_header]</div>
	<div class="chat-messages">
		effect[chat-list] 
		while(item) item.messageType=='recv' ? echo[chat_recv] : echo[chat_send]	
	</div>
	<div class="chat-input-container">
		<div class="chat-toolbar">echo[chat-toolbar]</div>	
		<div class="emoji-picker">
			effect[emoji-list]
			while(item) echo[chat_emoji]
		</div>	
		<div class="chat-input">echo[chat_input]</div>	
		<div class="file-preview"></div>
	</div>
}


#{set[chat-header]
	<h1>채팅 메시지</h1>
	<div class="user-info">
		<span class="user-name">사용자</span>
		<span class="status online"></span>
	</div>
}

#{html[chat-recv]
	<div class="message received">
		<div class="message-content">
			<p>${item.msg}</p>
			<span class="message-time">${item.time}</span>
		</div>
	</div>
}

#{html[chat-send]
	<div class="message sent">
		<div class="message-content">
			<p>${item.msg}</p>
			<span class="message-time">${item.time}</span>
		</div>
	</div>
}

/* 이모지추가 아이콘 추가 */
#{set[chat-toolbar]
	<button class="toolbar-button">
		<i class="far fa-smile"></i>
	</button>
	<label class="toolbar-button upload-clip">
		<i class="fas fa-paperclip"></i>
	</label>
}

#{html[chat-emoji]
	<div class="emoji-category">
		<h4>${item.name}</h4>
		<div class="emoji-list">
			while(cur, item.chidren) 
			<span class="emoji" data-emoji="${cur.emoji}">${cur.emoji}</span>
		</div>
	</div>
}
 

#{set[chat-input]
	<textarea id="message-input" placeholder="메시지를 입력하세요..." rows="1"></textarea>
	<button id="send-button">
		<i class="fas fa-paper-plane"></i>
	</button>
}


#{css[chat]
	/* 기본 스타일 */
	* {
		margin: 0;
		padding: 0;
		box-sizing: border-box;
		font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
	}
	body {
		background-color: #f5f5f5;
		height: 100vh;
		display: flex;
		justify-content: center;
		align-items: center;
	}

}
 
 
