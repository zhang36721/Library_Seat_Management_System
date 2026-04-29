"""
用户认证相关 Pydantic 模型
"""

from typing import Optional, List
from pydantic import BaseModel, Field


class LoginRequest(BaseModel):
    """登录请求"""
    username: str = Field(..., min_length=1, max_length=50)
    password: str = Field(..., min_length=1, max_length=100)


class LoginResponse(BaseModel):
    """登录响应"""
    access_token: str
    token_type: str = "bearer"
    user: "UserInfo"


class UserInfo(BaseModel):
    """用户信息"""
    id: int
    username: str
    role: str = "user"  # user / admin / superadmin


class TokenPayload(BaseModel):
    """JWT Token 载荷"""
    sub: str  # username
    user_id: int
    role: str
    exp: int


class AdminUserCreate(BaseModel):
    """创建管理员用户"""
    username: str = Field(..., min_length=3, max_length=50)
    password: str = Field(..., min_length=6, max_length=100)
    role: str = Field(default="admin")


class AdminUserResponse(BaseModel):
    """管理员用户响应"""
    id: int
    username: str
    role: str
    created_at: str = ""