<api>
emoji(req, param, &uri) { 
	conf('webdata.emoji',#[
표정
	😀,😃,😄,😁,😅,😂,🤣,😊,😇,🙂,🙃,😉,😌,😍,🥰
동물
	🐶,🐱,🐭,🐹,🐰,🦊,🐻,🐼,🐨,🐯,🦁,🐮,🐷,🐸,🐵
음식
	🍎,🍐,🍊,🍋,🍌,🍉,🍇,🍓,🍈,🍒,🍑,🥭,🍍,🥥,🥝	
])

	s = conf('webdata.emoji')
	s.ref()
	sp=-1;
	cur=null;
	while(s.valid()) {
		line = s.findPos("\n")
		indent = indentCount(line)
		not(line.ch()) continue;
		if(sp.eq(-1)) sp = indent
		dist = indent - sp;
		if(dist==0 ) {
			name = line.trim()
			cur = param.addNode().with(name)
		} else if(dist==1) {
			while(line.valid()) {
				not(line.ch()) break;
				emoji = line.findPos(',').trim()
				cur.addNode().with(emoji)
			}
		}
	}
	param.val('@treeMode', true)
	return param;
}
</api>