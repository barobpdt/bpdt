-- secretguard.sg_act_info definition

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


-- secretguard.sg_announce definition

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


-- secretguard.sg_app_menu definition

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


-- secretguard.sg_app_setting definition

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


-- secretguard.sg_ask definition

CREATE TABLE `sg_ask` (
  `ASK_NO` varchar(20) NOT NULL COMMENT '문의번호',
  `ASK_SUBJECT` varchar(100) DEFAULT NULL COMMENT '문의제목',
  `ASK_DESC` varchar(1000) DEFAULT NULL COMMENT '문의내용',
  `EMAIL` varchar(100) DEFAULT NULL COMMENT '이메일',
  `ASK_STAT_CD` varchar(20) DEFAULT NULL COMMENT '문의상태',
  `ASK_STAT_DT` datetime DEFAULT NULL COMMENT '문의상태',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `REPLY_DESC` varchar(1000) DEFAULT NULL COMMENT '답변내용',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ASK_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='문의정보';


-- secretguard.sg_auth_dtl definition

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


-- secretguard.sg_auth_info definition

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


-- secretguard.sg_call_message definition

CREATE TABLE `sg_call_message` (
  `MEMB_NO` varchar(40) NOT NULL COMMENT '방번호',
  `QR_NO` varchar(20) NOT NULL DEFAULT '1' COMMENT '메세지순번',
  `MSG_SEQ` int(11) NOT NULL DEFAULT 1 COMMENT '대화 순번',
  `MSG_DESC` varchar(4000) DEFAULT NULL COMMENT '메세지',
  `CALL_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '문자, 통화 구분',
  `WRITE_MEMB_NO` varchar(20) DEFAULT NULL COMMENT '작성자',
  `REAL_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '응답SEQ',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`QR_NO`,`MSG_SEQ`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='차빼줘 메세지이력 정보';


-- secretguard.sg_cas_agree_info definition

CREATE TABLE `sg_cas_agree_info` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `CAS_NO` varchar(20) NOT NULL COMMENT '약관번호',
  `CAS_SEQ` int(11) NOT NULL DEFAULT 1 COMMENT '약관순번',
  `AGREE_YN` char(1) DEFAULT NULL COMMENT '동의여부',
  `AGREE_DT` datetime DEFAULT NULL COMMENT '동의일자',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  PRIMARY KEY (`MEMB_NO`,`CAS_NO`,`CAS_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='약관동의정보';


-- secretguard.sg_cas_mng definition

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


-- secretguard.sg_cd_dtl definition

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


-- secretguard.sg_cd_info definition

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


-- secretguard.sg_distributor_info definition

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
  `QR_DEFIN` varchar(6) DEFAULT NULL COMMENT 'QR정의어',
  `ZIP_NO` varchar(6) DEFAULT NULL COMMENT '우편번호',
  `ADDR1` varchar(100) DEFAULT NULL COMMENT '주소1',
  `ADDR2` varchar(100) DEFAULT NULL COMMENT '주소2',
  `JOIN_STT_DT` varchar(8) DEFAULT NULL COMMENT '가입시작일자',
  `JOIN_END_DT` varchar(8) DEFAULT NULL COMMENT '가입종료일자',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `QR_AMT` decimal(20,6) DEFAULT NULL COMMENT 'qr가격',
  `GIFT_URL` varchar(100) DEFAULT NULL COMMENT '선물URL',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`DISTRIBUTOR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='가맹점기본정보';


-- secretguard.sg_faq definition

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


-- secretguard.sg_file definition

CREATE TABLE `sg_file` (
  `FILE_NO` varchar(20) NOT NULL COMMENT '파일번호',
  `FILE_SEQ` int(11) NOT NULL COMMENT '파일순번',
  `FILE_PATH` varchar(200) DEFAULT NULL COMMENT '파일경로',
  `LOCAL_PATH` varchar(200) DEFAULT NULL COMMENT '다운로드경로',
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


-- secretguard.sg_gnb_ad definition

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


-- secretguard.sg_good_info definition

CREATE TABLE `sg_good_info` (
  `GOOD_NO` varchar(20) NOT NULL COMMENT '제품파일번호',
  `GOOD_NM` varchar(40) NOT NULL COMMENT '제품명',
  `GOOD_FILE_NO` varchar(20) NOT NULL COMMENT '제품파일번호',
  `GOOD_INFO` varchar(100) DEFAULT NULL COMMENT '제품 로컬정보',
  `GOOD_WIDTH` decimal(10,2) DEFAULT NULL COMMENT '제품 사이즈이미지WIDHT',
  `GOOD_HEIGHT` decimal(10,2) DEFAULT NULL COMMENT '제품 사이즈이미지HEIGHT',
  `QR_X` decimal(10,2) DEFAULT NULL COMMENT 'QR위치X',
  `QR_Y` decimal(10,2) DEFAULT NULL COMMENT 'QR위치 Y',
  `QR_WIDTH` decimal(10,2) DEFAULT NULL COMMENT 'qr 길이',
  `QR_HEIGHT` decimal(10,2) DEFAULT NULL COMMENT 'QR 높이',
  `PRINT_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '용지타입 A4, B4',
  `PDF_WIDTH_GAP` decimal(10,2) DEFAULT NULL COMMENT 'PDF 간격 WIDTH mm',
  `PDF_HEIGHT_GAP` decimal(10,2) DEFAULT NULL COMMENT 'PDF 간격 HEIGHT mm',
  `PNT_WIDTH` decimal(10,2) DEFAULT NULL COMMENT '출력물길이 CM',
  `PNT_HEIGHT` decimal(10,2) DEFAULT NULL COMMENT '출력물높이 CM',
  `USE_YN` varchar(2) DEFAULT 'Y' COMMENT '사용여부',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT '삭제여부',
  `GOOD_DESC` varchar(4000) DEFAULT NULL COMMENT '제품설명',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`GOOD_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='제품정보';


-- secretguard.sg_intro definition

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


-- secretguard.sg_last_message definition

CREATE TABLE `sg_last_message` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `CHAT_SEQ` varchar(20) DEFAULT '' COMMENT '메세지순번',
  `MSG_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '메세지타입 T, C, F',
  `MSG_DESC` varchar(4000) DEFAULT NULL COMMENT '메세지',
  `FILE_IMG_URL` varchar(100) DEFAULT NULL COMMENT '파일URL',
  `FILE_PATH` varchar(100) DEFAULT NULL COMMENT '파일경로',
  `WRITE_MEMB_NO` varchar(20) DEFAULT NULL COMMENT '작성자',
  `REAL_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '관련 번호',
  `DELETE_YN` varchar(20) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='마지막 메세지정보';


-- secretguard.sg_manual definition

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


-- secretguard.sg_memb definition

CREATE TABLE `sg_memb` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `DEVI_ID` varchar(256) DEFAULT NULL COMMENT '단말ID',
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
  `NICK_NM` varchar(40) DEFAULT 'Y' COMMENT '닉네임',
  `PIN_NO` varchar(10) DEFAULT 'Y' COMMENT 'PIN번호-패스워드',
  `INTRODUCTION_INFO` varchar(200) DEFAULT 'Y' COMMENT '소개글',
  `IMG_FILE_PATH` varchar(200) DEFAULT 'Y' COMMENT '이미지파일저장경로',
  `IMG_FILE_URL` varchar(200) DEFAULT 'Y' COMMENT '이미지파일다운로드URL',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='회원정보';


-- secretguard.sg_memb_app_setting definition

CREATE TABLE `sg_memb_app_setting` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `ALARM_USE_YN` varchar(2) DEFAULT 'Y' COMMENT 'CALL알람 여부',
  `ALARM_INTERVAL_CD` varchar(20) DEFAULT NULL COMMENT 'CALL호출간격',
  `ALARM_SOUND_YN` varchar(2) DEFAULT 'Y' COMMENT '사운드 여부',
  `ALARM_SOUND_NM` varchar(40) DEFAULT NULL COMMENT '사운드명',
  `ALARM_VIB_YN` varchar(2) DEFAULT 'Y' COMMENT '진동 여부',
  `CALL_PHONE_YN` varchar(2) DEFAULT 'Y' COMMENT '보이스톡 여부',
  `CALL_MSG_YN` varchar(2) DEFAULT 'Y' COMMENT '문자여부',
  `HIDE_MSG_YN` varchar(2) DEFAULT 'N' COMMENT '메세지숨김여부',
  `LOCATION_YN` varchar(2) DEFAULT 'Y' COMMENT '위치허용여부',
  `LANG_CD` varchar(10) DEFAULT NULL COMMENT '언어',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='앱호출세팅';


-- secretguard.sg_memb_appkey definition

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


-- secretguard.sg_memb_call definition

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
  `WRITE_MEMB_NO` varchar(40) DEFAULT NULL COMMENT '보낸자qr - WEB은 MYKYEY값',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`CALL_SEQ`),
  KEY `SUB_SCR` (`SUB_SCR`),
  KEY `QR_NO` (`QR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='호출정보';


-- secretguard.sg_memb_call_new definition

CREATE TABLE `sg_memb_call_new` (
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
  `WRITE_MEMB_NO` varchar(40) DEFAULT NULL COMMENT '보낸자qr - WEB은 MYKYEY값',
  `CALL_QR_NO` varchar(40) DEFAULT NULL COMMENT '호출자qr번호 - NULL이면 차주',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`CALL_SEQ`) USING BTREE,
  KEY `SUB_SCR` (`SUB_SCR`) USING BTREE,
  KEY `QR_NO` (`QR_NO`) USING BTREE,
  KEY `callSeq` (`CALL_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='호출정보';


-- secretguard.sg_memb_devi_info definition

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


-- secretguard.sg_memb_friend definition

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
  PRIMARY KEY (`MEMB_NO`,`FRIEND_NO`,`SERVICE_TYPE_CD`,`MEMB_QR_NO`,`FRIEND_QR_NO`) USING BTREE,
  KEY `idx_sg_memb_friend_01` (`MEMB_NO`,`SERVICE_TYPE_CD`),
  KEY `idx_sg_memb_friend_02` (`MEMB_NO`,`ROOM_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='친구정보';


-- secretguard.sg_memb_locat definition

CREATE TABLE `sg_memb_locat` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `LOCAT_COL_SEQ` int(11) NOT NULL DEFAULT 1 COMMENT '수집순번',
  `LATITUDE` decimal(20,10) DEFAULT NULL COMMENT '위도',
  `LONGITUDE` decimal(20,10) DEFAULT NULL COMMENT '경도',
  `COL_DT` datetime DEFAULT NULL COMMENT '수집일',
  `LAST_COL_YN` varchar(10) DEFAULT NULL COMMENT '마지막수집데이타여부',
  `LOCAT_NM` varchar(100) DEFAULT NULL COMMENT '위치정보',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`LOCAT_COL_SEQ`),
  KEY `sg_memb_locat_idx01` (`MEMB_NO`,`LAST_COL_YN`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='회원위치정보';


-- secretguard.sg_memb_locat_data definition

CREATE TABLE `sg_memb_locat_data` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `LOCAT_SEQ` varchar(20) NOT NULL COMMENT '순번',
  `LATITUDE` decimal(20,10) DEFAULT NULL COMMENT '위도',
  `LONGITUDE` decimal(20,10) DEFAULT NULL COMMENT '경도',
  `COL_DT` varchar(14) DEFAULT NULL COMMENT '수집일',
  `LOCAT_NM` varchar(100) DEFAULT NULL COMMENT '위치정보',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`LOCAT_SEQ`) USING BTREE,
  KEY `MEMB_NO_COL_DT` (`MEMB_NO`,`COL_DT`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='회원위치정보데이타';


-- secretguard.sg_memb_locat_hist definition

CREATE TABLE `sg_memb_locat_hist` (
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `LOCAT_COL_SEQ` int(11) NOT NULL AUTO_INCREMENT COMMENT '순번',
  `LATITUDE` decimal(20,10) DEFAULT NULL COMMENT '위도',
  `LONGITUDE` decimal(20,10) DEFAULT NULL COMMENT '경도',
  `COL_DT` datetime DEFAULT NULL COMMENT '수집일',
  `LAST_COL_YN` varchar(10) DEFAULT NULL COMMENT '마지막수집데이타여부',
  `LOCAT_NM` varchar(100) DEFAULT NULL COMMENT '위치정보',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`LOCAT_COL_SEQ`)
) ENGINE=InnoDB AUTO_INCREMENT=1888 DEFAULT CHARSET=utf8 COMMENT='회원위치정보이력';


-- secretguard.sg_memb_qr definition

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
  `QR_INFO` varchar(200) DEFAULT NULL COMMENT 'QR이미지',
  `IMG_FILE_PATH` varchar(200) DEFAULT NULL COMMENT '이미지로컬위치',
  `IMG_FILE_URL` varchar(200) DEFAULT NULL COMMENT '이미지URL',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`MEMB_NO`,`QR_NO`),
  UNIQUE KEY `membqr_01_idx` (`SUB_SCR`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='QR사용자정보';


-- secretguard.sg_message definition

CREATE TABLE `sg_message` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `CHAT_SEQ` varchar(20) NOT NULL DEFAULT '1' COMMENT '메세지순번',
  `MSG_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '메세지타입 T, C, F',
  `MSG_DESC` varchar(4000) DEFAULT NULL COMMENT '메세지',
  `FILE_IMG_URL` varchar(100) DEFAULT NULL COMMENT '파일URL',
  `FILE_PATH` varchar(100) DEFAULT NULL COMMENT '파일경로',
  `WRITE_MEMB_NO` varchar(20) DEFAULT NULL COMMENT '작성자',
  `REAL_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '응답SEQ',
  `DELETE_YN` varchar(20) DEFAULT NULL COMMENT '삭제여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`CHAT_SEQ`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='메세지정보';


-- secretguard.sg_num definition

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


-- secretguard.sg_po definition

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
  `DOWNLOAD_ST_DT` datetime DEFAULT NULL COMMENT '최근다운로드 일자',
  `DOWNLOAD_ST_USER_ID` varchar(20) DEFAULT NULL COMMENT '최근다운로드 담당자',
  `DOWNLOAD_ST_FILE_NO` varchar(20) DEFAULT NULL COMMENT '최근다운로드 파일번호',
  `DOWNLOAD_PDF_CNT` decimal(10,2) DEFAULT NULL COMMENT 'PDF다운로드수',
  `DOWNLOAD_PDF_DT` varchar(20) DEFAULT NULL COMMENT 'PDF다운로드일자',
  `DOWNLOAD_PDF_USER_ID` varchar(20) DEFAULT NULL COMMENT 'PDF다운로드담당자',
  `DOWNLOAD_PDF_FILE_NO` varchar(20) DEFAULT NULL COMMENT 'PDF 파일번호',
  `UNIT_PRICE_AMT` decimal(20,6) DEFAULT NULL COMMENT '개별단가',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`DISTRIBUTOR_NO`,`PO_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='거래처 QR발급리스트';


-- secretguard.sg_qr_info definition

CREATE TABLE `sg_qr_info` (
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR파일번호',
  `QR_FILE_NO` varchar(20) NOT NULL COMMENT 'QR파일번호',
  `QR_INFO` varchar(200) DEFAULT NULL COMMENT 'QR 내부 URL 정보',
  `LOW_QR_INFO` varchar(200) DEFAULT NULL COMMENT '로컬저장위치',
  `USE_YN` varchar(2) DEFAULT 'Y' COMMENT 'QR 사용여부',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT 'QR 삭제여부',
  `SERVICE_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '서비스구분',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`QR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='QR생성정보';


-- secretguard.sg_qr_po definition

CREATE TABLE `sg_qr_po` (
  `DISTRIBUTOR_NO` varchar(20) NOT NULL COMMENT '가맹점번호',
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR번호',
  `QR_FILE_NO` varchar(20) DEFAULT NULL COMMENT 'QR파일번호',
  `QR_INFO` varchar(200) DEFAULT NULL COMMENT 'QR 내부 URL 정보',
  `QR_STICKER_NO` varchar(20) DEFAULT NULL COMMENT 'QR 스티커 파일번호',
  `QR_STICKER_INFO` varchar(200) DEFAULT NULL COMMENT 'QR 스티커 정보',
  `APPLY_YN` varchar(2) DEFAULT 'N' COMMENT '적용여부',
  `DEL_YN` varchar(2) DEFAULT 'N' COMMENT '삭제여부',
  `CREATE_DT` varchar(8) DEFAULT 'N' COMMENT '생성일자',
  `APPLY_DT` varchar(8) DEFAULT 'N' COMMENT '적용일자',
  `PO_NO` varchar(20) DEFAULT NULL COMMENT 'PO번호',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`DISTRIBUTOR_NO`,`QR_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='거래처 QR발급리스트';


-- secretguard.sg_room_del_message definition

CREATE TABLE `sg_room_del_message` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `CHAT_SEQ` varchar(20) NOT NULL DEFAULT '' COMMENT '삭제 생성 순번',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`MEMB_NO`,`CHAT_SEQ`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='메세지삭제정보';


-- secretguard.sg_room_file definition

CREATE TABLE `sg_room_file` (
  `ROOM_NO` varchar(20) NOT NULL COMMENT 'ROOM번호',
  `FILE_NO` varchar(20) NOT NULL COMMENT '파일번호',
  `FILE_SEQ` int(11) NOT NULL COMMENT '파일순번',
  `FILE_PATH` varchar(200) DEFAULT NULL COMMENT '파일경로',
  `LOCAL_PATH` varchar(200) DEFAULT NULL COMMENT '다운로드경로',
  `FILE_NM` varchar(128) DEFAULT NULL COMMENT '파일명',
  `FILE_ORG_NM` varchar(128) DEFAULT NULL COMMENT '파일초기명',
  `FILE_SIZE` varchar(20) DEFAULT NULL COMMENT '파일사이즈',
  `FILE_TYPE` varchar(20) DEFAULT NULL COMMENT '파일타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`FILE_NO`,`FILE_SEQ`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='대화방파일관리';


-- secretguard.sg_room_info definition

CREATE TABLE `sg_room_info` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `ROOM_NM` varchar(40) NOT NULL COMMENT '방명',
  `ROOM_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '방타입 Group Sigle',
  `LAST_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '마지막번호',
  `SERVICE_TYPE_CD` varchar(20) DEFAULT NULL COMMENT '서비스타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='방정보';


-- secretguard.sg_room_member definition

CREATE TABLE `sg_room_member` (
  `ROOM_NO` varchar(40) NOT NULL COMMENT '방번호',
  `MEMB_SEQ` varchar(40) NOT NULL COMMENT '방명',
  `MEMB_NO` varchar(20) NOT NULL COMMENT '회원번호',
  `QR_NO` varchar(20) NOT NULL COMMENT 'QR번호',
  `FIRST_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '최초 생성 순번',
  `WRITE_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '마지막작성순번',
  `READ_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '마지막읽음순번',
  `EXIT_CHAT_SEQ` varchar(20) DEFAULT NULL COMMENT '탈퇴마지막순번',
  `FRIEND_JOIN_YN` varchar(2) DEFAULT 'N' COMMENT '친구추가여부',
  `ROOM_NM` varchar(40) DEFAULT 'N' COMMENT '자체ROOM명',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`ROOM_NO`,`MEMB_SEQ`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='방멤버정보';


-- secretguard.sg_seller_info definition

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


-- secretguard.sg_server_info definition

CREATE TABLE `sg_server_info` (
  `SERVER_NO` varchar(20) NOT NULL COMMENT '서버번호',
  `SERVER_NM` varchar(40) DEFAULT NULL COMMENT '서버명',
  `INFO_SERVER` varchar(100) DEFAULT NULL COMMENT '데이타서버',
  `CALL_SERVER` varchar(100) DEFAULT NULL COMMENT '통신서버',
  `CONNECT_CNT` int(11) DEFAULT 0 COMMENT '접속수',
  `USE_YN` char(1) DEFAULT NULL COMMENT '사용여부',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`SERVER_NO`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='서버정보';


-- secretguard.sg_sys_menu definition

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


-- secretguard.sg_tuto definition

CREATE TABLE `sg_tuto` (
  `TUTO_NO` varchar(20) NOT NULL COMMENT '튜토리얼번호',
  `FILE_NO` varchar(20) DEFAULT NULL COMMENT '이미지파일번호',
  `DEL_YN` char(1) DEFAULT NULL COMMENT '삭제여부',
  `VIEW_NUM` int(11) DEFAULT NULL COMMENT '노출순번',
  `DEVI_TP_CD` varchar(10) DEFAULT NULL COMMENT '디바이스타입(  SA, SI )',
  `LANG_CD` varchar(10) DEFAULT NULL COMMENT '언어타입',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  `CHG_DT` datetime DEFAULT NULL COMMENT '수정일',
  `CHG_ID` varchar(20) DEFAULT NULL COMMENT '수정자',
  PRIMARY KEY (`TUTO_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='튜토리얼';


-- secretguard.sg_user_auth definition

CREATE TABLE `sg_user_auth` (
  `USER_ID` varchar(20) NOT NULL COMMENT '사용자ID',
  `AUTH_NO` varchar(20) NOT NULL COMMENT '권한코드',
  `ADD_DT` datetime DEFAULT NULL COMMENT '등록일',
  `ADD_ID` varchar(20) DEFAULT NULL COMMENT '등록자',
  PRIMARY KEY (`USER_ID`,`AUTH_NO`),
  KEY `R_6` (`AUTH_NO`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='사용자권한';


-- secretguard.sg_user_info definition

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


-- secretguard.sg_ver_info definition

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