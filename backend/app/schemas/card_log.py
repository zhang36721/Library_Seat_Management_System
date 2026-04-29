"""
刷卡记录 Pydantic 模型
"""

from typing import Optional
from pydantic import BaseModel, Field


class CardLogBase(BaseModel):
    """刷卡记录基础信息"""
    card_id: str = Field(..., min_length=1, max_length=20)
    user_name: str = Field(default="", max_length=50)
    action: str = Field(..., description="check_in 签到 check_out 签退")
    seat_id: int = Field(..., ge=1)
    result: str = Field(default="success", description="success成功 failed失败 rejected拒绝")


class CardLogCreate(CardLogBase):
    """创建刷卡记录"""
    pass


class CardLogResponse(CardLogBase):
    """刷卡记录响应"""
    id: int
    timestamp: str = Field(default="", description="刷卡时间 ISO格式")

    class Config:
        from_attributes = True