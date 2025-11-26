##> loader {title:페이지 로딩, init() {
	clog('page loading ')
}}
container
	loader {onclick(e) {
		clog('loader click', e)
	}}
		counter
			number <>11<span>%</span><>
			progress
				percentBar
	end <css>
		@[this] {
			width:100%;
			height:100%;
		}
	</css>
end

<style>
	.main {
		width:100%;
		height:100%;
	}
</style>

