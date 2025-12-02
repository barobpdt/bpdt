<api>
	test(req, param, &uri) {
		type=uri.findPos('/').trim()
		param=uri.findPos('/').trim()
		if(type.eq('navIcons')) {
			return '["application_add","application_cascade","application_delete","application_double"]'
		}
		return "api test URI=> $uri type:$type ($param)";
	}
</api>
