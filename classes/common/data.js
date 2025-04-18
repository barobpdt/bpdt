<script>
	makePageTree() {
		o=Cf.getObject()
		root = object('data.pageTree')
		while(k, o.keys()) {
			find(k)
		}
		return root;
		
		find = func(&s) {
			key=s.findPos('.').trim()
			not( key.eq('page','dialog') ) continue;
			a=findKey(root, key)
			not(a)  a=root.addNode().with(key)
			key=s.findPos(':').trim() // project
			b=findKey(a,key)
			not(b) b=a.addNode().with(key)
			key=s.trim()
			b.addNode().with(key)
		};
		findKey = func(node, key) {
			while(cur, node) {
				if( cur.cmp('key',key) ) return cur;
			}
			return null
		};
	}
    getFolderInfo(path) {
		ln="\r\n"
		return parse(path)

		parse = func(path,depth) {
			ss=''
			not(depth) depth=0
			tab=''
			while(n=0, depth) tab.add("\t")
			fo=Baro.file('test')
			fo.var(sort,'type,name')
			fo.list(path, func(info) {
				nextDepth = depth+1
				while(info.next()) {
					info.inject(type, name, fullPath, size, modifyDt, createDt)
					ss.add(tab, "$name, $size, $modifyDt",ln)
					if( type.eq('folder')) {
						ss.add(parse(fullPath, nextDepth))
					}
				}
				return ss;
			});
		};
	}
</script>