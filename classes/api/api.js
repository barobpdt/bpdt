<api>
	 
	test(req, param, uri) {
		return "api test URI=> $uri";
	}
	sleep(req, param, &uri) {
		System.sleep(10000)
		param.message='sleep 10s test'
		return param
	}
</api>
