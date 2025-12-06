import { sqliteTable, text, integer } from 'drizzle-orm/sqlite-core';
import { sql } from 'drizzle-orm';

// Posts 테이블
export const posts = sqliteTable('posts', {
    id: integer('id').primaryKey({ autoIncrement: true }),
    title: text('title').notNull(),
    content: text('content').notNull(),
    author: text('author').notNull(),
    published: integer('published', { mode: 'boolean' }).default(false),
    createdAt: text('created_at').default(sql`CURRENT_TIMESTAMP`),
    updatedAt: text('updated_at').default(sql`CURRENT_TIMESTAMP`)
});

// Comments 테이블
export const comments = sqliteTable('comments', {
    id: integer('id').primaryKey({ autoIncrement: true }),
    postId: integer('post_id').notNull().references(() => posts.id, { onDelete: 'cascade' }),
    author: text('author').notNull(),
    content: text('content').notNull(),
    createdAt: text('created_at').default(sql`CURRENT_TIMESTAMP`)
});

// Categories 테이블
export const categories = sqliteTable('categories', {
    id: integer('id').primaryKey({ autoIncrement: true }),
    name: text('name').notNull().unique(),
    description: text('description'),
    createdAt: text('created_at').default(sql`CURRENT_TIMESTAMP`)
});
