<api>
	funcSrc(req, param, &uri) {
		
	}
	funcCheck(req, param, &uri) {
		fnm=uri.findPos('/').trim()
		path=object('@inc.userFunc').get(fnm)
		if(path) {
			param.path=path
		} else {
			param.error="$fnm 함수찾기오류"
		}
		return param;
	}
	confVal(req, param, &uri) {
		name=uri.findPos('/').trim()
		db=Baro.db('config')
		db.fetchAll("select grp, cd from conf_info where grp='$name'", param, true)
		return param;
	}
</api>