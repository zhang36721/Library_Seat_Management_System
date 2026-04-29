"""
RFID 卡片相关 Pydantic 模型
"""

from typing import Optional
from datetime import datetime
from pydantic import BaseModel, Field


class CardBase(BaseModel):
    """卡片基础信息"""
    card_id: str = Field(..., min_length=1, max_length=20, description="卡片物理ID")
    user_name: str = Field(..., min_length=1, max_length=50, description="持卡人姓名")
    user_id: str = Field(..., min_length=1, max_length=20, description="学号/工号")


class CardCreate(CardBase):
    """创建卡片"""
    pass


class CardUpdate(BaseModel):
    """更新卡片"""
    user_name: Optional[str] = Field(None, min_length=1, max_length=50)
    user_id: Optional[str] = Field(None, min_length=1, max_length=20)
    status: Optional[str] = Field(None, description="active/inactive/lost")


class CardResponse(CardBase):
    """卡片响应"""
    status: str = Field(default="active", description="active有效 inactive无效 lost挂失")
    created_at: str = Field(default="", description="创建时间")

    class Config:
        from_attributes = True