db=Baro.db('tj')
db.open('C:\project\funny_music\DB\tj_info.db')
root=db.fetchAll('select title, summary from lyrics_song_info limit 100 offset 100')
object('web.req').removeAll(true)
~~
data=_node('test')
while(c,root, n) {
	c.title.split('(+)').inject(artist, title)
	a=c.summary.ref()
	a.findPos('>')
	lyrics=a.value()
	not(artist) continue;
	sub=object('web.req').addNode()
	print(artist, title,lyrics)
	data.with(artist, title,lyrics)
	sub.type='save'
	sub.header='Content-Type:application/json'
	sub.url='http://localhost:8081/api/songs/save'
	sub.method='POST'
	sub.data=json(data)
	addApiJob('add_song',sub, @callback.songs)
}

~~


PostgreSQL에서 유사도 검색을 구현하는 방법은 크게 3가지가 있습니다.

방법 비교
pg_trgm	Full-Text Search	pgvector
난이도	⭐ 쉬움	⭐⭐	⭐⭐⭐
검색 방식	문자 패턴 유사도	단어 역 인덱스	의미 기반 임베딩
한국어 지원	✅ 잘 됨	⚠️ 사전 설정 필요	✅ 잘 됨
외부 의존성	없음	없음	임베딩 API 필요
결과 품질	패턴 일치 기반	단어 빈도 기반	의미적 유사도
방법 1 — pg_trgm (추천 ⭐ — 가장 쉽고 빠름)
PostgreSQL 내장 확장. similarity() 함수로 유사도 점수(0~1)를 계산해 정렬.

sql
-- 확장 활성화 (최초 1회)
CREATE EXTENSION IF NOT EXISTS pg_trgm;
-- 유사도 검색
SELECT id, artist, title,
       similarity(lyrics, '사랑했지만') AS score
FROM songs
WHERE lyrics % '사랑했지만'       -- 유사도 임계값 이상만 (기본 0.3)
ORDER BY score DESC
LIMIT 20;
서버 코드:

javascript
// 최초 1회 확장 활성화
await sql`CREATE EXTENSION IF NOT EXISTS pg_trgm`;
// 검색 API
app.get("/api/songs/similar", catchAsyncErrors(async (req, res) => {
    const { q, threshold = 0.1 } = req.query;
    const songs = await sql`
        SELECT id, artist, title,
               similarity(lyrics, ${q}) AS score
        FROM songs
        WHERE lyrics % ${q}
           OR similarity(lyrics, ${q}) > ${threshold}
        ORDER BY score DESC
        LIMIT 20
    `;
    res.json(songs);
}));
방법 2 — Full-Text Search (tsvector)
단어 단위 검색, 빈도 기반 랭킹.

sql
SELECT id, artist, title,
       ts_rank(to_tsvector('simple', lyrics), plainto_tsquery('simple', '사랑 기다림')) AS rank
FROM songs
WHERE to_tsvector('simple', lyrics) @@ plainto_tsquery('simple', '사랑 기다림')
ORDER BY rank DESC;
방법 3 — pgvector (진짜 의미 검색)
OpenAI/로컬 임베딩 API로 벡터 생성 후 코사인 유사도 검색.

bash
# PostgreSQL pgvector 확장 설치 필요
CREATE EXTENSION IF NOT EXISTS vector;
javascript
// 1. 가사 저장 시 임베딩 생성
const embedding = await openai.embeddings.create({
    model: "text-embedding-3-small",
    input: lyrics,
});
// 2. DB에 벡터 저장 (별도 컬럼 필요)
await sql`UPDATE songs SET embedding = ${JSON.stringify(embedding.data[0].embedding)}::vector WHERE id = ${id}`;
// 3. 검색
const queryEmbed = await openai.embeddings.create({ model: "text-embedding-3-small", input: q });
const results = await sql`
    SELECT id, artist, title,
           1 - (embedding <=> ${JSON.stringify(queryEmbed.data[0].embedding)}::vector) AS similarity
    FROM songs
    ORDER BY embedding <=> ${...}::vector
    LIMIT 20
`;
현재 프로젝트에는 pg_trgm 방식을 추천드립니다. 설치나 외부 API 없이 SQL 확장 하나로 바로 사용할 수 있고, 한국어 가사 유사도 검색에도 충분히 효과적입니다.