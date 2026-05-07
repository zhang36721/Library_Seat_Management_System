# API Routes Package

from fastapi import APIRouter

# 导入各个路由模块（待实现）
# from . import auth, users, seats, reservations, devices, statistics

# 主路由
router = APIRouter()

# 注册子路由（待实现）
# router.include_router(auth.router, prefix="/auth", tags=["认证"])
# router.include_router(users.router, prefix="/users", tags=["用户"])
# router.include_router(seats.router, prefix="/seats", tags=["座位"])
# router.include_router(reservations.router, prefix="/reservations", tags=["预约"])
# router.include_router(devices.router, prefix="/devices", tags=["设备"])
# router.include_router(statistics.router, prefix="/statistics", tags=["统计"])

# 设备数据上传接口（ESP32S3 调用）
@router.post("/seat/status/batch")
async def batch_upload_seat_status():
    """批量上传座位状态（ESP32S3 调用）"""
    pass

@router.post("/device/heartbeat")
async def device_heartbeat():
    """设备心跳"""
    pass