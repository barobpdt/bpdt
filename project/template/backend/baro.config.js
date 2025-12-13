
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
[get] profile/:username { asyncHandler } 
	sql( userProfile(username) => user ) not(user) error('${username} 사용자를 찾을수 없습니다')
	res(user)

/* 사용자 동기화 */
[post] sync { asyncHandeler, protectRoute } 
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
[post] register { next }
	body( name, email, password) not(name, email, password) error('')
	query( findUser => result  ) exisit(result) error('')
	set( await bcrypt.hash(password,10) => hashPassword )
	query( newUser = addUser : {name, email password:hashPassword} ) not(newUser) error()
	set( token = jwt.sign({ id: user.id }, ENV.JWT_SECRET_KEY, { expiresIn: process.env.JWT_EXPIRES_IN} )
	send(200, 
		cookie { token, { expire: addTime(ENV.COOKIE_EXPIRES_IN) ), httpOnly: true }
		json { sucess:true, newUser, message:'사용자가 등록되었습니다', useer:newUser, token}
	)

[put] profile {ayncHandler, protectRoute)
	auth(userId)
	sql(findUser(userId) => user ) not(user) error(${username} 사용자를 찾을수 없습니다)

[post] follow/:targetUserId { protectRoute }
	auth(userId)
	

 