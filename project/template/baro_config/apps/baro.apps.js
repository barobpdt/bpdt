##> func { name=ApplicationUtils }
	@apps.loadPage(&src, base, pageId) {
		source =str( '<widgets base="$base">$src</widgets>')
		Cf.rootNode('@funcInfo').set('pageBase', base)
		print("load page source========$source")
		Cf.sourceApply(source)
		Cf.rootNode('@funcInfo').set('pageBase', '')
		return page(base, pageId, pageId)
	}
