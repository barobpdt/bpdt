## 노래방책 조회 
node=_node('sample')
node.url='https://kysing.kr/karaoke-book/'
node.name='kybook01'
not(node.parseResult) node[
	parseResult(&s) {
		node=this;
		print("@@ parse result start ",  s.size())
		s.findPos('<div class="index_daily_search_wrap',0,1)
		not(s.ch()) {
			print("목록찾기 오류")
			return;
		}
		parse(s.match('<div','</div>'))
		s.findPos('<div class="search_chart_page_nav">')
		s.findPos('<a') 
		s.findPos('<a href=')
		href = s.match().trim()
		if(href ) {
			not(node.baseUrl ) node.baseUrl = node.url
			node.url = _s('${node.baseUrl}${href}')
			print("@@ next xxxxx ${node.url} xxxx")
		}
		parse = func(&s) {
			while(s.valid(), n) {
				s.findPos('<ul',0,1)
				ss=s.match('<ul>','</ul>')	if(typeof(ss,'bool')) return;	
				ss.findPos('</li>')				
				num=tagVal(ss)
				title=tagVal(ss)
				singer=tagVal(ss)
				print("xx[$n]xx", num, title, singer)
				if(n>4) break;
			}
		};
		tagVal = func(tag) {
			c=ss.ch() not(c) return;
			ss.findPos('<li',0,1)
			line=ss.match("<li","</li>") if(typeof(line,'bool')) return;
			line.findPos('>')
			return line.trim();
		}
	}	
]
print("@@ [web job start]")
@job.addWebJob('openUrl', node.url, node) 

## 태진 top100 조회
while(y=0,9) {
	year="201$y"
	root = _node('top100').removeAll(true)
	while(n=0,12) {
		m=n+1
		month=when(m.lt(10), "0$m", "$m")
		days=System.date(System.localtime("$year-$month-01"),'daysInMonth')
		data = _s('chartType=TOP&searchStartDate=$year-$month-01&searchEndDate=$year-$month-$days&strType=')
		node = root.addNode()
		node.set('work_month', "$year-$month")
		node.set('@method','POST')
		node.set('@data', data)
		@job.addWebJob('apiCall_tjTop1000', 'https://www.tjmedia.com/legacy/api/topAndHot100', node)
	}
}

@job.apiCall_tjTop1000#web(&s, node) {
	node.inject(apiName, work_month)
	cur=node.addNode('@current').removeAll(true)
	cur.parseJson(s)
	items = cur.resultData.items
	row=items.get(0)
	keys=row.keys()
	tm=System.localtime()
	db=Baro.db('tj')
	db.open('tj_info.db')
	not(db.open()) db.exec('create table top100 (work_month, tm, indexTitle, indexSong, word, mv_yn, imgthumb_path, rank, pro, com, icongubun)')
	sql = #[
		insert into top100 (
			work_month, tm,
			indexTitle, indexSong, word, mv_yn, imgthumb_path, rank, pro, com, icongubun
		) values(
			${work_month}, ${tm},
			#{indexTitle}, #{indexSong}, #{word}, #{mv_yn}, #{imgthumb_path}, #{rank}, #{pro}, #{com}, #{icongubun}
		)
	]
	while(row, items) {
		db.exec(sql, row)
	}
}

## 유튜브 가요정보 조회 
node=_node('test')
db=Baro.db('media_info') not(db.open()) {
	dbFile = _s('${@classes.path}/data/tj_info.db')
	db.open(dbFile)
}
not(db.count("select count(1) from sqlite_master where name='music_info' ")) {
	sql = _s('create music_info (${node.fields}) values(${#node.fields})')
	db.exec(sql)
}
node.name = '185815'
node.url = 'https://www.lyrics.co.kr/main/youtube_list_do'
node.fields = 'description,thumbnails_default,thumbnails_high, thumbnails_medium, title, yid, tm'
node.data = 'search_word=(+) 이문세 - 그대와 영원히&p=185815'
node.set('@method','POST')
node.set('@data',  data)
not(node.parseResult) node[
	parseResult(&s, node) {
		db=sqlite('media_info')
		not(db.isTable(
		ss=s.decode('unicode')
		cur = node.addNode('musicInfo')
		cur.parseJson(ss)
		search = cur.data.search_list
		if(search) {
			tm = System.localtime()
			sql = _s('insert into music_info (${node.fields}) values (${#node.fields})')
			while(row, search ) {
				row.with(tm)
				db.exec(sql, row)
			}
		}
	}
]
@job.addWebJob('openUrl',node.url, node)	
