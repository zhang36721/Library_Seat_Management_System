"""
设备相关 Pydantic 模型
"""

from typing import Optional
from pydantic import BaseModel, Field


class DeviceStatusUpload(BaseModel):
    """ESP32S3 上传的设备状态"""
    device_id: int = Field(..., ge=1)
    time: int = Field(..., description="Unix 时间戳")
    seat1: int = Field(..., ge=0, le=2)
    seat2: int = Field(..., ge=0, le=2)
    seat3: int = Field(..., ge=0, le=2)
    free_seats: int = Field(..., ge=0)
    card_id: str = Field(default="")
    action: int = Field(default=0, ge=0, le=2)
    gate_status: int = Field(default=0, ge=0, le=1)


class HeartbeatRequest(BaseModel):
    """设备心跳请求"""
    device_id: int = Field(..., ge=1)
    timestamp: int = Field(..., description="Unix 时间戳")
    status: str = Field(default="online", description="online/offline/error")


class DeviceResponse(BaseModel):
    """设备响应"""
    device_id: int
    name: str
    type: str
    status: str = "online"
    last_heartbeat: Optional[str] = None
    firmware_version: str = "1.0.0"
    ip_address: str = ""

    class Config:
        from_attributes = True