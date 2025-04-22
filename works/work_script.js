x=conf('#confMap')
x=System.driveList()

~~
x=System.tick()
root=listFolder(path)
d=System.tick() - x;
print("xxx", d,x,root)
~~
path='C:/work'
root = _node('listFolder').removeAll();

~~
<func>
	listFolder(path, root, depth) {
		not(root) root = _node('listFolder').removeAll();
		not(depth) depth = 0;
		fo=Baro.file()
		fo.var(sort,'name, case')
		depth++;
		fo.list(path, func(info) {
			while(info.next()) {
				info.inject(type,name, fullPath)
				if(type.eq('file')) continue;
				if(name.eq('windows')) continue;
				cur=root.addNode().with(type,name,fullPath)
				if(depth<3) {
					listFolder(fullPath, cur, depth)
				}
			}
		})
		return root;
	}
</func>