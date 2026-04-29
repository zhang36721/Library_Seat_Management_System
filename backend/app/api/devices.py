"""
设备管理 API 路由
"""

from fastapi import APIRouter, Depends

from app.schemas.device import DeviceResponse, HeartbeatRequest
from app.schemas.common import APIResponse
from app.services import DeviceService
from app.core import get_current_user

router = APIRouter()


@router.get("", response_model=APIResponse[list[DeviceResponse]])
def list_devices(user=Depends(get_current_user)):
    """获取所有设备列表（需登录）"""
    devices = DeviceService.list_devices()
    return APIResponse(data=[DeviceResponse(**d) for d in devices])


@router.post("/heartbeat", response_model=APIResponse[DeviceResponse])
def device_heartbeat(req: HeartbeatRequest):
    """设备心跳上报（设备调用，暂无需认证）"""
    device = DeviceService.process_heartbeat(req.device_id, req.status)
    return APIResponse(data=DeviceResponse(**device), message="心跳接收成功")