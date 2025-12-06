import { sqliteTable, text, integer } from 'drizzle-orm/sqlite-core';
import { sql } from 'drizzle-orm';

// Messages 테이블 (채팅 메시지 저장)
export const messages = sqliteTable('messages', {
    id: integer('id').primaryKey({ autoIncrement: true }),
    username: text('username').notNull(),
    message: text('message').notNull(),
    room: text('room').default('general'),
    createdAt: text('created_at').default(sql`CURRENT_TIMESTAMP`)
});
