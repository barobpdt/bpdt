## 정규식처리
a='create table aaaa bbb;;'
a.exp('(\w+)\s*(IF\s+NOT\s+EXISTS\s+)\s*?(\w+)(.*?);',func() {
	exp.caseSensitive()
	a=exp.match()
	len=exp.length()
	print("len=>$len", a)	
})

parseDdl(req,param,&uri,data) {
	sql=stripSqlComment(fileRead('c:/temp/jkj.sql'))
	req.send(sql)	
	stripSqlComment = func(&s) {
		ss=''
		while(s.valid()) {
			left=s.findPos('--',1)
			ss.add(left)
			not(s.ch()) break;
			s.findPos("\n")
		}
		return ss;
	};
	parse = func(&s) {
		while(s.valid()) {			
			c=s.ch()
			not(c) break;
			s.findPos('create',2)
			not(s.ch()) break;
			a=s.move().lower()
			if(a.eq('table')) {
				
			}
		}
	}
}
## sql
-- jikyeojo.sg_act_info definition

CREATE TABLE `sg_act_info` (
  `MENU_NO` int(11) NOT NULL COMMENT '메뉴번호',
  `ACT_ID` varchar(30) NOT NULL COMMENT '액션순번',
  `ACT_TP_CD` varchar(10) DEFAULT NULL COMMENT '액션타입',
  `ACT_NM` varchar(128) DEFAULT NULL COMMENT '액션명',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `CONN_URL` varchar(128) DEFAULT NULL COMMENT '호출URL',
  `WR_YN` char(1) DEFAULT NULL COMMENT 'WR_YN',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MENU_NO`,`ACT_ID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='액션정보';


-- jikyeojo.sg_announce definition

CREATE TABLE `sg_announce` (
  `ANN_NO` varchar(20) NOT NULL COMMENT '공지번호',
  `ANN_NM_KO` varchar(40) DEFAULT NULL COMMENT '공지명 한국어',
  `ANN_NM_EN` varchar(40) DEFAULT NULL COMMENT '공지명 영어',
  `ANN_NM_CH` varchar(40) DEFAULT NULL COMMENT '공지명 중국어',
  `ANN_NM_JP` varchar(40) DEFAULT NULL COMMENT '공지명 일본어',
  `ANN_DESC_KO` varchar(2000) DEFAULT NULL COMMENT '공지내용 한국어',
  `ANN_DESC_EN` varchar(2000) DEFAULT NULL COMMENT '공지내용 영어',
  `ANN_DESC_CH` varchar(2000) DEFAULT NULL COMMENT '공지내용 중국어',
  `ANN_DESC_JP` varchar(2000) DEFAULT NULL COMMENT '공지내용 일본어',
  `ANN_START_DT` varchar(8) DEFAULT NULL COMMENT '공지시작일',
  `ANN_END_DT` varchar(8) DEFAULT NULL COMMENT '공지종료일',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ANN_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='공지정보';


-- jikyeojo.sg_app_menu definition

CREATE TABLE `sg_app_menu` (
  `APP_MENU_NO` varchar(20) NOT NULL COMMENT '메뉴번호',
  `APP_MENU_TYPE_CD` varchar(20) NOT NULL COMMENT '앱타입',
  `APP_MENU_NM_KR` varchar(40) DEFAULT NULL COMMENT '메뉴명KO',
  `APP_MENU_NM_EN` varchar(40) DEFAULT NULL COMMENT '메뉴명EN',
  `APP_MENU_NM_CH` varchar(40) DEFAULT NULL COMMENT '메뉴명ZH',
  `APP_MENU_NM_JP` varchar(40) DEFAULT NULL COMMENT '메뉴명JA',
  `USE_YN` varchar(2) DEFAULT 'N' COMMENT '사용여부',
  `MENU_FILE_NO` varchar(20) DEFAULT NULL COMMENT '메뉴 이미지 파일번호',
  `AD_OUT_YN` varchar(2) DEFAULT NULL COMMENT '광고출력메뉴',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '순번',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`APP_MENU_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='앱메뉴정보';


-- jikyeojo.sg_app_setting definition

CREATE TABLE `sg_app_setting` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `ALARM_USE_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT 'CALL알람 여부',
  `ALARM_INTERVAL_CD` varchar(20) NOT NULL COMMENT 'CALL호출간격',
  `ALARM_SOUND_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '사운드 여부',
  `ALARM_SOUND_NM` varchar(40) NOT NULL COMMENT '사운드명',
  `ALARM_VIB_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '진동 여부',
  `CALL_PHONE_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '보이스톡 여부',
  `CALL_MSG_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '문자여부',
  `LANG_CD` varchar(10) NOT NULL COMMENT '언어',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='앱호출세팅';


-- jikyeojo.sg_ask definition

CREATE TABLE `sg_ask` (
  `ASK_NO` varchar(20) NOT NULL COMMENT '문의번호',
  `ASK_SUBJECT` varchar(100) DEFAULT NULL COMMENT '문의제목',
  `ASK_DESC` varchar(1000) DEFAULT NULL COMMENT '문의내용',
  `EMAIL` varchar(100) DEFAULT NULL COMMENT '이메일',
  `ASK_STAT_CD` varchar(20) DEFAULT NULL COMMENT '문의상태',
  `ASK_STAT_DT` datetime DEFAULT NULL COMMENT '문의상태',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `REPLY_DESC` varchar(1000) DEFAULT NULL COMMENT '답변',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ASK_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='문의정보';


-- jikyeojo.sg_auth_dtl definition

CREATE TABLE `sg_auth_dtl` (
  `AUTH_NO` varchar(20) NOT NULL COMMENT '권한코드',
  `MENU_NO` int(11) NOT NULL COMMENT '메뉴번호',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `WR_YN` char(1) DEFAULT NULL COMMENT '읽기여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`AUTH_NO`,`MENU_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='권한상세';


-- jikyeojo.sg_auth_info definition

CREATE TABLE `sg_auth_info` (
  `AUTH_NO` varchar(20) NOT NULL COMMENT '권한코드',
  `AUTH_NM` varchar(40) DEFAULT NULL COMMENT '권한명',
  `AUTH_TP_CD` varchar(10) DEFAULT NULL COMMENT '권한타입',
  `TOP_MENU_NO` varchar(20) DEFAULT NULL COMMENT '상위메뉴',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`AUTH_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='권한정보';


-- jikyeojo.sg_cas_agree_info definition

CREATE TABLE `sg_cas_agree_info` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `CAS_NO` varchar(20) NOT NULL COMMENT '약관번호',
  `AGREE_YN` char(1) DEFAULT NULL COMMENT '동의여부',
  `AGREE_DT` datetime DEFAULT NULL COMMENT '동의일자',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  PRIMARY KEY (`MEMB_NO`,`CAS_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='약관동의정보';


-- jikyeojo.sg_cas_mng definition

CREATE TABLE `sg_cas_mng` (
  `CAS_NO` varchar(20) NOT NULL COMMENT '약관번호',
  `CAS_TY_CD` varchar(10) DEFAULT NULL COMMENT '약관타입',
  `CAS_NM_KO` varchar(40) DEFAULT NULL COMMENT '약관명',
  `CAS_NM_EN` varchar(40) DEFAULT NULL,
  `CAS_NM_ZH` varchar(40) DEFAULT NULL,
  `CAS_NM_JA` varchar(40) DEFAULT NULL,
  `CAS_DESC_KO` text DEFAULT NULL COMMENT '약관내용',
  `CAS_DESC_EN` text DEFAULT NULL,
  `CAS_DESC_ZH` text DEFAULT NULL,
  `CAS_DESC_JA` text DEFAULT NULL,
  `CAS_CHG_DT` datetime DEFAULT NULL COMMENT '약관변경일',
  `REQUIRED_YN` varchar(2) DEFAULT NULL COMMENT '필수여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '순번',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`CAS_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='약관관리';


-- jikyeojo.sg_cd_dtl definition

CREATE TABLE `sg_cd_dtl` (
  `CODE_CD` varchar(20) NOT NULL COMMENT '코드',
  `CODE_DTL_CD` varchar(10) NOT NULL COMMENT '상세코드',
  `CODE_DTL_NM_KR` varchar(100) DEFAULT NULL COMMENT '상세코드명 한국어',
  `CODE_DTL_NM_EN` varchar(100) DEFAULT NULL COMMENT '상세코드명 영어',
  `CODE_DTL_NM_CH` varchar(100) DEFAULT NULL COMMENT '상세코드명 중국어',
  `CODE_DTL_NM_JP` varchar(100) DEFAULT NULL COMMENT '상세코드명 일본어',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '출력순번',
  `CODE_DESC` varchar(128) DEFAULT NULL COMMENT '비고',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`CODE_CD`,`CODE_DTL_CD`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='코드상세';


-- jikyeojo.sg_cd_info definition

CREATE TABLE `sg_cd_info` (
  `CODE_CD` varchar(20) NOT NULL COMMENT '코드',
  `CODE_TP_CD` varchar(10) DEFAULT NULL COMMENT '코드타입',
  `CODE_NM` varchar(40) DEFAULT NULL COMMENT '코드명',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`CODE_CD`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='코드마스터';


-- jikyeojo.sg_distributor_info definition

CREATE TABLE `sg_distributor_info` (
  `DISTRIBUTOR_NO` varchar(20) NOT NULL COMMENT '가맹점번호',
  `DISTRIBUTOR_NM` varchar(40) DEFAULT NULL COMMENT '가맹점명',
  `USE_STAT_CD` varchar(4) DEFAULT NULL COMMENT '상태코드',
  `BUSI_TYPE_CD` varchar(4) DEFAULT NULL COMMENT '상버구분-법인,개인',
  `BUSI_GRADE_CD` varchar(4) DEFAULT NULL COMMENT '거래처등록 - 총판, 대리점, 개인',
  `BSNS_REG_NO` varchar(10) DEFAULT NULL COMMENT '사업자등록번호',
  `CORP_REG_NO` varchar(13) DEFAULT NULL COMMENT '법인등록번호',
  `BSNS_CNDT` varchar(40) DEFAULT NULL COMMENT '업태',
  `CTGR_BSNS` varchar(40) DEFAULT NULL COMMENT '업종',
  `OWNER_NM` varchar(40) DEFAULT NULL COMMENT '대표자명',
  `OWNER_TEL_NO` varchar(20) DEFAULT NULL COMMENT '대표자연락처',
  `RPRST_FAX_NO` varchar(20) DEFAULT NULL COMMENT '대표팩스번호',
  `RPRST_EMAIL` varchar(50) DEFAULT NULL COMMENT '대표이메일',
  `ZIP_NO` varchar(6) DEFAULT NULL COMMENT '우편번호',
  `ADDR1` varchar(100) DEFAULT NULL COMMENT '주소1',
  `ADDR2` varchar(100) DEFAULT NULL COMMENT '주소2',
  `JOIN_STT_DT` varchar(8) DEFAULT NULL COMMENT '가입시작일자',
  `JOIN_END_DT` varchar(8) DEFAULT NULL COMMENT '가입종료일자',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `QR_DEFIN` varchar(6) DEFAULT NULL COMMENT 'QR정의어',
  `QR_AMT` decimal(20,6) DEFAULT NULL,
  `GIFT_URL` varchar(100) DEFAULT NULL,
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`DISTRIBUTOR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='가맹점기본정보';


-- jikyeojo.sg_faq definition

CREATE TABLE `sg_faq` (
  `FAQ_NO` varchar(20) NOT NULL COMMENT 'FAQ번호',
  `FAQ_TYPE_CD` varchar(100) DEFAULT NULL COMMENT 'FAQ 타입',
  `FAQ_SUBJECT_KR` varchar(100) DEFAULT NULL COMMENT 'FAQ제목',
  `FAQ_SUBJECT_EN` varchar(100) DEFAULT NULL COMMENT 'FAQ제목',
  `FAQ_SUBJECT_CH` varchar(100) DEFAULT NULL COMMENT 'FAQ제목',
  `FAQ_SUBJECT_JP` varchar(100) DEFAULT NULL COMMENT 'FAQ제목',
  `FAQ_DESC_KR` varchar(2000) DEFAULT NULL COMMENT 'FAQ내용',
  `FAQ_DESC_EN` varchar(2000) DEFAULT NULL COMMENT 'FAQ내용',
  `FAQ_DESC_CH` varchar(2000) DEFAULT NULL COMMENT 'FAQ내용',
  `FAQ_DESC_JP` varchar(2000) DEFAULT NULL COMMENT 'FAQ내용',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`FAQ_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='FAQ 정보';


-- jikyeojo.sg_file definition

CREATE TABLE `sg_file` (
  `FILE_NO` varchar(20) NOT NULL COMMENT '파일번호',
  `FILE_SEQ` int(11) NOT NULL COMMENT '파일순번',
  `FILE_PATH` varchar(128) DEFAULT NULL COMMENT '파일경로',
  `LOCAL_PATH` varchar(128) DEFAULT NULL COMMENT '다운로드경로',
  `FILE_NM` varchar(128) DEFAULT NULL COMMENT '파일명',
  `FILE_ORG_NM` varchar(128) DEFAULT NULL COMMENT '파일초기명',
  `FILE_SIZE` varchar(20) DEFAULT NULL COMMENT '파일사이즈',
  `FILE_TYPE` varchar(20) DEFAULT NULL COMMENT '파일타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`FILE_NO`,`FILE_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='파일관리';


-- jikyeojo.sg_gnb_ad definition

CREATE TABLE `sg_gnb_ad` (
  `GNB_NO` varchar(20) NOT NULL COMMENT '앱광로위치번호',
  `GNB_NM` varchar(20) NOT NULL COMMENT '앱광고위치명',
  `AD_OUT_YN` varchar(40) DEFAULT NULL COMMENT '출력여부',
  `AD_INFO` varchar(40) DEFAULT NULL COMMENT '광고정보-출력사이즈',
  `USE_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '사용여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`GNB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='광고출력위치';


-- jikyeojo.sg_good_info definition

CREATE TABLE `sg_good_info` (
  `GOOD_NO` varchar(20) NOT NULL COMMENT '제품파일번호',
  `GOOD_NM` varchar(40) NOT NULL COMMENT '제품명',
  `GOOD_FILE_NO` varchar(20) NOT NULL COMMENT '제품파일번호',
  `GOOD_INFO` varchar(100) DEFAULT NULL COMMENT '제품 로컬정보',
  `GOOD_WIDTH` decimal(10,2) DEFAULT NULL COMMENT '제품 사이즈이미지WIDHT',
  `GOOD_HEIGHT` decimal(10,2) DEFAULT NULL COMMENT '제품 사이즈이미지HEIGHT',
  `QR_X` decimal(10,2) DEFAULT NULL COMMENT 'QR위치X',
  `QR_Y` decimal(10,2) DEFAULT NULL COMMENT 'QR위치 Y',
  `QR_WIDTH` decimal(10,2) DEFAULT NULL COMMENT 'qr길이',
  `QR_HEIGHT` decimal(10,2) DEFAULT NULL COMMENT 'qr 높이',
  `PRINT_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '프린터용지타임 A4, B4',
  `PDF_WIDTH_GAP` decimal(10,2) DEFAULT NULL COMMENT '길이간격 mm',
  `PDF_HEIGHT_GAP` decimal(10,2) DEFAULT NULL COMMENT '높이간격 mm',
  `PNT_WIDHT` decimal(10,2) DEFAULT NULL COMMENT '출력물 길이 cm',
  `PNT_HEIGHT` decimal(10,2) DEFAULT NULL COMMENT '출력물 높이 cm',
  `USE_YN` varchar(2) DEFAULT 'Y' COMMENT '사용여부',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT '삭제여부',
  `GOOD_DESC` varchar(4000) DEFAULT NULL COMMENT '제품설명',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`GOOD_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='제품정보';


-- jikyeojo.sg_intro definition

CREATE TABLE `sg_intro` (
  `INTRO_NO` varchar(20) NOT NULL COMMENT '소개이미지번호',
  `FILE_PATH` varchar(200) DEFAULT NULL COMMENT '이미지파일경로',
  `FILE_URL` varchar(200) DEFAULT NULL COMMENT '이미지파일다운로드 URL',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '노출순번',
  `DEVI_TP_CD` varchar(10) DEFAULT NULL COMMENT '디바이스타입(  SA, SI )',
  `LANG_CD` varchar(10) DEFAULT NULL COMMENT '언어타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`INTRO_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='소개이미지';


-- jikyeojo.sg_manual definition

CREATE TABLE `sg_manual` (
  `MANUAL_NO` varchar(20) NOT NULL COMMENT '매뉴얼번호',
  `FILE_NO` varchar(20) DEFAULT NULL COMMENT '이미지파일번호',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '노출순번',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MANUAL_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='매뉴얼';


-- jikyeojo.sg_memb definition

CREATE TABLE `sg_memb` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `DEVI_ID` varchar(256) DEFAULT NULL COMMENT '단말ID',
  `DEVI_KEY` varchar(256) DEFAULT NULL COMMENT '단말KEY 유일한값 자체생성',
  `DEVI_TP_CD` varchar(10) DEFAULT NULL COMMENT '단말기타입',
  `DEVI_OS_VER` varchar(128) DEFAULT NULL COMMENT '단말OS버전',
  `APP_REG` varchar(250) DEFAULT NULL COMMENT '앱레지스트값',
  `APP_VER` varchar(10) DEFAULT NULL COMMENT 'APP버전',
  `F_MEMB_NM` varchar(40) DEFAULT NULL COMMENT 'FIRST 회원명',
  `L_MEMB_NM` varchar(40) DEFAULT NULL COMMENT 'LAST 회원명',
  `EMAIL` varchar(128) DEFAULT NULL COMMENT '이메일',
  `PHONE_NO` varchar(15) DEFAULT NULL COMMENT '휴대폰번호',
  `COUNTRY_NM` varchar(20) DEFAULT NULL COMMENT '국가명',
  `BIR_DT` varchar(8) DEFAULT NULL COMMENT '생년월일',
  `GEN_CD` varchar(10) DEFAULT NULL COMMENT '성별',
  `NOTI_RCV_YN` char(1) DEFAULT NULL COMMENT '공지수신동의여부',
  `MEMB_STAT_CD` varchar(10) DEFAULT NULL COMMENT '회원상태',
  `STAT_DT` datetime DEFAULT NULL COMMENT '상태날자',
  `MEMB_JOIN_DT` datetime DEFAULT NULL COMMENT '회원가입일',
  `FIRST_DAY` datetime DEFAULT NULL COMMENT '최초실행일',
  `LAST_DAY` datetime DEFAULT NULL COMMENT '마지막실행일',
  `MEMB_EXIT_DT` datetime DEFAULT NULL COMMENT '회원탈퇴일',
  `AD_OUT_YN` varchar(2) DEFAULT 'Y' COMMENT '광고출력여부',
  `NICK_NM` varchar(40) DEFAULT NULL COMMENT '닉네임',
  `INTRODUCTION_INFO` varchar(200) DEFAULT NULL COMMENT '소개글',
  `IMG_FILE_PATH` varchar(200) DEFAULT NULL COMMENT '이미지파일저장경로',
  `IMG_FILE_URL` varchar(200) DEFAULT NULL COMMENT '이미지파일다운로그 url',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='회원정보';


-- jikyeojo.sg_memb_app_setting definition

CREATE TABLE `sg_memb_app_setting` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `ALARM_USE_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT 'CALL알람 여부',
  `ALARM_INTERVAL_CD` varchar(20) NOT NULL COMMENT 'CALL호출간격',
  `ALARM_SOUND_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '사운드 여부',
  `ALARM_SOUND_NM` varchar(40) NOT NULL COMMENT '사운드명',
  `ALARM_VIB_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '진동 여부',
  `CALL_PHONE_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '보이스톡 여부',
  `CALL_MSG_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '문자여부',
  `HIDE_MSG_YN` varchar(2) NOT NULL DEFAULT 'N' COMMENT '메세지숨김여부',
  `LOCATION_YN` varchar(2) NOT NULL DEFAULT 'Y' COMMENT '위치허용여부',
  `LANG_CD` varchar(10) NOT NULL COMMENT '언어',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='앱호출세팅';


-- jikyeojo.sg_memb_appkey definition

CREATE TABLE `sg_memb_appkey` (
  `DEVI_ID` varchar(256) NOT NULL COMMENT '단말ID',
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회워번호',
  `MEMB_CD` varchar(2) DEFAULT NULL COMMENT '회원구분',
  `APP_KEY` varchar(512) DEFAULT NULL COMMENT '앱키',
  `APP_REG` varchar(512) DEFAULT NULL COMMENT 'PUSH발송키',
  `DEL_YN` varchar(2) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  PRIMARY KEY (`DEVI_ID`,`MEMB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='회원앱키정보';


-- jikyeojo.sg_memb_call definition

CREATE TABLE `sg_memb_call` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `CALL_SEQ` varchar(20) NOT NULL COMMENT 'CALL순번',
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR번호',
  `CALL_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '호출타입 - 차량, 인증, 문자, 보이스톡',
  `CALL_REQ_CD` varchar(20) DEFAULT NULL COMMENT '콜 요청 메세지코드',
  `CALL_REQ_MSG` varchar(100) DEFAULT NULL COMMENT '콜 요청 메세지',
  `CALL_REPLY_CD` varchar(20) DEFAULT NULL COMMENT '콜 응답  메세지 코드',
  `CALL_REPLY_MSG` varchar(100) DEFAULT NULL COMMENT '콜 응답 메세지',
  `CALL_STAT_CD` varchar(20) DEFAULT NULL COMMENT '콜상태',
  `CALL_STAT_DT` datetime DEFAULT NULL COMMENT '콜 상태일시',
  `CALL_SUB_SCR` varchar(200) DEFAULT NULL COMMENT '구독정보',
  `REPLY_MEMB_NO` varchar(20) DEFAULT NULL COMMENT '답변회원번호',
  `SUB_SCR` varchar(200) DEFAULT NULL COMMENT 'ROOID번호',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`CALL_SEQ`),
  KEY `SUB_SCR` (`SUB_SCR`),
  KEY `QR_NO` (`QR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='호출정보';


-- jikyeojo.sg_memb_devi_info definition

CREATE TABLE `sg_memb_devi_info` (
  `DEVI_ID` varchar(256) NOT NULL COMMENT '단말ID',
  `DEVI_SEQ` int(11) NOT NULL COMMENT '순번',
  `DEVI_TP_CD` varchar(10) DEFAULT NULL COMMENT '단말기타입',
  `DEVI_OS_VER` varchar(10) DEFAULT NULL COMMENT '단말OS버전',
  `APP_REG` varchar(250) DEFAULT NULL COMMENT '앱레지스트값',
  `APP_VER` varchar(10) DEFAULT NULL COMMENT 'APP버전',
  `LANG_CD` varchar(10) DEFAULT NULL COMMENT '최종 사용언어',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일시',
  `CHG_DT` datetime DEFAULT NULL COMMENT '변경일시',
  PRIMARY KEY (`DEVI_ID`,`DEVI_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='회원단말정보';


-- jikyeojo.sg_memb_friend definition

CREATE TABLE `sg_memb_friend` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `FRIEND_NO` varchar(20) NOT NULL COMMENT '친구회원번호',
  `SERVICE_TYPE_CD` varchar(20) NOT NULL COMMENT '서비스타입',
  `MEMB_QR_NO` varchar(20) NOT NULL COMMENT '회원QR번호',
  `FRIEND_QR_NO` varchar(20) NOT NULL COMMENT '친구QR번호',
  `FRIEND_NICK_NM` varchar(40) DEFAULT NULL COMMENT '친구닉네임',
  `ROOM_NO` varchar(40) DEFAULT NULL COMMENT '방번호',
  `MEMB_LOCATION_CD` varchar(20) DEFAULT NULL COMMENT '회원위치정보상태',
  `MEMB_ALERT_YN` varchar(2) DEFAULT NULL COMMENT '회원위치알럿여부',
  `FRIEND_LOCATION_CD` varchar(20) DEFAULT NULL COMMENT '친구위치정보상태',
  `FRIEND_ALERT_YN` varchar(2) DEFAULT NULL COMMENT '친구위치알럿여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`FRIEND_NO`,`SERVICE_TYPE_CD`),
  KEY `idx_sg_memb_friend_01` (`MEMB_NO`,`SERVICE_TYPE_CD`),
  KEY `idx_sg_memb_friend_02` (`MEMB_NO`,`ROOM_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='친구정보';


-- jikyeojo.sg_memb_qr definition

CREATE TABLE `sg_memb_qr` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR번호',
  `QR_OWNER_CD` varchar(20) DEFAULT NULL COMMENT 'QR소유자구분 - M:MASTER, S:SUB',
  `QR_USE_CD` varchar(20) DEFAULT NULL COMMENT 'QR상태 - 사용,대기,미사용',
  `NICK_NM` varchar(40) DEFAULT NULL COMMENT '명칭',
  `CAR_INFO` varchar(40) DEFAULT NULL COMMENT '차량번호',
  `MAKER_CAR` varchar(40) DEFAULT NULL COMMENT '차종',
  `SUB_SCR` varchar(200) DEFAULT NULL COMMENT 'MQ번호-구독진행',
  `PARK_TEXT` varchar(50) DEFAULT '잠시 주차중입니다.' COMMENT '주차문구',
  `PIN_NO` varchar(10) DEFAULT NULL COMMENT 'PIN번호',
  `INTRODUCTION_INFO` varchar(200) DEFAULT NULL COMMENT '소개글',
  `SERVICE_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '서비스타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`QR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='QR사용자정보';


-- jikyeojo.sg_message definition

CREATE TABLE `sg_message` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `CHAT_SEQ` int(11) NOT NULL COMMENT '메세지순번',
  `MSG_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '메세지타입 T, C, F',
  `MSG_DESC` varchar(4000) DEFAULT NULL COMMENT '메세지',
  `FILE_IMG_URL` varchar(100) DEFAULT NULL COMMENT '파일URL',
  `FILE_PATH` varchar(100) DEFAULT NULL COMMENT '파일경로',
  `WRITE_MEMB_NO` varchar(20) DEFAULT NULL COMMENT '작성자',
  `DELETE_YN` varchar(20) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`CHAT_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='메세지정보';


-- jikyeojo.sg_num definition

CREATE TABLE `sg_num` (
  `NUM_NO` varchar(10) NOT NULL COMMENT '채번코드',
  `DEFI_INFO` varchar(20) DEFAULT NULL COMMENT '정의어',
  `NUM_NM` varchar(40) DEFAULT NULL COMMENT '채번명',
  `YEAR_CHK` char(1) DEFAULT NULL COMMENT '연',
  `MONTH_CHK` char(1) DEFAULT NULL COMMENT '월',
  `DAY_CHK` char(1) DEFAULT NULL COMMENT '일',
  `NUM_ST_NO` int(11) DEFAULT NULL COMMENT '채번식작번호',
  `NUM_ED_NO` int(11) DEFAULT NULL COMMENT '채번종료번호',
  `NUM_USE_NO` int(11) DEFAULT NULL COMMENT '마지막사용번호',
  `NUM_CU_NO` varchar(20) DEFAULT NULL COMMENT '현채번번호',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `USE_DAY` datetime DEFAULT NULL COMMENT '마지막생성일',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`NUM_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='채번정보';


-- jikyeojo.sg_po definition

CREATE TABLE `sg_po` (
  `DISTRIBUTOR_NO` varchar(20) NOT NULL COMMENT '가맹점번호',
  `PO_NO` varchar(20) NOT NULL COMMENT 'PO번호',
  `PO_DT` datetime DEFAULT NULL COMMENT '발행일',
  `PO_USER_ID` varchar(20) NOT NULL COMMENT 'PO발행자ID',
  `PO_REQ_CNT` decimal(10,2) DEFAULT NULL COMMENT '발행요청수',
  `PO_CNT` decimal(10,2) DEFAULT NULL COMMENT '발행수',
  `START_QR_NO` varchar(20) NOT NULL COMMENT '시작QR번호',
  `END_QR_NO` varchar(20) NOT NULL COMMENT '종료QR번호',
  `QR_STICKER_NO` varchar(20) DEFAULT NULL COMMENT 'QR 스티커 파일번호',
  `DOWNLOAD_CNT` decimal(10,2) DEFAULT NULL COMMENT '다운로드수',
  `DOWNLOAD_DT` datetime DEFAULT NULL COMMENT '최근다운로드 일자',
  `DOWNLOAD_USER_ID` varchar(20) DEFAULT NULL COMMENT '최근다운로드 담당자',
  `DOWNLOAD_FILE_NO` varchar(20) DEFAULT NULL COMMENT '최근다운로드 파일번호',
  `DOWNLOAD_ST_CNT` decimal(10,2) DEFAULT NULL COMMENT '다운로드수',
  `DOWNLOAD_PDF_CNT` decimal(10,2) DEFAULT NULL,
  `DOWNLOAD_ST_DT` datetime DEFAULT NULL COMMENT '최근다운로드 일자',
  `DOWNLOAD_PDF_DT` datetime DEFAULT NULL,
  `DOWNLOAD_ST_USER_ID` varchar(20) DEFAULT NULL COMMENT '최근다운로드 담당자',
  `DOWNLOAD_PDF_USER_ID` varchar(20) DEFAULT NULL,
  `DOWNLOAD_ST_FILE_NO` varchar(20) DEFAULT NULL COMMENT '최근다운로드 파일번호',
  `DOWNLOAD_PDF_FILE_NO` varchar(20) DEFAULT NULL,
  `UNIT_PRICE_AMT` decimal(20,6) DEFAULT NULL COMMENT '개별단가',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`DISTRIBUTOR_NO`,`PO_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='거래처 QR발급리스트';


-- jikyeojo.sg_qr_info definition

CREATE TABLE `sg_qr_info` (
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR파일번호',
  `QR_FILE_NO` varchar(20) NOT NULL COMMENT 'QR파일번호',
  `QR_INFO` varchar(100) DEFAULT NULL COMMENT 'QR 내부 URL 정보',
  `LOW_QR_INFO` varchar(100) DEFAULT NULL COMMENT '앱에서보는 사이즈이미지URL',
  `USE_YN` varchar(2) DEFAULT 'Y' COMMENT 'QR 사용여부',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT 'QR 삭제여부',
  `SERVICE_TYPE_CD` varchar(20) DEFAULT 'N' COMMENT '서비스 타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`QR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='QR생성정보';


-- jikyeojo.sg_qr_po definition

CREATE TABLE `sg_qr_po` (
  `DISTRIBUTOR_NO` varchar(20) NOT NULL COMMENT '가맹점번호',
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR번호',
  `QR_FILE_NO` varchar(20) DEFAULT NULL COMMENT 'QR파일번호',
  `QR_INFO` varchar(100) DEFAULT NULL COMMENT 'QR 내부 URL 정보',
  `QR_STICKER_NO` varchar(20) DEFAULT NULL COMMENT 'QR 스티커 파일번호',
  `QR_STICKER_INFO` varchar(100) DEFAULT NULL COMMENT 'QR 스티커 정보',
  `APPLY_YN` varchar(2) DEFAULT 'N' COMMENT '적용여부',
  `PO_NO` varchar(20) DEFAULT NULL COMMENT 'PO번호',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT '삭제여부',
  `CREATE_DT` varchar(8) DEFAULT 'N' COMMENT '생성일자',
  `APPLY_DT` varchar(8) DEFAULT 'N' COMMENT '적용일자',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`DISTRIBUTOR_NO`,`QR_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='거래처 QR발급리스트';


-- jikyeojo.sg_room_del_message definition

CREATE TABLE `sg_room_del_message` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `CHAT_SEQ` int(11) NOT NULL COMMENT '삭제 생성 순번',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`MEMB_NO`,`CHAT_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='메세지삭제정보';


-- jikyeojo.sg_room_info definition

CREATE TABLE `sg_room_info` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `ROOM_NM` varchar(40) NOT NULL COMMENT '방명',
  `ROOM_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '방타입 Group Sigle',
  `LAST_CHAT_SEQ` int(11) DEFAULT NULL COMMENT '마지막번호',
  `SERVICE_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '서비스타입',
  `QR_NO` varchar(20) DEFAULT NULL COMMENT 'qr번호',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='방정보';


-- jikyeojo.sg_room_member definition

CREATE TABLE `sg_room_member` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `MEMB_SEQ` varchar(40) NOT NULL COMMENT '방명',
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR번호',
  `FIRST_CHAT_SEQ` int(11) DEFAULT NULL COMMENT '최초 생성 순번',
  `WRITE_CHAT_SEQ` int(11) DEFAULT NULL COMMENT '마지막작성순번',
  `READ_CHAT_SEQ` int(11) DEFAULT NULL COMMENT '마지막읽음순번',
  `EXIT_CHAT_SEQ` int(11) DEFAULT NULL COMMENT '탈퇴마지막순번',
  `FRIEND_JOIN_YN` varchar(2) DEFAULT 'N' COMMENT '친구추가여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`MEMB_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='방멤버정보';


-- jikyeojo.sg_seller_info definition

CREATE TABLE `sg_seller_info` (
  `SELLER_USER_ID` varchar(20) NOT NULL COMMENT '사용자ID',
  `DISTRIBUTOR_NO` varchar(20) NOT NULL COMMENT '가맹점번호',
  `SELLER_USER_NM` varchar(40) DEFAULT NULL COMMENT '담당자명',
  `SELLER_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '사업구분',
  `USE_STAT_CD` varchar(20) DEFAULT NULL COMMENT '상태',
  `TEL_NO` varchar(20) DEFAULT NULL COMMENT '연락처',
  `EMAIL` varchar(50) DEFAULT NULL COMMENT '메일',
  `JOIN_STT_DT` varchar(8) DEFAULT NULL COMMENT '가입시작일자',
  `JOIN_END_DT` varchar(8) DEFAULT NULL COMMENT '가입종료일자',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`SELLER_USER_ID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='영업담당자 기본 정보';


-- jikyeojo.sg_sys_menu definition

CREATE TABLE `sg_sys_menu` (
  `MENU_NO` int(11) NOT NULL AUTO_INCREMENT COMMENT '메뉴번호',
  `MENU_ID` varchar(20) DEFAULT NULL COMMENT '메뉴아이디',
  `TOP_MENU_NO` int(11) DEFAULT NULL COMMENT '상위메뉴번호',
  `MENU_NM` varchar(40) DEFAULT NULL COMMENT '메뉴명',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '출력순번',
  `CONN_URL` varchar(128) DEFAULT NULL COMMENT '접속URL',
  `ICON_YN` char(1) DEFAULT NULL COMMENT '아이콘여부',
  `ROLE_DESC` varchar(128) DEFAULT NULL COMMENT '역할내역',
  `OPEN_CD` varchar(10) DEFAULT NULL COMMENT 'View구분',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `SYS_TYPE_CD` varchar(10) DEFAULT NULL COMMENT '시스템구분 W 웹 , M 모바일',
  PRIMARY KEY (`MENU_NO`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=utf8 COMMENT='시스템메뉴정보';


-- jikyeojo.sg_tuto definition

CREATE TABLE `sg_tuto` (
  `TUTO_NO` varchar(20) NOT NULL COMMENT '튜토리얼번호',
  `FILE_NO` varchar(20) DEFAULT NULL COMMENT '이미지파일번호',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '노출순번',
  `DEVI_TP_CD` varchar(10) DEFAULT NULL COMMENT '단말기타입',
  `LANG_CD` varchar(10) DEFAULT NULL COMMENT '언어',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`TUTO_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='튜토리얼';


-- jikyeojo.sg_user_auth definition

CREATE TABLE `sg_user_auth` (
  `USER_ID` varchar(20) NOT NULL COMMENT '사용자ID',
  `AUTH_NO` varchar(20) NOT NULL COMMENT '권한코드',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  PRIMARY KEY (`USER_ID`,`AUTH_NO`),
  KEY `R_6` (`AUTH_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='사용자권한';


-- jikyeojo.sg_user_info definition

CREATE TABLE `sg_user_info` (
  `USER_ID` varchar(20) NOT NULL COMMENT '사용자ID',
  `USER_NM` varchar(40) DEFAULT NULL COMMENT '사용자명',
  `TEL_NO` varchar(20) DEFAULT NULL COMMENT '전화번호',
  `EMAIL` varchar(128) DEFAULT NULL COMMENT '이메일',
  `USER_PW` varchar(128) DEFAULT NULL COMMENT '비밀번호',
  `FIRST_LOGIN_DT` datetime DEFAULT NULL COMMENT '최초로그인일자',
  `LAST_LOGIN_DT` datetime DEFAULT NULL COMMENT '최종로그인일자',
  `PW_ERR_CNT` int(11) DEFAULT NULL COMMENT '오류횟수',
  `USER_STAT_CD` varchar(10) DEFAULT NULL COMMENT '사용자상태(10:활성,20:잠김,30:보류,90:삭제)',
  `USER_STAT_DT` datetime DEFAULT NULL,
  `AUTH_TP_CD` varchar(10) DEFAULT NULL COMMENT '권한타입(AUTH_TP_CD )',
  `DISTRIBUTOR_NO` varchar(20) DEFAULT NULL COMMENT '가맹점코드',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  `add_dt` datetime DEFAULT NULL,
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `ROLE` varbinary(255) DEFAULT NULL,
  PRIMARY KEY (`USER_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='사용자정보 ';


-- jikyeojo.sg_ver_info definition

CREATE TABLE `sg_ver_info` (
  `APP_NO` varchar(10) NOT NULL COMMENT 'APP번호',
  `DEVI_TP_CD` varchar(10) DEFAULT NULL COMMENT '단말타입',
  `APP_VER` varchar(10) DEFAULT NULL COMMENT 'APP버전',
  `DES_VER` varchar(128) DEFAULT NULL COMMENT '버전설명',
  `APP_VER_DT` datetime DEFAULT NULL COMMENT '버전생성일',
  `UP_REQUIRE_YN` varchar(2) DEFAULT NULL COMMENT '업그레이드필수여부',
  `STORE_URL` varchar(100) DEFAULT NULL COMMENT '다운로드경로',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`APP_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='앱버전정보';

## parse_ddl
#!/usr/bin/env python3
"""
DDL to SQLAlchemy Model Generator
DDL 스키마를 파싱하여 SQLAlchemy 모델을 자동으로 생성합니다.
"""

import re
import sys
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
from enum import Enum

class ColumnType(Enum):
    INTEGER = "Integer"
    BIGINT = "BigInteger"
    SMALLINT = "SmallInteger"
    FLOAT = "Float"
    DECIMAL = "Numeric"
    VARCHAR = "String"
    CHAR = "String"
    TEXT = "Text"
    BOOLEAN = "Boolean"
    DATE = "Date"
    DATETIME = "DateTime"
    TIMESTAMP = "DateTime"
    JSON = "JSON"
    BLOB = "LargeBinary"
    CLOB = "Text"
    STRING = "String"
    NUMERIC = "Numeric"

@dataclass
class Column:
    name: str
    type: str
    nullable: bool = True
    primary_key: bool = False
    auto_increment: bool = False
    unique: bool = False
    default: Optional[str] = None
    length: Optional[int] = None
    precision: Optional[int] = None
    scale: Optional[int] = None
    comment: Optional[str] = None

@dataclass
class Table:
    name: str
    columns: List[Column]
    primary_keys: List[str]
    foreign_keys: List[Dict]
    indexes: List[Dict]
    comment: Optional[str] = None

class DDLParser:
    def __init__(self):
        self.tables: List[Table] = []
        
    def parse_ddl(self, ddl_content: str) -> List[Table]:
        """DDL 내용을 파싱하여 테이블 정보를 추출합니다."""
        # 주석 제거
        ddl_content = self._remove_comments(ddl_content)
        
        # CREATE TABLE 문들을 찾기 - 중첩된 괄호를 올바르게 처리
        # 더 정확한 패턴: 테이블명과 테이블 본문을 분리하여 추출
        create_table_pattern = r'CREATE\s+TABLE\s+(?:IF\s+NOT\s+EXISTS\s+)?`?(\w+)`?\s*\((.*?)\)\s*;'
        matches = re.finditer(create_table_pattern, ddl_content, re.IGNORECASE | re.DOTALL)
        for match in matches:
            table_name = match.group(1)
            table_body = match.group(2)
            print("@@ parse_ddl table_name:", table_name)
            print("@@ parse_ddl table_body:", table_body[:100] + "..." if len(table_body) > 100 else table_body)
            table = self._parse_table(table_name, table_body)
            if table:
                self.tables.append(table)
            # test 
            return self.tables
        
        return self.tables
    
    def _remove_comments(self, content: str) -> str:
        """SQL 주석을 제거합니다."""
        # -- 주석 제거
        content = re.sub(r'--.*$', '', content, flags=re.MULTILINE)
        # /* */ 주석 제거
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        return content
    
    def _parse_table(self, table_name: str, table_body: str) -> Optional[Table]:
        """테이블 본문을 파싱합니다."""
        columns = []
        primary_keys = []
        foreign_keys = []
        indexes = []
        
        print("@@ _parse_table table_body: ", table_body)
        # 각 라인을 처리
        lines = [line.strip() for line in table_body.split('\n') if line.strip()]
        
        for line in lines:
            line = line.strip().rstrip(',')
            print("@@ _parse_table line: ", line)
            if not line:
                continue
                
            # PRIMARY KEY 처리
            if re.match(r'PRIMARY\s+KEY', line, re.IGNORECASE):
                pk_match = re.search(r'PRIMARY\s+KEY\s*\(([^)]+)\)', line, re.IGNORECASE)
                if pk_match:
                    pk_columns = [col.strip().strip('`') for col in pk_match.group(1).split(',')]
                    primary_keys.extend(pk_columns)
                continue
            
            # FOREIGN KEY 처리
            if re.match(r'FOREIGN\s+KEY', line, re.IGNORECASE):
                fk_match = re.search(r'FOREIGN\s+KEY\s*\(([^)]+)\)\s+REFERENCES\s+`?(\w+)`?\s*\(([^)]+)\)', line, re.IGNORECASE)
                if fk_match:
                    foreign_keys.append({
                        'column': fk_match.group(1).strip().strip('`'),
                        'ref_table': fk_match.group(2),
                        'ref_column': fk_match.group(3).strip().strip('`')
                    })
                continue
            
            # INDEX 처리
            if re.match(r'(?:UNIQUE\s+)?KEY\s+`?\w+`?\s*\(', line, re.IGNORECASE):
                index_match = re.search(r'(?:UNIQUE\s+)?KEY\s+`?(\w+)`?\s*\(([^)]+)\)', line, re.IGNORECASE)
                if index_match:
                    indexes.append({
                        'name': index_match.group(1),
                        'columns': [col.strip().strip('`') for col in index_match.group(2).split(',')],
                        'unique': 'UNIQUE' in line.upper()
                    })
                continue
            
            # 컬럼 정의 처리
            column = self._parse_column(line)
            if column:
                columns.append(column)
        
        if columns:
            return Table(
                name=table_name,
                columns=columns,
                primary_keys=primary_keys,
                foreign_keys=foreign_keys,
                indexes=indexes
            )
        return None
    
    def _parse_column(self, line: str) -> Optional[Column]:
        """컬럼 정의를 파싱합니다."""
        # 컬럼명 추출 - 괄호를 포함한 타입도 추출하도록 수정
        col_match = re.match(r'`?(\w+)`?\s+([^,\s]+(?:\([^)]*\))?)', line)
        if not col_match:
            print("@@ _parse_column error", line)
            return None
            
        col_name = col_match.group(1)
        col_type = col_match.group(2).upper()
        # 타입 파싱
        sqlalchemy_type, length, precision, scale = self._parse_type(col_type)
        
        # 제약조건 파싱
        nullable = 'NOT NULL' not in line.upper()
        primary_key = 'PRIMARY KEY' in line.upper()
        auto_increment = 'AUTO_INCREMENT' in line.upper() or 'IDENTITY' in line.upper()
        unique = 'UNIQUE' in line.upper()
        
        # 기본값 파싱
        default_match = re.search(r'DEFAULT\s+([^,\s]+)', line, re.IGNORECASE)
        default_value = default_match.group(1) if default_match else None
        
        # MySQL 커멘트 파싱 - 다양한 형식 지원
        comment = self._extract_comment(line)
        
        return Column(
            name=col_name,
            type=sqlalchemy_type,
            nullable=nullable,
            primary_key=primary_key,
            auto_increment=auto_increment,
            unique=unique,
            default=default_value,
            length=length,
            precision=precision,
            scale=scale,
            comment=comment
        )
    
    def _extract_comment(self, line: str) -> Optional[str]:
        """MySQL 커멘트를 추출합니다. 다양한 형식을 지원합니다."""
        # 1. COMMENT 'comment' 형식 (작은따옴표)
        comment_match = re.search(r"COMMENT\s+'([^']*)'", line, re.IGNORECASE)
        if comment_match:
            return comment_match.group(1)
        
        # 2. COMMENT "comment" 형식 (큰따옴표)
        comment_match = re.search(r'COMMENT\s+"([^"]*)"', line, re.IGNORECASE)
        if comment_match:
            return comment_match.group(1)
        
        # 3. COMMENT `comment` 형식 (백틱)
        comment_match = re.search(r'COMMENT\s+`([^`]*)`', line, re.IGNORECASE)
        if comment_match:
            return comment_match.group(1)
        
        # 4. COMMENT comment 형식 (따옴표 없음, 공백으로 구분)
        comment_match = re.search(r'COMMENT\s+([^\s,]+)', line, re.IGNORECASE)
        if comment_match:
            return comment_match.group(1)
        
        # 5. COMMENT 뒤에 오는 전체 문자열 (마지막 쉼표나 세미콜론까지)
        comment_match = re.search(r'COMMENT\s+(.+?)(?:\s*,\s*|\s*$)', line, re.IGNORECASE)
        if comment_match:
            comment_text = comment_match.group(1).strip()
            # 따옴표 제거
            if comment_text.startswith("'") and comment_text.endswith("'"):
                return comment_text[1:-1]
            elif comment_text.startswith('"') and comment_text.endswith('"'):
                return comment_text[1:-1]
            elif comment_text.startswith('`') and comment_text.endswith('`'):
                return comment_text[1:-1]
            else:
                return comment_text
        
        return None
    
    def _parse_type(self, type_str: str) -> Tuple[str, Optional[int], Optional[int], Optional[int]]:
        """SQL 타입을 SQLAlchemy 타입으로 변환합니다."""
        type_str = type_str.upper()
        
        # VARCHAR(n)
        if 'VARCHAR' in type_str:
            length = self._extract_length(type_str)
            return ColumnType.VARCHAR.value, length, None, None
        
        # CHAR(n)
        elif 'CHAR' in type_str and 'VARCHAR' not in type_str:
            length = self._extract_length(type_str)
            return ColumnType.CHAR.value, length, None, None
        
        # INT, INTEGER
        elif any(t in type_str for t in ['INT', 'INTEGER']):
            if 'BIG' in type_str:
                return ColumnType.BIGINT.value, None, None, None
            elif 'SMALL' in type_str:
                return ColumnType.SMALLINT.value, None, None, None
            else:
                return ColumnType.INTEGER.value, None, None, None
        
        # DECIMAL, NUMERIC
        elif any(t in type_str for t in ['DECIMAL', 'NUMERIC']):
            precision, scale = self._extract_precision_scale(type_str)
            return ColumnType.DECIMAL.value, None, precision, scale
        
        # FLOAT, DOUBLE
        elif any(t in type_str for t in ['FLOAT', 'DOUBLE']):
            return ColumnType.FLOAT.value, None, None, None
        
        # TEXT
        elif 'TEXT' in type_str:
            return ColumnType.TEXT.value, None, None, None
        
        # BOOLEAN, BOOL
        elif any(t in type_str for t in ['BOOLEAN', 'BOOL']):
            return ColumnType.BOOLEAN.value, None, None, None
        
        # DATE
        elif 'DATE' in type_str and 'TIME' not in type_str:
            return ColumnType.DATE.value, None, None, None
        
        # DATETIME, TIMESTAMP
        elif any(t in type_str for t in ['DATETIME', 'TIMESTAMP']):
            return ColumnType.DATETIME.value, None, None, None
        
        # JSON
        elif 'JSON' in type_str:
            return ColumnType.JSON.value, None, None, None
        
        # BLOB
        elif 'BLOB' in type_str:
            return ColumnType.BLOB.value, None, None, None
        
        # 기본값
        else:
            return ColumnType.VARCHAR.value, None, None, None
    
    def _extract_length(self, type_str: str) -> Optional[int]:
        """타입에서 길이를 추출합니다."""
        match = re.search(r'\((\d+)\)', type_str)
        return int(match.group(1)) if match else None
    
    def _extract_precision_scale(self, type_str: str) -> Tuple[Optional[int], Optional[int]]:
        """타입에서 precision과 scale을 추출합니다."""
        match = re.search(r'\((\d+)(?:,(\d+))?\)', type_str)
        if match:
            precision = int(match.group(1))
            scale = int(match.group(2)) if match.group(2) else None
            return precision, scale
        return None, None

class SQLAlchemyGenerator:
    def __init__(self):
        self.imports = set()
        self.relationships = []
    
    def generate_models(self, tables: List[Table], output_file: str = None) -> str:
        """테이블 정보를 바탕으로 SQLAlchemy 모델을 생성합니다."""
        self.imports.clear()
        self.relationships.clear()
        # 필요한 import 수집
        self._collect_imports(tables)
        
        # 관계 설정 수집
        self._collect_relationships(tables)
        
        # 모델 코드 생성
        code = self._generate_code(tables)

        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(code)
        
        return code
    
    def _collect_imports(self, tables: List[Table]):
        """필요한 import를 수집합니다."""
        self.imports.add("from sqlalchemy import Column, Integer, String, Text, Boolean, Date, DateTime, Numeric, BigInteger, SmallInteger, Float, JSON, LargeBinary, ForeignKey")
        self.imports.add("from sqlalchemy.ext.declarative import declarative_base")
        self.imports.add("from sqlalchemy.orm import relationship")
        self.imports.add("from datetime import datetime")
        
        # 사용되는 타입들 확인
        used_types = set()
        for table in tables:
            for column in table.columns:
                used_types.add(column.type)
        
        # 필요한 import 추가
        if any('Numeric' in t for t in used_types):
            self.imports.add("from decimal import Decimal")
    
    def _collect_relationships(self, tables: List[Table]):
        """외래키 관계를 수집합니다."""
        for table in tables:
            for fk in table.foreign_keys:
                self.relationships.append({
                    'table': table.name,
                    'column': fk['column'],
                    'ref_table': fk['ref_table'],
                    'ref_column': fk['ref_column']
                })
    
    def _generate_code(self, tables: List[Table]) -> str:
        """전체 코드를 생성합니다."""
        code_lines = []
        
        # 헤더
        code_lines.append('"""')
        code_lines.append('Auto-generated SQLAlchemy models from DDL')
        code_lines.append('Generated by DDL to SQLAlchemy Generator')
        code_lines.append('"""')
        code_lines.append('')
        
        # Imports
        for import_stmt in sorted(self.imports):
            code_lines.append(import_stmt)
        code_lines.append('')
        
        # Base class
        code_lines.append('Base = declarative_base()')
        code_lines.append('')
        
        # Models
        for table in tables:
            model_code = self._generate_model(table)
            code_lines.extend(model_code)
            code_lines.append('')
        
        return '\n'.join(code_lines)
    
    def _generate_model(self, table: Table) -> List[str]:
        """개별 모델을 생성합니다."""
        lines = []
        
        # 클래스 정의
        class_name = self._to_camel_case(table.name)
        lines.append(f'class {class_name}(Base):')
        print("@@ generate_model 1 "+class_name)
        # 테이블명
        lines.append(f'    __tablename__ = "{table.name}"')
        lines.append('')
        print("@@ generate_model 2", table.columns)
        # 컬럼들
        for column in table.columns:
            col_lines = self._generate_column(column)
            lines.extend(col_lines)

        # 관계 설정
        relationships = self._generate_relationships(table)
        if relationships:
            lines.append('')
            lines.extend(relationships)
        print("@@ generate_model 3")
        # __repr__ 메서드
        lines.append('')
        lines.append('    def __repr__(self):')
        primary_key = next((col for col in table.columns if col.primary_key), None)
        if primary_key:
            lines.append(f'        return f"<{class_name}({primary_key.name}={{self.{primary_key.name}}})>"')
        else:
            lines.append(f'        return f"<{class_name}>"')
        
        return lines
    
    def _generate_column(self, column: Column) -> List[str]:
        """컬럼을 생성합니다."""
        lines = []
        
        # 컬럼 정의 시작
        col_def = f'    {column.name} = Column('
        
        # 타입

        if column.type == ColumnType.STRING.value and column.length:
            col_def += f'String({column.length})'
        elif column.type == ColumnType.NUMERIC.value and column.precision:
            if column.scale:
                col_def += f'Numeric({column.precision}, {column.scale})'
            else:
                col_def += f'Numeric({column.precision})'
        else:
            col_def += column.type + '()'
        
        # 제약조건들
        constraints = []
        
        if column.primary_key:
            constraints.append('primary_key=True')
        
        if not column.nullable:
            constraints.append('nullable=False')
        
        if column.unique:
            constraints.append('unique=True')
        
        if column.auto_increment:
            constraints.append('autoincrement=True')
        
        if column.default:
            if column.default.upper() in ['NULL', 'CURRENT_TIMESTAMP']:
                constraints.append(f"default={column.default}")
            else:
                constraints.append(f"default='{column.default}'")
        
        if constraints:
            col_def += ', ' + ', '.join(constraints)
        
        col_def += ')'
        
        # 주석
        print("@@ generate_column 1", column.comment)
        if column.comment:
            lines.append(f'    # {column.comment}')
        
        lines.append(col_def)
        return lines
    
    def _generate_relationships(self, table: Table) -> List[str]:
        """관계를 생성합니다."""
        lines = []
        
        for fk in table.foreign_keys:
            ref_class = self._to_camel_case(fk['ref_table'])
            lines.append(f'    {fk["ref_table"]} = relationship("{ref_class}", back_populates="{table.name}")')
        
        return lines
    
    def _to_camel_case(self, snake_str: str) -> str:
        """snake_case를 CamelCase로 변환합니다."""
        components = snake_str.split('_')
        return ''.join(word.capitalize() for word in components)

def main():
    """메인 함수"""
    if len(sys.argv) < 2:
        print("Usage: python ddl_to_sqlalchemy.py <ddl_file> [output_file]")
        sys.exit(1)
    
    ddl_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    try:
        # DDL 파일 읽기
        with open(ddl_file, 'r', encoding='utf-8') as f:
            ddl_content = f.read()
        
        # 파싱
        parser = DDLParser()
        tables = parser.parse_ddl(ddl_content)
        
        if not tables:
            print("No tables found in DDL file")
            sys.exit(1)
        
        # SQLAlchemy 모델 생성
        generator = SQLAlchemyGenerator()
        code = generator.generate_models(tables, output_file)
        
        if not output_file:
            print(code)
        else:
            print(f"Generated SQLAlchemy models saved to {output_file}")
            print(f"Found {len(tables)} tables:")
            for table in tables:
                print(f"  - {table.name} ({len(table.columns)} columns)")
    
    except FileNotFoundError:
        print(f"Error: File '{ddl_file}' not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main() 
