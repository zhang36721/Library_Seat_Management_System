"""
系统配置 API 路由
"""

from fastapi import APIRouter, Depends

from app.schemas.system import SystemConfigUpdate, SystemConfigResponse
from app.schemas.common import APIResponse
from app.services import SystemConfigService
from app.core import require_admin

router = APIRouter()


@router.get("/config", response_model=APIResponse[SystemConfigResponse])
def get_config():
    """获取系统配置（公开）"""
    config = SystemConfigService.get_config()
    return APIResponse(data=SystemConfigResponse(**config))


@router.put("/config", response_model=APIResponse[SystemConfigResponse])
def update_config(data: SystemConfigUpdate, user=Depends(require_admin)):
    """更新系统配置（需管理员权限）"""
    updated = SystemConfigService.update_config(data.model_dump(exclude_none=True))
    return APIResponse(data=SystemConfigResponse(**updated), message="配置更新成功")