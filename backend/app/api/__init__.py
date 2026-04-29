"""
API 路由 - 统一注册
"""

from fastapi import APIRouter

from app.api.auth import router as auth_router
from app.api.seats import router as seats_router
from app.api.cards import router as cards_router
from app.api.logs import router as logs_router
from app.api.devices import router as devices_router
from app.api.system import router as system_router

router = APIRouter()

router.include_router(auth_router, prefix="/auth", tags=["认证"])
router.include_router(seats_router, prefix="/seats", tags=["座位"])
router.include_router(cards_router, prefix="/cards", tags=["卡片"])
router.include_router(logs_router, prefix="/logs", tags=["刷卡记录"])
router.include_router(devices_router, prefix="/devices", tags=["设备"])
router.include_router(system_router, prefix="/system", tags=["系统"])
