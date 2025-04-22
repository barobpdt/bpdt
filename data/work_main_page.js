<widgets>
	<page id="main">
		<splitter>
	</page>
	<page margin="0" id="leftTab">
		<tab>
	</page>
	<page margin="0"  id="contents">
		<div>
	</page>
	<page id="tab1">
		<editor id="e">
		<hbox>
			<button id="ok" text="tab1">
			<space>
		</hbox>
	</page>
	
	<page id="tab2">
		<editor id="e">
		<hbox>
			<button id="ok" text="tab2">
			<space>
		</hbox>
	</page>
	
	<page id="c1">
		<canvas id="c">
	</page>
	
</widgets>

~~
main = page('test:main')
main[
	init() {
		splitter=widget('splitter')
		@leftTab = splitter.addPage(page('leftTab'))
		@contents = splitter.addPage(page('contents'))		
	}
]
main.init()
~~
leftTab = main.leftTab
leftTab[
	init() {
		tab=widget('tab')
		tab.addPage(page('tab1'))
		tab.addPage(page('tab2'))		
	}
]
leftTab.init()

~~
contents=main.contents
contents[
	init() {
		widget('div').addPage(page('c1'))
	}
]
contents.init()

main.open()


tab=widget(leftTab,'tab')

tab.addPage( page('test:tab2'), 'bbb')

x=page('test:tab1')
x.child(0).spacing(4)
x.child(0).margin(4)

div=widget(contents,'div')
c=div.child(0)
~~
c[
	onDraw(dc, rc) {
		dc.fill('#aaa')
		dc.text('hello world ','center')
	}
]


main.child(0).margin(6,0,6,4)