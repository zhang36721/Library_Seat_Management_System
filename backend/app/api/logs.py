"""
刷卡记录 API 路由
"""

from fastapi import APIRouter, Query, Depends
from typing import Optional

from app.schemas.card_log import CardLogResponse
from app.schemas.common import APIResponse, PaginatedResponse
from app.services import CardLogService
from app.core import get_current_user

router = APIRouter()


@router.get("", response_model=APIResponse[PaginatedResponse[CardLogResponse]])
def list_logs(
        page: int = Query(default=1, ge=1),
        page_size: int = Query(default=20, ge=1, le=100),
        seat_id: Optional[int] = Query(default=None, description="按座位ID筛选"),
        card_id: Optional[str] = Query(default=None, description="按卡片ID筛选"),
        action: Optional[str] = Query(default=None, description="按动作筛选 check_in/check_out"),
        user=Depends(get_current_user)
):
    """分页查询刷卡记录（需登录）"""
    items, total = CardLogService.list_logs(
        page=page, page_size=page_size,
        seat_id=seat_id, card_id=card_id, action=action
    )
    return APIResponse(data=PaginatedResponse(
        total=total, page=page, page_size=page_size,
        items=[CardLogResponse(**it) for it in items]
    ))