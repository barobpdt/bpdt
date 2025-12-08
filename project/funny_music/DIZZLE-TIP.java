##> db 스키마 생성
[설치]
	npm i drizzle-orm postgres

	npm i -D drizzle-kit

	설정 : drizzle.config.ts
	
	<drizzle.config.ts>
	import { defineConfig } from "drizzle-kit";
	export default defineConfig({
	  schema: ["./src/lib/db/schema.ts", "./src/features/**/db/schema.ts"], // 경로 추가
	  out: "./drizzle/migrations",
	  dialect: "postgresql",
	  dbCredentials: { url: process.env.DATABASE_URL! }
	});
	
	<drizzle.config.js>
	import { ENV } from "./src/config/env.js";
	export default {
	  schema: "./src/db/schema.js",
	  out: "./src/db/migrations",
	  dialect: "postgresql",
	  dbCredentials: { url: ENV.DATABASE_URL },
	};
	
[소스]	
	import {lower} from '@/drizzle/db.utils';
	import { pgTable, serial, text, integer, varchar, timestamp, uniqueIndex } from 'drizzle-orm/pg-core';
	// Define a 'users' table
	export const users = pgTable('users', 
	{
		id: serial('id').primaryKey(), // Auto-incrementing primary key
		name: text('name').notNull(),    // Text field, cannot be null
		email: text('email').unique(),   // Unique text field
	});
	// Define a 'posts' table with a foreign key to 'users'
	export const posts = pgTable('posts', 
	{
		id: serial('id').primaryKey(),
		title: text('title').notNull(),
		authorId: integer('author_id').references(() => users.id), // Foreign key to users.id
	});

	export const emailSubmissionTable = pgTable('email_submission',
	{
		id: serial('id').primaryKey(),
		name: text('name').notNull(),
		email: text('email').notNull().unique(),
		createdAt: timestamp('created_at').notNull().defaultNow(),
	},
	(table) => [uniqueIndex('unique_email_idx').on(lower(table.email))]
	);

	export const countries = pgTable('countries', {
		id: serial('id').primaryKey(),
		name: varchar('name', { length: 256 }),
	});

	export const cities = pgTable('cities', {
		id: serial('id').primaryKey(),
		name: varchar('name', { length: 256 }),
		countryId: integer('country_id').references(() => countries.id),
	});
	
##> 조회
	import * as schema from "@drizzle/schema"
	import { Product, ProductTags, Tag } from "@drizzle/schema"
	import { Inject, Injectable } from "@nestjs/common"
	import { getProductlistSchema } from "@product/common/product-zod.dto"
	import { eq, getTableColumns, sql } from "drizzle-orm"

	await db
	.select()
	.from(countries)
	.leftJoin(cities, eq(cities.countryId, countries.id))
	.where(eq(countries.id, 10))



	async getProductList() {
		const result = await db
		  .select({
			product: getTableColumns(Product),
			tags: sql<string[]>`array_agg(${Tag.name})`,
		  })
		  .from(Product)
		  .leftJoin(ProductTags, eq(ProductTags.productId, Product.id))
		  .leftJoin(Tag, eq(ProductTags.tagId, Tag.id))
		  .groupBy(Product.id)

		const res = getProductlistSchema.parse(result)
		return res
	}

	async addProductData(payload) {
		const [data] = await db
		  .insert(schema.Product)
		  .values({ ...payload })
		  .returning()
		return data
	}
	
	async function getUsersWithPets() {
		// [호출예] getUsersWithPets().then(data => console.log(data));
		const result = await db
			.select({
				userId: users.id,
				userName: users.name,
				petId: pets.id,
				petName: pets.name,
			})
			.from(users)
			.leftJoin(pets, eq(users.id, pets.ownerId));

		return result;
	}
	
##> insert return
	await db.insert(users).values({ name: "Dan" }).returning();
	// partial return
	await db.insert(users).values({ name: "Partial Dan" }).returning({ insertedId: users.id });

##> 데이터베이스 관계설정
import { pgTable, serial, text, integer } from 'drizzle-orm/pg-core';
import { relations } from 'drizzle-orm';

export const users = pgTable('users', {
	id: serial('id').primaryKey(),
	name: text('name').notNull(),
});

export const posts = pgTable('posts', {
	id: serial('id').primaryKey(),
	title: text('title').notNull(),
	content: text('content'),
	authorId: integer('author_id').references(() => users.id).notNull(),
});

export const userRelations = relations(users, ({ many }) => ({
	posts: many(posts),
}));

export const postRelations = relations(posts, ({ one }) => ({
	author: one(users, {
		fields: [posts.authorId],
		references: [users.id],
	}),
}));

import { db } from './db'; // Assuming you have your Drizzle instance initialized
async function createNewUserWithPost(
	userName: string, 
	postTitle: string, 
	postContent?: string
) {
  await db.transaction(async (tx) => {
    // Insert the user first
    const [newUser] = await tx.insert(users).values({ name: userName }).returning();
    if (newUser) {
      // Then insert the post, referencing the newly created user's ID
      await tx.insert(posts).values({
        title: postTitle,
        content: postContent,
        authorId: newUser.id,
      });
    }
  });
}

// Example usage
createNewUserWithPost('Alice', 'My First Post', 'This is the content of my first post.');