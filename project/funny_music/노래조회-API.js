✅ 추천 조합
무료로 빠르게 시작 → LRCLIB or Lyrics.ovh

GET https://lrclib.net/api/search?q=아이유&search_fields=artist_name
GET https://api.lyrics.ovh/v1/{아티스트}/{곡명}

✅ 가사제공 사이트 목록
서비스	무료 여부	특징
Musixmatch	무료(30% 제한)	Spotify·Apple Music 연동. 가장 방대한 DB. 한국곡 지원
Genius API	무료	가사 직접 반환 아닌 가사 페이지 URL 반환. 파싱 필요
LRCLIB	완전 무료	싱크 가사(.lrc) 지원. 약 300만 곡 DB. 오픈소스
Lyrics.ovh	완전 무료	아티스트+곡명으로 가사 반환. 매우 단순한 API

http://www.maniadb.com/api/

✅ 초성검색

import { getChoseong } from "es-hangul";
getChoseong("블루밍")       // → "ㅂㄹㅁ"
getChoseong("아이유")       // → "ㅇㅇㅇ"
getChoseong("IU Blueming") // → "IU Blueming" (영문은 그대로)

// schema.js
export const songsTable = pgTable("songs", {
  ...
  artistChosung: text("artist_chosung"), // ← 추가
  titleChosung:  text("title_chosung"),  // ← 추가
});



// server.js - insert 시
import { getChoseong } from "es-hangul";

db.insert(songsTable).values({
  artist, title, lyrics,
  artistChosung: getChoseong(artist), // ← 자동 계산
  titleChosung:  getChoseong(title),
})

