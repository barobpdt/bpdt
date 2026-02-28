import { neon } from '@neondatabase/serverless';

// Neon Console > Connect 에서 복사한 연결 문자열을 여기에 넣으세요
const DATABASE_URL = 'postgresql://mytest_owner:Bvbh6LgkJV8R@ep-fancy-bonus-a8xl2bvb-pooler.eastus2.azure.neon.tech/mytest?sslmode=require&channel_binding=require';

const sql = neon(DATABASE_URL);

// posts 테이블에서 데이터 조회
const data = await sql`
    SELECT * FROM posts
    WHERE 1=1
    ORDER BY created_at DESC
`;

console.log(data);