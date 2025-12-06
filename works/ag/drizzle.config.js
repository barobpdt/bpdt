import { defineConfig } from 'drizzle-kit';

export default defineConfig({
    schema: './db/schema.js',
    out: './drizzle',
    driver: 'better-sqlite',
    dbCredentials: {
        url: 'sqlite.db'
    },
    verbose: true,
    strict: true
});
