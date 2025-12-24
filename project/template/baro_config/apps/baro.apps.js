##> func { name=ApplicationUtils }
	@apps.loadPage(&src, base, pageId) {
		source =str( '<widgets base="$base">$src</widgets>')
		Cf.rootNode('@funcInfo').set('pageBase', base)
		Cf.sourceApply(source)
		print("load page source========$source")
		Cf.rootNode('@funcInfo').set('pageBase', '')
		return page(base, pageId, pageId)
	}
