"""
座位相关 Pydantic 模型
"""

from typing import Optional
from pydantic import BaseModel, Field


class SeatStatusUpload(BaseModel):
    """ESP32S3 上传的座位状态"""
    device_id: int = Field(..., ge=1, description="设备编号")
    time: int = Field(..., description="Unix 时间戳")
    seat1: int = Field(..., ge=0, le=2, description="座位1状态")
    seat2: int = Field(..., ge=0, le=2, description="座位2状态")
    seat3: int = Field(..., ge=0, le=2, description="座位3状态")
    free_seats: int = Field(..., ge=0, description="空闲座位数")
    card_id: str = Field(default="", description="当前刷卡ID")
    action: int = Field(default=0, ge=0, le=2, description="动作类型")
    gate_status: int = Field(default=0, ge=0, le=1, description="闸机状态")


class SeatBase(BaseModel):
    """座位基础信息"""
    name: str = Field(..., min_length=1, max_length=50, description="座位名称")
    node_id: int = Field(..., ge=1, description="设备节点ID")
    description: str = Field(default="", max_length=200, description="座位描述")


class SeatCreate(SeatBase):
    """创建座位"""
    pass


class SeatUpdate(BaseModel):
    """更新座位"""
    name: Optional[str] = Field(None, min_length=1, max_length=50)
    node_id: Optional[int] = Field(None, ge=1)
    description: Optional[str] = Field(None, max_length=200)


class SeatResponse(SeatBase):
    """座位响应"""
    id: int
    status: int = Field(default=0, ge=0, le=2, description="座位状态 0空闲 1占用 2异常")
    current_user: Optional[str] = Field(default=None, description="当前使用者")
    last_update: Optional[str] = Field(default=None, description="最后更新时间")

    class Config:
        from_attributes = True