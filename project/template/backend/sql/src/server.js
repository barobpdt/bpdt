import express from "express";
import { ENV } from "./config/env.js";
import { initDB } from "./config/db.js";
@[useCors] ?
import { initCors } from "./config/cors.js";
@[useJob] ?
import { initJob } from "./config/cron.js";
@[useClerk] ? 
import { clerkMiddleware } from "@clerk/express";
@[useArcjet] ?
import { arcjetMiddleware } from "./middleware/arcjet.middleware.js";
@[useRateLimiter] ?
import rateLimiter from "./middleware/rateLimiter.js";

@[importRoutes]

// middleware
const app = express();
app.use(express.json());

@[useClerk] ? 
app.use(clerkMiddleware());
@[useArcjet] ? 
app.use(arcjetMiddleware);
@[useRateLimiter] ?
app.use(rateLimiter);

app.use(express.json());

@[appUseRoutes] 
// app.use("/api/transactions", transactionsRoute);

@[appRoutes]

const initServer = async () => {
	try {
		@[useCors] ? initCors()
		@[useJob] ? initJob()
		await initDb()
	} catch (err) {
		console.error('서비시작 오류 : ', err)
		process.exit(1)
	}
}

initServer().then(() => {
	app.listen(ENV.PORT, () => {
		console.log("Server is up and running on PORT:", ENV.PORT);
	});
});
