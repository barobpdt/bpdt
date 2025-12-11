##> config { }
	vars(
		@[vars]
	)
	depoly(

	)
	install(
		npm install @neondatabase/serverless dotenv
	) 
	importAll(
		express from "express";
		dotenv from "dotenv";
		asyncHandler from "express-async-handler"
		{ getAuth, clerkClient } from "@clerk/express"
		{ protectRoute } from "../middleware/auth.middleware.js"
	)
##> env
text(
	PORT = 8090
	DATABASE_URL = <user_db_url>
	NODE_ENV = developer
)

##> table {}
	transactions (
		id serial pk // comment
		user_id text(255) notnull 
		title
		amount int(10,2) notnull
		category text(255)
		create_dt date def(current_date)
	)

##> sql { name = user }
	userProfile(
		select * 
		from 
			user 
		where 1=1
			@[userId] ? and username=@[0]
	)
	findUser(
		select count(1) as cnt from user where clerkId=@[0] ${@[]?''}
	)
	createUser(
		insert into user (
			@fields(user)
		) value(
			@bind(user)
		)
	)

##> routes { name = user }
/* 사용자 정보 */
[get] 
profile/:username { asyncHandler } 
	sql( userProfile(username) => user ) not(user) error('${username} 사용자를 찾을수 없습니다')
	res(user)

/* 사용자 동기화 */
[post] 
sync { asyncHandeler, protectRoute } 
	auth(userId)
	set( bindData, {})
	sql( findUser(:bindData) => user) if(user) error("사용자가 이미 존재합니다")
	js(
		const clerkUser = await clerkClient.users.getUser(authId);
		const userData = {
			clerkId: userId,
			email: clerkUser.emailAddresses[0].emailAddress,
			firstName: clerkUser.firstName || "",
			lastName: clerkUser.lastName || "",
			username: clerkUser.emailAddresses[0].emailAddress.split("@")[0],
			profilePicture: clerkUser.imageUrl || "",
		};
		const data = @[tableInfo(user)]
	)
	sql(createUser(:userData) => user )
	res({user, message:'${userData.username} 사용자가 등록되었습니다'})

/* 현재로그인 사용자 */
[get] 
me { protectRoute, getCurrentUser}
	auth(userId)
	sql(findUser(userId) => user ) not(user) error(${username} 사용자를 찾을수 없습니다)
	res(user)
[put] 
profile {ayncHandler, protectRoute)
	auth(userId)
	sql(findUser(userId) => user ) not(user) error(${username} 사용자를 찾을수 없습니다)
[post]
follow/:targetUserId { protectRoute }
	auth(userId)
	


##> routes { name:pos }

