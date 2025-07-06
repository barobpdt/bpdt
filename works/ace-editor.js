https://github.com/manubb/react-ace-builds/blob/local/docs/Ace.md
import React from "react";
import AceEditor from "react-ace";
import { IntegrationTests } from "./example-input";
// import brace from "brace";

import "brace/mode/typescript";
import "brace/theme/tomorrow_night";
/*
function onChange(newValue) {
  console.log("change", newValue);
}
*/

const markers = [
  {
    startRow: 3,
    endRow: 4,
    startCol: 2,
    type: "text",
    className: "test-marker"
  }
];

const annotations = [
  {
    row: 3, // must be 0 based
    column: 4, // must be 0 based
    text: "error.message", // text to show in tooltip
    type: "error"
  }
];

function getDelay() {
  return Math.round(Math.random() * 0) + 1;
}

const Editor = ({ useDelay } = { useDelay: false }) => {
  const [textValue, setTextValue] = React.useState("");

  React.useEffect(() => {
    if (useDelay) {
      let counter = 0;
      let timeout;

      function updateTextAndCounter() {
        setTextValue(IntegrationTests.slice(0, counter));
        counter++;
        if (counter < IntegrationTests.length) {
          timeout = setTimeout(updateTextAndCounter, getDelay());
        }
      }

      timeout = setTimeout(updateTextAndCounter, getDelay());
      return () => {
        if (typeof timeout !== "undefined") {
          clearTimeout(timeout);
        }
      };
    } else {
      setTextValue(IntegrationTests.slice(0, IntegrationTests.length));
    }
  }, [useDelay]);

  return (
    <div>
      <h3>Integration testing done right</h3>
      <AceEditor
        mode="typescript"
        theme="tomorrow_night"
        width="100%"
        name="ace-editor"
        editorProps={{ $blockScrolling: false }}
        value={textValue}
        showGutter={true}
        setOptions={{
          enableBasicAutocompletion: false,
          enableLiveAutocompletion: false,
          enableSnippets: false,
          showLineNumbers: false,
          tabSize: 2
        }}
        markers={markers}
        annotations={annotations}
      />
    </div>
  );
};

export default Editor;
## custon syntax
export default class CustomSqlMode extends ace.acequire('ace/mode/text').Mode {

	constructor(){
		super();
		// Your code goes here
	}
}
And my react-ace code looks like:

render() {
		return <div>
			<AceEditor
				ref="aceEditor"
				mode="sql"     // Default value since this props must be set.
				theme="chrome" // Default value since this props must be set.
			/>
		</div>;
	}

	componentDidMount() {
		const customMode = new CustomSqlMode();
		this.refs.aceEditor.editor.getSession().setMode(customMode);
	}



## command 
const commands = [{
  name: 'saveFile',
  bindKey: {win: 'Ctrl-S', mac: 'Command-S'},
  exec: function(editor) {
    console.log('File saved');
    // Implement save logic here
  }
}];

<AceEditor
  mode="javascript"
  theme="github"
  commands={commands}
  // ... other props
/>



##
EuiCodeEditor
	mode="yaml"
	theme="github"
	width="100%"
	height="100%"
	className="ace-tm"
	value={configurationText}
	onChange={(e) => onChangeEventText(e)}
		setOptions={{
			fontSize: '14px',
			enableBasicAutocompletion: true,
			enableSnippets: true,
			enableLiveAutocompletion: true
		}}
	onBlur={() => {
	console.log('blur');
	}} // eslint-disable-line no-console
	aria-label="Code Editor"
	onLoad = {editor => {   
	console.log("EDITOR");
	console.log(editor);
	console.log(editor.completers);
	//  editor.completers = [staticWordCompleter];
	/*  var mode = editor.getMode();   
	mode.getCompletions =  (state, session, pos, prefix) => {
	    return [];
	} ; 
	editor.setMode(mode);
	editor.completers = [staticWordCompleter];*/
	}}
	/>

mport { render } from "react-dom";
import AceEditor from "../src/ace";
import "brace/mode/jsx";
import 'brace/mode/HCPCustomCalcs'
import 'brace/theme/monokai'
import "brace/snippets/HCPCustomCalcs";
import "brace/ext/language_tools";
const defaultValue = `function onLoad(editor) {
  console.log("i've loaded");
}`;
class App extends Component {

  constructor(props, context) {
    super(props, context);
    this.onChange = this.onChange.bind(this);
}
onChange(newValue) {
    console.log('changes:', newValue);
}
  render() {

      return (
          <div>
              <AceEditor
                  mode="HCPCustomCalcs"
                  theme="monokai"
                  width={ '100%' }
                  height={ '100vh' }
                  onChange={this.onChange}
                  name="UNIQUE_ID_OF_DIV"
                  editorProps={{
                      $blockScrolling: true
                  }}
                  enableBasicAutocompletion={true}
                  enableLiveAutocompletion={true}
                  enableSnippets={true}
              />
          </div>
      );
  }
}

import 'brace/ext/language_tools';
And wrote my onLoad as follow:

onLoad={editor => {
	var mode = editor.getSession().getMode();
	mode.getCompletions = (state, session, pos, prefix, callback) => {
	var completions = [];
	["example1", "example2"].forEach(function (w) {
		completions.push({
			value: w,
			meta: "my completion",
			snippet: `@{${w || ""}}`,
			caption: w || ""
		});
	});

		return completions;
	  
	}

	editor.getSession().setMode(mode);

	console.log("EDITOR");
	console.log(editor.getSession().getMode().getCompletions());

}}

##
var staticWordCompleter = {
    getCompletions: function(editor, session, pos, prefix, callback) {
        var wordList = ["foo", "bar", "baz"];
        callback(null, [...wordList.map(function(word) {
            return {
                caption: word,
                value: word,
                meta: "static"
            };
        }), ...session.$mode.$highlightRules.$keywordList.map(function(word) {
        return {
          caption: word,
          value: word,
          meta: 'keyword',
        };
      })]);

    }
}

langTools.setCompleters([staticWordCompleter])
// or 
editor.completers = [staticWordCompleter]  

##
onst customCompleter = {
	identifierRegexps: [/[a-zA-Z_0-9\.\$\-\u00A2-\uFFFF]/],
	getCompletions: (
		editor: Ace.Editor,
		session: Ace.EditSession,
		pos: Ace.Point,
		prefix: string,
		callback: Ace.CompleterCallback
	): void => {
		var completions: any[] = [];
		completions.push({
			value: "custom",
			className: "iconable"
		});
		if (prefix == "custom.") {
				RList = ["custom.Base64Decode",
					"custom.AnotherMethod",
					"custom.Method3",
					"custom.TestingFunction"
				];
				RList.forEach(function (w) {
					completions.push({
						value: w,
						className: "iconable"
					});
				});
		}
		callback(null, completions);
	}
}
langTools.addCompleter(customCompleter);
So when I'm pushing to completions i add a className of "iconable". The CSS file then looks like this:

.ace_iconable:after {
    font-family: "Font Awesome 5 Free";
    content: "\f1b2";
    display: inline-block;
    padding-right: 10px;
    padding-left: 10px;
    vertical-align: middle;
    font-weight: 900;
}

##
var editor = ace.edit('myeditor');
// Default value is the first one in comments
// All options are set to default value
editor.setOptions({
  // editor options
  selectionStyle: 'line',// "line"|"text"
  highlightActiveLine: true, // boolean
  highlightSelectedWord: true, // boolean
  readOnly: false, // boolean: true if read only
  cursorStyle: 'ace', // "ace"|"slim"|"smooth"|"wide"
  mergeUndoDeltas: true, // false|true|"always"
  behavioursEnabled: true, // boolean: true if enable custom behaviours
  wrapBehavioursEnabled: true, // boolean
  autoScrollEditorIntoView: undefined, // boolean: this is needed if editor is inside scrollable page
  keyboardHandler: null, // function: handle custom keyboard events
  
  // renderer options
  animatedScroll: false, // boolean: true if scroll should be animated
  displayIndentGuides: false, // boolean: true if the indent should be shown. See 'showInvisibles'
  showInvisibles: false, // boolean -> displayIndentGuides: true if show the invisible tabs/spaces in indents
  showPrintMargin: true, // boolean: true if show the vertical print margin
  printMarginColumn: 80, // number: number of columns for vertical print margin
  printMargin: undefined, // boolean | number: showPrintMargin | printMarginColumn
  showGutter: true, // boolean: true if show line gutter
  fadeFoldWidgets: false, // boolean: true if the fold lines should be faded
  showFoldWidgets: true, // boolean: true if the fold lines should be shown ?
  showLineNumbers: true,
  highlightGutterLine: false, // boolean: true if the gutter line should be highlighted
  hScrollBarAlwaysVisible: false, // boolean: true if the horizontal scroll bar should be shown regardless
  vScrollBarAlwaysVisible: false, // boolean: true if the vertical scroll bar should be shown regardless
  fontSize: 12, // number | string: set the font size to this many pixels
  fontFamily: undefined, // string: set the font-family css value
  maxLines: undefined, // number: set the maximum lines possible. This will make the editor height changes
  minLines: undefined, // number: set the minimum lines possible. This will make the editor height changes
  maxPixelHeight: 0, // number -> maxLines: set the maximum height in pixel, when 'maxLines' is defined. 
  scrollPastEnd: 0, // number -> !maxLines: if positive, user can scroll pass the last line and go n * editorHeight more distance 
  fixedWidthGutter: false, // boolean: true if the gutter should be fixed width
  theme: 'ace/theme/textmate', // theme string from ace/theme or custom?
 
  // mouseHandler options
  scrollSpeed: 2, // number: the scroll speed index
  dragDelay: 0, // number: the drag delay before drag starts. it's 150ms for mac by default 
  dragEnabled: true, // boolean: enable dragging
  focusTimout: 0, // number: the focus delay before focus starts.
  tooltipFollowsMouse: true, // boolean: true if the gutter tooltip should follow mouse

  // session options
  firstLineNumber: 1, // number: the line number in first line
  overwrite: false, // boolean
  newLineMode: 'auto', // "auto" | "unix" | "windows"
  useWorker: true, // boolean: true if use web worker for loading scripts
  useSoftTabs: true, // boolean: true if we want to use spaces than tabs
  tabSize: 4, // number
  wrap: false, // boolean | string | number: true/'free' means wrap instead of horizontal scroll, false/'off' means horizontal scroll instead of wrap, and number means number of column before wrap. -1 means wrap at print margin
  indentedSoftWrap: true, // boolean
  foldStyle: 'markbegin', // enum: 'manual'/'markbegin'/'markbeginend'.
  mode: 'ace/mode/html' // string: path to language mode 
});