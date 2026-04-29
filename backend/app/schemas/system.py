"""
系统配置 Pydantic 模型
"""

from typing import Optional, List
from pydantic import BaseModel, Field


class TimeSettings(BaseModel):
    """时间设置"""
    timezone: str = "Asia/Shanghai"
    opening_time: str = "08:00"
    closing_time: str = "22:00"


class NotificationSettings(BaseModel):
    """通知设置"""
    email_enabled: bool = False
    sms_enabled: bool = False


class SystemConfigBase(BaseModel):
    """系统配置基础"""
    system_name: str = "图书馆座位管理系统"
    total_seats: int = Field(default=3, ge=1)
    check_in_timeout: int = Field(default=15, ge=1, description="签到超时(分钟)")
    auto_release_time: int = Field(default=30, ge=1, description="自动释放时间(分钟)")
    gate_open_duration: int = Field(default=3, ge=1, description="闸机打开时长(秒)")
    maintenance_mode: bool = False
    log_retention_days: int = Field(default=30, ge=1)
    time_settings: TimeSettings = TimeSettings()
    notification_settings: NotificationSettings = NotificationSettings()


class SystemConfigUpdate(BaseModel):
    """更新系统配置（所有字段可选）"""
    system_name: Optional[str] = None
    total_seats: Optional[int] = Field(None, ge=1)
    check_in_timeout: Optional[int] = Field(None, ge=1)
    auto_release_time: Optional[int] = Field(None, ge=1)
    gate_open_duration: Optional[int] = Field(None, ge=1)
    maintenance_mode: Optional[bool] = None
    log_retention_days: Optional[int] = Field(None, ge=1)
    time_settings: Optional[TimeSettings] = None
    notification_settings: Optional[NotificationSettings] = None


class SystemConfigResponse(BaseModel):
    """系统配置响应"""
    config: SystemConfigBase
    version: str = "1.0"

    class Config:
        from_attributes = True