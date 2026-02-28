## 사용자 설정
create user master with password 'pass1234';
grant connect on database funny_music to master;
grant all privileges on database funny_music to master;

CREATE DATABASE funny_music
    WITH
    OWNER = postgres
    ENCODING = 'UTF8'
    LOCALE_PROVIDER = 'libc'
    CONNECTION LIMIT = -1
    IS_TEMPLATE = False;

##> 연결설정
// config/db.js
import * as schema from "../db/schema.js";
import { ENV } from "./env.js";
let db;
if (ENV.DB_DRIVER === "local") {
  const { drizzle } = await import("drizzle-orm/postgres-js");
  const postgres = (await import("postgres")).default;
  db = drizzle(postgres(ENV.DATABASE_URL), { schema });
} else {
  const { drizzle } = await import("drizzle-orm/neon-http");
  const { neon } = await import("@neondatabase/serverless");
  db = drizzle(neon(ENV.DATABASE_URL), { schema });
}
export { db };


##> git push
# 추적 목록에서만 제거 (실제 파일은 유지)
git rm -r --cached node_modules
git commit -m "chore: remove node_modules from tracking"
git push


