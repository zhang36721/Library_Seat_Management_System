"""
认证相关 API 路由
"""

from fastapi import APIRouter, Depends, HTTPException, status

from app.schemas.user import LoginRequest, LoginResponse, UserInfo, AdminUserCreate, AdminUserResponse
from app.schemas.common import APIResponse
from app.services import AuthService
from app.core import create_access_token, get_current_user, require_admin, require_superadmin

router = APIRouter()


@router.post("/login", response_model=APIResponse[LoginResponse])
def login(req: LoginRequest):
    """用户登录，返回 JWT Token"""
    user = AuthService.login(req.username, req.password)
    if not user:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="用户名或密码错误")

    token = create_access_token({
        "sub": user["username"],
        "user_id": user["id"],
        "role": user["role"]
    })

    return APIResponse(data=LoginResponse(
        access_token=token,
        token_type="bearer",
        user=UserInfo(**user)
    ))


@router.get("/me", response_model=APIResponse[UserInfo])
def get_me(user=Depends(get_current_user)):
    """获取当前登录用户信息"""
    return APIResponse(data=UserInfo(**user))


@router.get("/users", response_model=APIResponse[list[AdminUserResponse]])
def list_users(user=Depends(require_admin)):
    """获取所有管理员用户列表（需管理员权限）"""
    users = AuthService.list_users()
    return APIResponse(data=[AdminUserResponse(**u) for u in users])


@router.post("/users", response_model=APIResponse[AdminUserResponse])
def create_user(req: AdminUserCreate, user=Depends(require_superadmin)):
    """创建管理员用户（需超级管理员权限）"""
    try:
        new_user = AuthService.create_user(req.username, req.password, req.role)
        return APIResponse(data=AdminUserResponse(**new_user), message="创建成功")
    except ValueError as e:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=str(e))