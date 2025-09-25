from pydantic import BaseModel
from typing import Optional

#> Post
class User(BaseModel):
    userId: str
    name: str
    email: str