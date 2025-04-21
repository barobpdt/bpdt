<script module="@zip:loader">
    init() {
		root=Cf.rootNode()
		System.watcherClipboard(this.dummyProc)
		setEvent(root,'onChanageClipboard',this.zipfileProc)
    }
    dummyProc(type, data, str) {
		
	}
    zipfileProc(type, data, str) {
		not(type.eq('text')) return;
		not(data.start('file:///')) return;
		path = data.value(8)
		ext=right(path,'.').lower()
		print("copy file =>", path, ext)
		not(ext.eq('zip')) return;
		this.zipParse(path)
	}
	zipParse(path, prefix) {
		zip=Baro.zip()
		zip.open(path)
		not(zip.open()) return print("압출파일 오픈오류 $path 파일이 없습니다");
		arr = zip.unzipList()
		base = this.newNode('@base').removeAll()
		not(prefix) prefix='tree_'
		idx=1
		while(cur, arr) {
			c=addChild(cur.
		}
		
		findName=func(root, name) {
			while(cur, root) {
				not(cur.name) return print("in valid node name=$name")
				if(cur.cmp('name',name)) return cur;
			}
			return null
		}
		addChild=func(&k) {
			root = base
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
	}
	
</script>
