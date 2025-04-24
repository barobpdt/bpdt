<func>
	gitUse(req, param, &uri) {
		Cf.include('web/app.js')
		owner='kwangho-na'
		repo='baro'
		path=f(uri)
		return @git.call("https://api.github.com/$path") 
	}
	gitSha(req, param, &uri) {
		sha=uri.findPos('/').trim()
		owner='kwangho-na'
		repo='baro'
		path=f(uri)
		return @git.call("https://api.github.com/$path") 
	}
	gitPost(req, param, &uri) {
		Cf.include('web/app.js')
		type=uri.findPos('/').trim()
		owner='kwangho-na'
		repo='baro'
		path=f(uri)
		print("xxx git post xxx", path, type)
		switch(type) {
		case createCommit:
			return @git.callPost("https://api.github.com/$path", '{"state":"success"}') 
		case createPull:
			data='{"title":"pull title","body":"pull bodys","head":"kwangho-na:main","base":"main"}'
			return @git.callPost("https://api.github.com/$path", data)
		case createRepo:
			name=uri.findPos('/').trim()
			path="user/repos"
			data=fmt('{"name":"${name}"}')
			print("====== create repo ========", name, data);
			return @git.callPost("https://api.github.com/$path", data);
		case addBlobs:
			path='repos/kwangho-na/baro/git/blobs';
			data=#[{"content":"print(\"Hello, World! aaa test 123\")","encoding":"utf-8"}]
			return @git.callPost("https://api.github.com/$path", data);
		case addTree:
			name=uri.trim();
			base='44e9056f1af8e3568e8dc184f2bd9b4d17bd61ff'
			sha='5b755ad51997bfa286819d2b3b559b4b14b96f08';
			path='repos/kwangho-na/baro/git/trees';
			/*
			100644 for file (blob), 
			100755 for executable (blob), 
			040000 for subdirectory (tree), 
			160000 for submodule (commit), or 
			120000 for a blob that specifies the path of a symlink.
			*/
			data=
#[{
	"tree": [
		{
			"path": "${name}",
			"mode": "100644",   
			"type": "blob",
			"sha": "${sha}"
		}
	],
	"base_tree": "${base}"
}]
			return @git.callPost("https://api.github.com/$path", data);
		case addCommit:
			base='44e9056f1af8e3568e8dc184f2bd9b4d17bd61ff'
			sha='ae4fab3b663521fbc9e136bf6a97a97f7a047743'
			path='repos/kwangho-na/baro/git/commits'
			data=
#[{
	"message": "새로운 커밋실행",
	"tree": "${sha}",
	"parents": ["${base}"]
}]
			return @git.callPost("https://api.github.com/$path", data);
		case addIssue:
			path='repos/kwangho-na/test001/issues'
			val='버그가 있어요! 🐛'
			data=fmt('{"title":"${val}"}')
			return @git.callPost("https://api.github.com/$path", data);
		case addComment:
			num=uri.findPos('/').trim()
			body=uri.trim() not(body) body="감사합니다"
			path=f('repos/kwangho-na/test001/issues/{num}/comments')
			data=fmt('{"body":"${body}"}')
			return @git.callPost("https://api.github.com/$path", data);
		case addPatch:
			path='repos/kwangho-na/baro/git/refs/heads/main'
			sha='d5a54d486c1bba5d544a58bb1fc5517ee05be4df'
			data=#[{"sha":"${sha}"}]
			return @git.callPatch("https://api.github.com/$path", data);
		case fullReq:
			path='repos/kwangho-na/baro/pulls'
			sha='d5a54d486c1bba5d544a58bb1fc5517ee05be4df'
			data=#[{"sha":"${sha}"}]
			return @git.callPatch("https://api.github.com/$path", data);
			
		case addFile:
			path='repos/kwangho-na/baro/contents/hello.txt'
			data=#[{
	"message":"hello text commit",
	"content":"bXkgbmV3IGZpbGUgY29udGVudHM=",
	"branch":"main"
}]
			print("== git add file ==", data, path);
			return @git.callPut("https://api.github.com/$path", data);
		case createRef:
			path='repos/kwangho-na/baro/git/refs';
			name=uri.findPos('/').trim();
			not(name) name="na";
			sha='44e9056f1af8e3568e8dc184f2bd9b4d17bd61ff'
			data=#[{"ref":"refs/heads/${name}", "sha":"${sha}"}]
			return @git.callPost("https://api.github.com/$path", data);
		default:
		}
		return @git.call("https://api.github.com/$path") 
	}
</func>
