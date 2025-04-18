<example>
	node = _node('stock.sise_index_day')
	headers = {'User-agent': 'Mozilla/5.0'}
	node.page = 1
	node.code = 'KPI200'
	node.urlTemplate = 'https://finance.naver.com/sise/sise_index_day.naver?code=${code}&page=#{page}'
	job('sise_index_day', 'jobWeb', @stock.sise_index_day )
	
	[종목정보]
	https://finance.naver.com/item/main.naver?code=005930
		<div id="tab_con1", <div class="first", 
		<table @<tr 
			[시가총액, ia]
			[순위, ib]
			[액면가/단위, ic]		
		<table @<tr
			[외국인한도주식수, id]
			[외국인보유주식수, ie]
			[외국인소진율, if]
		<table @<tr
			[투자의견/목표주가, ig]
			[52주 최고/최저, ih]
			[투자의견/목표주가, ii ]
			[투자의견/목표주가, ij ]
		<table @<tr
			[PER/EPS, ik]
			[추정PER/EPS, il]
			[PBR/BPS (2024.12), im]
			[배당수익률/, in ]
			
	
		
	[실시간 정보]
	https://polling.finance.naver.com/api/realtime?query=SERVICE_ITEM:005930|SERVICE_RECENT_ITEM:005930,036460,047050,00125
	
	[시간별 통계]
	https://finance.naver.com/item/sise_time.naver?code=005930&thistime=20250410161124
	
	<table @<tr {not(colspan), add(header)} 
		[체결시각, ]
		[체결가, ]
		[전일비, ]
		[매도, ]
		[매수, ]
		[거래량, ]
		[변동량, ]
</example>

<script module="@stock">
	// 날짜 데이터 추출(https://finance.naver.com/item/sise_day.naver?code=종목코드번호)
	sise_index_day(s) {
		db=Baro.db('stock') not(db.open()) return print("jobWeb sise_index_day db('stock') 데이터베이스가 연결되지 않았습니다")
		job = null
		node = null
		worker = this.currentWorker
		if( worker.module ) {
			job = worker.module			
			node = job.currentJobNode
		}
		not(node) return print("jobWeb sise_index_day 현재 실행노드 미정의")
		s.findPos('<div class="box_type_m">')
		s.findPos('<table',0,1)
		ss=s.match('<table','</table>')
		ss.findPos('>')
		while(ss.valid(), n) {
			ss.findPos('<tr',0,1)
			not(ss.ch()) break;
			tr=ss.match('<tr','</tr>')
			if(n.eq(0)) {
				if( page.eq('1') ) {
					tr.findPos('>')
					val = tr.trim()
					node.header = val.utf8()
					db.exec("insert into header_info(type, header) values('sise_index_day',#{header})", node)
				}
				continue
			}
			if(typeof(tr,'bool')) continue;
			if(tr.find('colspan=')) continue;
			work_dt='', price='', rate='', a='', b='', c='', tm=System.localtime()
			while(tr.valid(), col) {
				tr.findPos('<td',0,1)
				not(tr.ch()) break;
				td=tr.match('<td','</td>')
				td.findPos('>')
				// if( n.eq(2)) print("td [$col] ===> $td ")
				if(td.ch('<')) {
					if(td.find('ico_up')) {
						td.findPos('>')
						v= stripTag(td.trim())
						vv="up $v"
					} else if(td.find('ico_down')) {
						td.findPos('>')
						v= stripTag(td.trim())
						vv="down $v"
					} else if(td.find('<span')) {
						td.findPos('>')
						vv=td.findPos('</span>').trim()
					}	else {
						vv=td.trim()
					}	
				} else {
					vv=td.trim()
				}
				switch(col) {
				case 0: work_dt=vv;
				case 1: price=vv
				case 2: rate=vv
				case 3: a=vv
				case 4: b=vv
				case 5: c=vv
				default:
				}
			}
			not(work_dt) continue;
			node.with(work_dt, price, rate, a, b, c, tm)
			db.exec("insert into sise_index_day(work_dt, price, rate, a, b, c, tm) values (#{work_dt}, #{price}, #{rate}, #{a}, #{b}, #{c}, #{tm})", node)
		}
		if( job ) {
			not(node.page) node.page=1
			not(node.lastPage) node.lastPage = 360 
			if( node.urlTemplate && node[page<lastPage] ) {
				node.incrNum('page')
				node.url = format(node.urlTemplate, node)
				job.jobAdd(node)
			}
		}
	}
	
</script>	