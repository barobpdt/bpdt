## zip 파일 읽어서 트리만들기
<widgets>
	<page id="p1">
		<tree id="tree">
		<hbox>
			<button id="ok" text="ok">
			<space>
		</hbox>
	</page>
</widgets>

p=page('test:p1')
tree= widget(p,'tree')
p.open()
tree.model('name')
tree.rootNode(node)

tree.sort()
tree[
	onFilter(a) {
		return true;
	}
	onSort(c,a,b) {
		if(a.cmp('type', b.type)) {
			aa=a.name.lower(), bb=b.name.lower()
			return bb.lessThan(aa,true)
		}
		if(b.type.eq('folder')) return true
	}
]
tree.update()

~~
<func>
	findName(root, name) {
		while(cur, root) {
			not(cur.name) return print("in valid node name=$name")
			if(cur.cmp('name',name)) return cur;
		}
		return null
	}
	findParentNode(root, &k) {
		while(k.valid()) {
			name=k.findPos('/')
			a=findName(root,name)
			not(a) {
				a=root.addNode().with(name)
			}
			root = a
			not(k.ch()) break
		}
		return root;
	}
</func>

##
e=env.editor
s=e.session
d=s.getDocument()
end=d.$lines.length

Range = require('ace/range')

s.addFold("name", new Range(1,1,8,Infinity));
s.removeFold("name")

removedFolds = s.getFoldsInRange(new Range(0,0,end,Number.MAX_VALUE) );
s.removeFolds(removedFolds)

	// 스니펫 등록
const pythonSnippets = [
	{
		name: "def",
		content: "def ${1:function_name}(${2:parameters}):\n\t${3:pass}",
		tabTrigger: "def"
	}
];
pythonSnippets.forEach(snippet => {
	langTools.addCompleter({
		getCompletions: function(editor, session, pos, prefix, callback) {
			callback(null, [snippet]);
		}
	});
});