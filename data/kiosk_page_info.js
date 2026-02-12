##> 키오스크 메인 [vicon.award_star_bronze_1]
KioskHiTec:main{
<Page Width="1080" Height="1920" bg="${imagePath}/type/main_bg.png">
	 <MainTitle Height="320">  
		<Logo src="${imagePath}/type/main_logo.png" Height="210"/>
		<HomeButton class="layer" src="${imagePath}/type/main_home.png" Width="100" Height="80" Margin=[980,130,20,0]/>
	</MainTitle>

	<!-- 코너 탭 -->
	<CornerTab  class="layer" Width="1080" Height="110" Width="1080" Height="110" Margin=[0,210,0,0]
		BackgroundImage="${imagePath}/type/tab_bg.png"
		TabImage="${imagePath}/type/tab_4_[#].png" 
		LeftButton="${imagePath}/type/tab_left_[#].png"
		RightButton="${imagePath}/type/tab_right_[#].png"
	/>
	
	<MenuList Width="1080" Height="1026" Background="#FFFFFF">
		<ListBox id="menuView"  Margin=[24,24,48,34] Height="912" space="12"
			Icon_DispType01="${imagePath}/type/icon_new.png"
			Icon_DispType02="${imagePath}/type/icon_best.png"
			MenuBlankImage="${imagePath}/type/bg_menu_blank.png"/>
		<PagePanel id="pageView" Margin=[24]
			PrevImage="${imagePath}/type/btn/btn_prev_[#].png"
			NextImage="${imagePath}/type/btn/btn_next_[#].png"
			PageImage="${imagePath}/type/btn/navi_[#].png" 
		/>
	</MenuList>
	
	<MenuCart Height="424" CurrentPage="AdPanel">
		<!-- 장바구니 -->
		<ShoppingCart id="orderView"
			OrderMinusImage="${imagePath}/type/list_minus_[#].png"
			OrderPlusImage="${imagePath}/type/list_plus_[#].png"
			OrderDeleteImage="${imagePath}/type/list_del_[#].png"
			ScrollUp="${imagePath}/type/scroll_up_[#].png"
			ScrollDown="${imagePath}/type/scroll_down_[#].png"
		/>
		<!-- 광고 -->
		<AdPanel id="adView" HeightArray=[72,352] TitleImage="${imagePath}/type/list_title_bg.png" TitleText="EVENT" />
	</MenuCart>        

	<!-- 하단 -->
	<MainStatus type="hbox" Height="150" BackgroundImage="${imagePath}/type/background_bottom.png">
		<!-- 주문수량금액 -->
		<OrderInfo id="orderInfoView" Margin=[20,14,40,26] Width="525"/>
		<MainButtons id="buttonsView" space="10">
			<!-- 전체취소 버튼 -->
			<ClearAll  Width="170" id="main_cancel"
				src="${imagePath}/type/main_cancel_[#].png" text="취소" 
			/> 
			<!-- 카드결제 버튼 -->
			<SelectCard Width="350" id="main_order"
				src="${imagePath}/type/main_order_[#].png" text="카드결제" 
			/>
		</MainButtons>
	</MainStatus>
	<Popup id="dialog" ClassPath="common"/>
	<Popup id="stack" ClassPath="common"/>
</Page>
}

##> 공통팝업 테스트 [vicon.bricks_defalut]
KioskHiTec:PopupTest{
<Page bg="${imagePath}/bg_pattern.jpg">
	<MyControl/>
	<Popup id="dialog" ClassPath="common"/>
	<Popup id="stack" ClassPath="common"/>
</Page>


}
 
##> 관리자 기능 구현 [vicon.computer_defalut]
KioskHiTec:AdminMenu{
<Page>
	<MyMenu Height=98/>
	<Content/>
	<Popup id="dialog" ClassPath="common"/>	
</Page>


}
 