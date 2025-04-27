"use strict";
/**
 * @typedef {import("../editor").Editor} Editor
 */

require("ace/ext/menu_tools/overlay_page");

const dom = require("ace/lib/dom");
const oop = require("ace/lib/oop");
const config = require("ace/config");
const EventEmitter = require("ace/lib/event_emitter").EventEmitter;
const buildDom = dom.buildDom;

const modelist = require("ace/ext/modelist");
const themelist = require("ace/ext/themelist");

const themes = { Bright: [], Dark: [] };
themelist.themes.forEach(function(x) {
    themes[x.isDark ? "Dark" : "Bright"].push({ caption: x.caption, value: x.theme });
});

const optionGroups = {
    Main: {
        Mode: {
            path: "mode",
            type: "select",
            items: modelist.modes.map(m=>({caption:m.caption,value:m.value}))
        },
        Theme: {
            path: "theme",
            type: "select",
            items: themes
        },
        "Keybinding": {
            type: "buttonBar",
            path: "keyboardHandler",
            items: [
                { caption : "Ace", value : null },
                { caption : "Vim", value : "ace/keyboard/vim" },
                { caption : "Emacs", value : "ace/keyboard/emacs" },
                { caption : "Sublime", value : "ace/keyboard/sublime" },
                { caption : "VSCode", value : "ace/keyboard/vscode" }
            ]
        },
        "Font Size": {
            path: "fontSize",
            type: "number",
            defaultValue: 12,
            defaults: [
                {caption: "12px", value: 12},
                {caption: "24px", value: 24}
            ]
        },
        "Soft Wrap": {
            type: "buttonBar",
            path: "wrap",
            items: [
               { caption : "Off",  value : "off" },
               { caption : "View", value : "free" },
               { caption : "margin", value : "printMargin" },
               { caption : "40",   value : "40" }
            ]
        },
        "Cursor Style": {
            path: "cursorStyle",
            items: [
               { caption : "Ace",    value : "ace" },
               { caption : "Slim",   value : "slim" },
               { caption : "Smooth", value : "smooth" },
               { caption : "Smooth And Slim", value : "smooth slim" },
               { caption : "Wide",   value : "wide" }
            ]
        },
        "Folding": {
            path: "foldStyle",
            items: [
                { caption : "Manual", value : "manual" },
                { caption : "Mark begin", value : "markbegin" },
                { caption : "Mark begin and end", value : "markbeginend" }
            ]
        },
        "Soft Tabs": [{
            path: "useSoftTabs"
        }, {
            ariaLabel: "Tab Size",
            path: "tabSize",
            type: "number",
            values: [2, 3, 4, 8, 16]
        }],
        "Overscroll": {
            type: "buttonBar",
            path: "scrollPastEnd",
            items: [
               { caption : "None",  value : 0 },
               { caption : "Half",   value : 0.5 },
               { caption : "Full",   value : 1 }
            ]
        }
    },
    More: {
        "Atomic soft tabs": {
            path: "navigateWithinSoftTabs"
        },
        "Enable Behaviours": {
            path: "behavioursEnabled"
        },
        "Wrap with quotes": {
            path: "wrapBehavioursEnabled"
        },
        "Enable Auto Indent": {
            path: "enableAutoIndent"
        },
        "Full Line Selection": {
            type: "checkbox",
            values: "text|line",
            path: "selectionStyle"
        },
        "Highlight Active Line": {
            path: "highlightActiveLine"
        },
        "Show Invisibles": {
            path: "showInvisibles"
        },
        "Show Indent Guides": {
            path: "displayIndentGuides"
        },
        "Highlight Indent Guides": {
            path: "highlightIndentGuides"
        },
        "Persistent HScrollbar": {
            path: "hScrollBarAlwaysVisible"
        },
        "Persistent VScrollbar": {
            path: "vScrollBarAlwaysVisible"
        },
        "Animate scrolling": {
            path: "animatedScroll"
        },
        "Show Gutter": {
            path: "showGutter"
        },
        "Show Line Numbers": {
            path: "showLineNumbers"
        },
        "Relative Line Numbers": {
            path: "relativeLineNumbers"
        },
        "Fixed Gutter Width": {
            path: "fixedWidthGutter"
        },
        "Show Print Margin": [{
            path: "showPrintMargin"
        }, {
            ariaLabel: "Print Margin",
            type: "number",
            path: "printMarginColumn"
        }],
        "Indented Soft Wrap": {
            path: "indentedSoftWrap"
        },
        "Highlight selected word": {
            path: "highlightSelectedWord"
        },
        "Fade Fold Widgets": {
            path: "fadeFoldWidgets"
        },
        "Use textarea for IME": {
            path: "useTextareaForIME"
        },
        "Merge Undo Deltas": {
            path: "mergeUndoDeltas",
            items: [
               { caption : "Always",  value : "always" },
               { caption : "Never",   value : "false" },
               { caption : "Timed",   value : "true" }
            ]
        },
        "Elastic Tabstops": {
            path: "useElasticTabstops"
        },
        "Incremental Search": {
            path: "useIncrementalSearch"
        },
        "Read-only": {
            path: "readOnly"
        },
        "Copy without selection": {
            path: "copyWithEmptySelection"
        },
        "Live Autocompletion": {
            path: "enableLiveAutocompletion"
        },
        "Custom scrollbar": {
            path: "customScrollbar"
        },
        "Use SVG gutter icons": {
            path: "useSvgGutterIcons"
        },
        "Annotations for folded lines": {
            path: "showFoldedAnnotations"
        },
        "Keyboard Accessibility Mode": {
            path: "enableKeyboardAccessibility"
        },
        "Gutter tooltip follows mouse": {
            path: "tooltipFollowsMouse",
            defaultValue: true
        }
    }
};

class myOptionPanel {
    constructor(editor, element) {
        this.editor = editor;
        this.container = element || document.createElement("div");
        this.groups = [];
        this.options = {};
    }
    add(config) {
        if (config.Main)
            oop.mixin(optionGroups.Main, config.Main);
        if (config.More)
            oop.mixin(optionGroups.More, config.More);
    }

  
    render() {
        this.container.innerHTML = "";
		const tables = ["table", {role: "presentation", id: "controls"}, 
            this.renderOptionGroup(optionGroups.Main),
            ["tr", null, ["td", {colspan: 2},
                ["table", {role: "presentation", id: "more-controls"}, 
                    this.renderOptionGroup(optionGroups.More)
                ]
            ]],
            ["tr", null, ["td", {colspan: 2}, "version " + config.version]]
        ]
        buildDom(tables, this.container, pageInfo?pageInfo.refs:null )
    }
    
    renderOptionGroup(group) {
		const self = this
        const result = Object.keys(group).map((key,idx) => {
            const item = group[key];
            if (!item.position)
                item.position = idx / 10000;
            if (!item.label)
                item.label = key;
            return item;
        })
		.sort((a,b) => a.position - b.position)
		.map(item => self.renderOption(item.label, item));
		console.log('@@ reandr group => ', result)
		return result
    }

    /**
     * @param {string} key
     * @param {Object} option
     */
    renderOptionControl(key, option) {
        const self = this;
        if (Array.isArray(option)) {
            return option.map(function(x) {
                return self.renderOptionControl(key, x);
            });
        }
        /**@type {any}*/
        let control = null;
        let value = self.getOption(option);
        
        if (option.values && option.type != "checkbox") {
            if (typeof option.values == "string")
                option.values = option.values.split("|");
            option.items = option.values.map(function(v) {
                return { value: v, name: v };
            });
        }
        
        if (option.type == "buttonBar") {
            control = ["div", {role: "group", "aria-labelledby": option.path + "-label"}, option.items.map(function(item) {
                return ["button", { 
                    value: item.value, 
                    ace_selected_button: value == item.value, 
                    'aria-pressed': value == item.value, 
                    onclick: function() {
                        self.setOption(option, item.value);
                        const nodes = this.parentNode.querySelectorAll("[ace_selected_button]");
                        for(let i = 0; i < nodes.length; i++) {
                            nodes[i].removeAttribute("ace_selected_button");
                            nodes[i].setAttribute("aria-pressed", false);
                        }
                        this.setAttribute("ace_selected_button", true);
                        this.setAttribute("aria-pressed", true);
                    } 
                }, item.desc || item.caption || item.name];
            })];
        } else if (option.type == "number") {
            control = ["input", {type: "number", value: value || option.defaultValue, style:"width:3em", oninput: function() {
                self.setOption(option, parseInt(this.value));
            }}];
            if (option.ariaLabel) {
                control[1]["aria-label"] = option.ariaLabel;
            } else {
                control[1].id = key;
            }
            if (option.defaults) {
                control = [control, option.defaults.map(function(item) {
                    return ["button", {onclick: function() {
                        const input = this.parentNode.firstChild;
                        input.value = item.value;
                        input.oninput();
                    }}, item.caption];
                })];
            }
        } else if (option.items) {
            const buildItems = function(items) {
                return items.map(function(item) {
                    return ["option", { value: item.value || item.name }, item.desc || item.caption || item.name];
                });
            };
            
            const items = Array.isArray(option.items) 
                ? buildItems(option.items)
                : Object.keys(option.items).map(function(key) {
                    return ["optgroup", {"label": key}, buildItems(option.items[key])];
                });
            control = ["select", { id: key, value: value, onchange: function() {
                self.setOption(option, this.value);
            } }, items];
        } else {
            if (typeof option.values == "string")
                option.values = option.values.split("|");
            if (option.values) value = value == option.values[1];
            control = ["input", { type: "checkbox", id: key, checked: value || null, onchange: function() {
                let value = this.checked;
                if (option.values) value = option.values[value ? 1 : 0];
                self.setOption(option, value);
            }}];
            if (option.type == "checkedNumber") {
                control = [control, []];
            }
        }
		console.log('@@ control =>', control)
        return control;
    }

    /**
     * 
     * @param key
     * @param option
     */
    renderOption(key, option) {
        if (option.path && !option.onchange && !this.editor.$options[option.path])
            return;
        const path = Array.isArray(option) ? option[0].path : option.path;
        this.options[path] = option;
        const safeKey = "-" + path;
        const safeId = path + "-label";
        const control = this.renderOptionControl(safeKey, option);
        return ["tr", {class: "ace_optionsMenuEntry"}, ["td",
            ["label", {for: safeKey, id: safeId}, key]
        ], ["td", control]];
    }

    /**
     * @param {string | number | Object} option
     * @param {string | number | boolean} value
     */
    setOption(option, value) {
        if (typeof option == "string")
            option = this.options[option];
        if (value == "false") value = false;
        if (value == "true") value = true;
        if (value == "null") value = null;
        if (value == "undefined") value = undefined;
        if (typeof value == "string" && parseFloat(value).toString() == value)
            value = parseFloat(value);
        if (option.onchange)
            option.onchange(value);
        else if (option.path)
            this.editor.setOption(option.path, value);
        this._signal("setOption", {name: option.path, value: value});
    }
    
    getOption(option) {
        if (option.getValue)
            return option.getValue();
        return this.editor.getOption(option.path);
    }
}
oop.implement(myOptionPanel.prototype, EventEmitter);

exports.myOptionPanel = myOptionPanel;
