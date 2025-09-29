from sqlalchemy.ext.asyncio import create_async_engine, async_sessionmaker, AsyncSession

engine = create_async_engine(url, echo=True)
session = async_sessionmaker(engine)

# 객체 생성
async with engine.begin() as conn:
       await conn.run_sync(Base.metadata.create_all)

# 데이터 삽입
async with session() as db:
    db.add(...)
    await db.commit()

# 데이터 쿼리
async with session() as db:
    stmt = select(A)
    row = await db.execute(stmt)
    for obj in row.scalars():
        print(obj.id)

await engine.dispose()

## 조건부 필터사용
query = select(User)
if username is not None:
    query = query.where(User.username == username)
if password is not None:
    query = query.where(User.password == password)
    
## JSON 매핑

data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))
   
id: Mapped[Integer] = mapped_column(primary_key=True, autoincrement=True)

## 필드연산사용
class ProductCatalog(IdBase):
"""Defines the details of the product catalog table in our SQL database."""
    __tablename__ = "product_catalog"
    id: Mapped[Integer] = mapped_column(primary_key=True, autoincrement=True)
    name: Mapped[String]
    amount_count: MappedSQLExpression[int] = column_property(
        select(func.count(AmountCatalog.id))
        .where(AmountCatalog.product_catalog_id == id)
        .correlate_except(AmountCatalog)
        .scalar_subquery()
    )


## 모델예
from datetime import UTC, datetime

from sqlalchemy import (
    DECIMAL,
    BigInteger,
    Boolean,
    DateTime,
    Enum,
    Float,
    Integer,
    String,
    text
)
from sqlalchemy.dialects.postgresql import JSONB, REAL, UUID
from sqlalchemy.orm import Mapped, mapped_column
from .base import Base

class ModelType(StrEnum):
    PLAIN = "PLAIN"
class DemoModel(Base):
    __tablename__ = "demo_model"

    id: Mapped[uuid.UUID] = mapped_column(
        UUID(as_uuid=True),
        primary_key=True,
        default_factory=uuid.uuid4,
        server_default=text("gen_random_uuid()"),
        init=False,
    )

    col_int: Mapped[int] = mapped_column(Integer)
    col_float: Mapped[float] = mapped_column(REAL(precision=4, asdecimal=False))
    col_decimal: Mapped[Decimal] = mapped_column(DECIMAL(precision=8))
    col_bigint: Mapped[int] = mapped_column(BigInteger)
    col_str: Mapped[str] = mapped_column(String(255))
    col_bool: Mapped[bool] = mapped_column(Boolean)
    col_datetime: Mapped[datetime.datetime] = mapped_column(DateTime(timezone=True))
    col_updated_at: Mapped[datetime.datetime] = mapped_column(DateTime(timezone=True), onupdate=datetime.now(UTC))
    col_json: Mapped[dict] = mapped_column(JSONB(none_as_null=True))
    col_enum: Mapped[ModelType] = mapped_column(
        Enum(ModelType),
        default=ModelType.PLAIN,
        server_default=text(f"'{ModelType.PLAIN.value}'"),
    )

result = await session.execute(
    select(DemoModel).where(DemoModel.id == demo_model.id)
    # also you can select particular fields
    # select(DemoModel.id, DemoModel.col_decimal).where(DemoModel.id == demo_model.id)
)

list_of_rows = result.mappings().fetchall()
assert isinstance(list_of_rows[0], Mapping)
assert "DemoModel" in list_of_rows[0] 
    