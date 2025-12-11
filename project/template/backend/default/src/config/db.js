@[usePool] ? import { Pool } from '@neondatabase/serverless' 
else
@[useNeon] ? import { neon } from "@neondatabase/serverless"
@[eq(mode,drizzle)] ? import { drizzle } from "drizzle-orm/neon-http";
@[useMongo] ? import mongoose from "mongoose"
import { ENV } from "./env.js"

@[useMongo] ? <>
export const connectDB = async () => {
  try {
    await mongoose.connect(ENV.MONGO_URI);
    console.log("Connected to DB SUCCESSFULLY ✅");
  } catch (error) {
    console.log("Error connecting to MONGODB");
    process.exit(1);
  }
};
</>

@[usePool] ? <>
export const pool = new Pool({
	connectionString: ENV.DATABASE_URL
});
</> else @[useNeon] ? <>
	export const sql = neon(ENV.DATABASE_URL)
	@[eq(mode,drizzle)] ? <>
		export const db = drizzle(sql, { schema }) 
	</> else <>
		export async function initDB() {
		  try {
			@[createTables]
			console.log("Database initialized successfully");
		  } catch (error) {
			console.log("Error initializing DB", error);
			process.exit(1); // status code 1 means failure, 0 success
		  }
		}
	</>
</>
