import express from "express";
import cors from "cors";
import { ENV } from "./config/env.js";
import { connectDB } from "./config/db.js";
@[useClerk] ? import { clerkMiddleware } from "@clerk/express";
@[useCookie] ? import cookieParser from "cookie-parser";
@[useArcjet] ? import { arcjetMiddleware } from "./middleware/arcjet.middleware.js";

@[importRoutes]

const app = express();

@[useCors] ? <>
	@[useCorsOption] ? 
	<js>
	const corsOptions = {
		origin: @[allowedOrigins] ? 
			function (origin, callback) {
				// 개발 환경에서는 모든 출처 허용
				if (process.env.NODE_ENV === 'development') {
					callback(null, true);
					return;
				}
				// 프로덕션 환경에서는 특정 출처만 허용
				const allowedOrigins = [
					@[allowedOrigins]
				];
				// origin이 undefined인 경우는 같은 출처 요청 (Postman, cURL 등)
				if (!origin || allowedOrigins.includes(origin)) {
					callback(null, true);
				} else {
					callback(new Error(`출처 ${origin}는 CORS 정책에 의해 차단되었습니다.`));
				}
			} else 
			@[origin] ||
			function (origin, callback) {
				// 모든 출처 허용
				callback(null, true)
			}
		// 허용할 HTTP 메서드
		methods:  [
			@[methods] || <>
			'GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'
			</>
		],

		// 허용할 요청 헤더
		allowedHeaders: [
			@[allowedHeaders] || <>
			'Content-Type',
			'Authorization',
			'X-API-Key',
			'X-Request-ID'
			</>
		],

		// 클라이언트가 접근 가능한 응답 헤더
		exposedHeaders: [
			@[exposedHeaders] || <>
			'X-Total-Count',
			'X-Page-Number',
			'X-Rate-Limit-Remaining'
			</>
		],

		// 인증 정보 포함 허용 (쿠키, Authorization 헤더 등)
		credentials: @[credentials] || true,

		// Preflight 요청 캐시 시간 (24시간)
		maxAge: @[maxAge] || 86400,

		// OPTIONS 요청 성공 상태 코드
		optionsSuccessStatus: 204
	};
	app.use(cors(corsOptions));
	</js> else <js>
	app.use(cors());
	</js>
</>

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

@[useUpload] ? <>
app.use(
  fileUpload({
	tempFileDir: "@[uploadPath||./uploads]",
	useTempFiles: true,
  })
);
</>

@[useError] ? app.use(errorMiddleware);
@[useCookie] ? app.use(cookieParser());
@[useClerk] ? app.use(clerkMiddleware());
@[useArcjet] ? app.use(arcjetMiddleware);

@[appRoutes]

@[appRoutes] || app.get("/", (req, res) => res.send("Hello from server"));

@[useError] ? <>
// error handling middleware
app.use((err, req, res, next) => {
  console.error("Unhandled error:", err);
  res.status(500).json({ error: err.message || "Internal server error" });
});
</>

const startServer = async () => {
	try {
		@[useCors] ? initCors()
		@[useJob] ? initJob()
		await initDb()
		if (ENV.NODE_ENV !== "production") {
			app.listen(ENV.PORT, () => console.log("Server is up and running on PORT:", ENV.PORT));
		}
	} catch (err) {
		console.error('서비시작 오류 : ', err)
		process.exit(1)
	}
};

startServer();

// export for vercel
export default app;
