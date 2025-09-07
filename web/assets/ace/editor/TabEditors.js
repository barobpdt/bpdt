"use strict";

var config = require("ace/config");
config.setLoader(function(moduleName, cb) {
    require([moduleName], function(module) {
        cb(null, module);
    });
});
var oop = require("./lib/oop");
var EventEmitter = require("./lib/event_emitter").EventEmitter;
var Renderer = require("./virtual_renderer").VirtualRenderer;
var EditSession = require("./edit_session").EditSession;
var Editor = require("ace/editor").Editor;
var Range = require("ace/range").Range;
var whitespace = require("ace/ext/whitespace");
var MarkerGroup = require("ace/marker_group").MarkerGroup;
var theme = require("ace/theme/textmate");
var UndoManager = require("ace/undomanager").UndoManager;

['ace/autocomplete', 'ace/ext/language_tools', 'ace/ext/inline_autocomplete'].forEach(name=>require(name))

const TabEditors = function(el, fontSize, path) {
	this.container = $(getEl(el))
	this.$currentEditor = null
	this.$fontSize = fontSize || 14
	this.$theme = theme
	this.$docList = []
	this.setCurrentEditor('new','html')
} 
(function(){
    oop.implement(this, EventEmitter);
	this.$createEditor = function() {
		var el = document.createElement("div");
		var editor = new Editor(new Renderer(el, this.$theme));
		el.style.cssText = "position: absolute; top:0px; bottom:0px";
		editor.on("focus", function() { this._emit("focus", editor) }.bind(this));
		editor.setFontSize(this.$fontSize); 
		return editor
	}
	this.setCurrentEditor = function(name, mode) {
		let node = this.$docList.find(c=>c.name==name)
		let session = null
		let modeInfo = null
		if( node ) {
			session = node.session
			this.container.each((n,el)=>{
				if(el==node.editor) $(el).show() else $(el).hide()
			})
			this.$currentEditor = node.editor
		} else {
			const editor = this.$createEditor()
			session = new EditSession();
			session.setUndoManager(new UndoManager());
			editor.session = session
			session.name = name
			session.setUseWrapMode(true)
			session.setWrapLimitRange(80, 80)
			node = { name, session, editor}
			this.$docList.push(node)
			this.container.each((n,el)=>$(el).hide())
			this.container.append(editor)
			this.$currentEditor = editor
		}
		if( mode ) {
			const modeInfo = modelist.find(m => m.name==mode)
			if(!modeInfo) modeInfo = modelist.getModeForPath(mode)
			if( modeInfo) {
				session.modeName = modeInfo.name;
				session.setMode(modeInfo.mode);
				node.modeInfo = modeInfo
			}
		}
		return session;
	}
	this.setEditorEvent(name) {
		const node = this.$docList.find(c=>c.name==name)
		if(!node ) return console.log(`[TabEditors] 탭명 ${name}을 찾을수 없습니다`)
		const editor = node.editor
		editor.setOptions({
			enableBasicAutocompletion: true,
			enableLiveAutocompletion: true,
			enableSnippets: true,
			fontSize: "16px",
			showPrintMargin: false,
			showGutter: true,
			highlightActiveLine: true,
			wrap: true,
			useSoftTabs: true,
			tabSize: 4,
			enableAutoIndent: true
		});
		let pasteStartPosition = null;
		// 붙여넣기 시작 위치 저장
		editor.getSession().on('change', function() {
			console.log('change start', editor.getCursorPosition())
		});
		editor.on('paste', function(e) {
			// 붙여넣기 후 자동 들여쓰기 적용
			pasteStartPosition = editor.getCursorPosition();
			setTimeout(function() {
				console.log('paste end', editor.getCursorPosition())
				// 현재 커서 위치 (붙여넣기 후 위치)
				if(!pasteStartPosition) return;
				const currentPosition = editor.getCursorPosition();
				// 선택 범위 설정 (붙여넣기 시작 위치에서 현재 위치까지)
				editor.selection.setRange({
					start: pasteStartPosition,
					end: currentPosition
				});
				console.log('paste -> ',pasteStartPosition,currentPosition)
				// 위치 초기화
				pasteStartPosition = null;
			}, 10);
		});
	}
	
}).call(TabEditors.prototype)

exports.TabEditors = TabEditors;