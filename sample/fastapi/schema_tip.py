''' 
https://gnidinger.tistory.com/category/Python/FastAPI

https://soogoonsoogoonpythonists.github.io/sqlalchemy-for-pythonist/tutorial/

데이터 타입

Integer: 정수형 데이터
String: 문자열 데이터 (길이 지정 가능)
Text: 긴 문자열 데이터
Float: 실수형 데이터
Boolean: 참/거짓 값을 가질 수 있는 논리형 데이터
DateTime: 날짜와 시간을 함께 저장
Date: 날짜만 저장
Time: 시간만 저장
JSON: JSON 형태의 데이터
Enum: 열거형 데이터 타입
관계 설정

ForeignKey: 외래 키 설정
relationship: ORM 레벨에서의 객체 관계 설정
back_populates 또는 backref: 양방향 관계 설정
Column 설정

primary_key: 기본 키 설정
index: 인덱스 설정
unique: 고유 값 설정
nullable: NULL 허용 설정
default: 기본값 설정
'''

class FeedCreate(BaseModel):
    title: str
    content: str
    image_urls: Optional[List[str]] = []


author = await db.execute(select(User).where(User.email == author_email))
author  = author.scalar_one_or_none()
        = author.one()
        = author.first()
        = author.all()

feeds = await db.execute(
    select(Feed, User.nickname).join(User, User.email == Feed.author_email)
).all()

feed_responses = []
for feed, nickname in feeds:
    feed_dict = {
        "id": feed.id,
        "title": feed.title,
        "content": feed.content,
        "author_email": feed.author_email,
        "author_nickname": nickname,
        "image_urls": feed.image_urls,
    }
    feed_responses.append(feed_dict)

return feed_responses


from sqlalchemy import Column, Integer, ForeignKey, UniqueConstraint
from config.db import Base


class Follow(Base):
    __tablename__ = "follows"

    follower_id = Column(Integer, ForeignKey("users.id"), primary_key=True)
    following_id = Column(Integer, ForeignKey("users.id"), primary_key=True)

    # 두 필드의 조합이 고유해야 함
    __table_args__ = (UniqueConstraint("follower_id", "following_id", name="unique_follow"),)


class UserLogin(BaseModel):
    email: str
    password: str

    @validator("email")
    def validate_email(cls, value):
        if not re.match(r"[^@]+@[^@]+\.[^@]+", value):
            raise ValueError("Invalid Email Format")
        return value


async def toggle_follow(db: AsyncSession, follower_id: int, following_id: int):
    if follower_id == following_id:
        raise HTTPException(status_code=400, detail="You cannot follow yourself")

    following = await auth_service.get_user_by_id(db, following_id)

    if not following:
        raise HTTPException(status_code=404, detail="User not found")

    result = await db.execute(
        select(Follow).where(Follow.follower_id == follower_id, Follow.following_id == following_id)
    )
    existing_follow = result.scalar_one_or_none()

    if existing_follow:
        await db.delete(existing_follow)
        action = "unfollowed"
    else:
        new_follow = Follow(follower_id=follower_id, following_id=following_id)
        db.add(new_follow)
        action = "followed"

    await db.commit()
    return {"action": action}


from fastapi import APIRouter
router = APIRouter()

@router.patch("/{following_id}")
async def toggle_follow_route(
    following_id: int,
    email: str = Depends(auth_service.get_current_user_authorization),
    db: AsyncSession = Depends(get_db),
):
    # 이메일을 이용해 사용자 정보를 가져옴
    result = await db.execute(select(User).filter_by(email=email))
    current_user = result.scalar_one_or_none()

    # 사용자 정보가 없으면 에러 처리
    if current_user is None:
        raise HTTPException(status_code=404, detail="User not found")

    current_user_id = current_user.id

    return await toggle_follow(db, current_user_id, following_id)



async def toggle_like(
    db: AsyncSession, user_email: str, feed_id: int = None, comment_id: int = None
):
    # feed_id와 comment_id 둘 다 없거나 둘 다 있을 경우 에러
    if (feed_id is None and comment_id is None) or (feed_id is not None and comment_id is not None):
        raise HTTPException(
            status_code=400, detail="Either feed_id or comment_id must be provided."
        )

    # 이미 좋아요가 있는지 찾기
    query = select(Like).where(
        (Like.user_email == user_email)
        & (Like.feed_id == feed_id)
        & (Like.comment_id == comment_id)
    )
    result = await db.execute(query)
    existing_like = result.scalar_one_or_none()

    # 좋아요가 이미 있다면 삭제, 없다면 추가
    if existing_like:
        await db.delete(existing_like)
    else:
        new_like = Like(user_email=user_email, feed_id=feed_id, comment_id=comment_id)
        db.add(new_like)

    await db.commit()


## sub query join
subq = select(Address).where(~Address.email_address.like('%@aol.com')).subquery()
address_subq = aliased(Address, subq)
stmt = select(User, address_subq).join_from(User, address_subq).order_by(User.id, address_subq.id)
with Session(engine) as session:
     for user, address in session.execute(stmt):
         print(f"{user} {address}")

""" 위 구문은 아래 쿼리를 표현합니다.
SELECT user_account.id, user_account.name, user_account.fullname,
anon_1.id AS id_1, anon_1.email_address, anon_1.user_id
FROM user_account JOIN
(SELECT address.id AS id, address.email_address AS email_address, address.user_id AS user_id
FROM address
WHERE address.email_address NOT LIKE ?) AS anon_1 ON user_account.id = anon_1.user_id
ORDER BY user_account.id, anon_1.id
[] ('%@aol.com',)
"""

User(id=1, name='spongebob', fullname='Spongebob Squarepants') Address(id=1, email_address='spongebob@sqlalchemy.org')
User(id=2, name='sandy', fullname='Sandy Cheeks') Address(id=2, email_address='sandy@sqlalchemy.org')
User(id=2, name='sandy', fullname='Sandy Cheeks') Address(id=3, email_address='sandy@squirrelpower.org')