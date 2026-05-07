"""
应用配置
"""

from pydantic import field_validator
from pydantic_settings import BaseSettings
from typing import Any, List


class Settings(BaseSettings):
    """应用配置类"""
    
    # 基本信息
    PROJECT_NAME: str = "图书馆座位管理系统"
    API_V1_PREFIX: str = "/api/v1"
    VERSION: str = "1.0.0"
    
    # CORS 配置
    CORS_ORIGINS: List[str] = ["*"]
    CORS_ALLOW_CREDENTIALS: bool = False
    
    # 服务器配置
    HOST: str = "0.0.0.0"
    PORT: int = 18080
    DEBUG: bool = False

    @field_validator("DEBUG", "CORS_ALLOW_CREDENTIALS", mode="before")
    @classmethod
    def parse_bool(cls, value: Any) -> bool:
        if isinstance(value, str):
            return value.lower() in {"1", "true", "yes", "on", "debug", "dev"}
        return bool(value)

    @field_validator("CORS_ORIGINS", mode="before")
    @classmethod
    def parse_cors_origins(cls, value: Any) -> List[str]:
        if isinstance(value, str):
            items = [item.strip() for item in value.split(",") if item.strip()]
            return items or ["*"]
        return value
    
    # 数据库配置
    DATABASE_URL: str = "mysql+pymysql://root:password@localhost:3306/library_seats"
    # DATABASE_URL: str = "postgresql://user:password@localhost:5432/library_seats"
    
    # JWT 配置
    SECRET_KEY: str = "your-secret-key-change-in-production"
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE_MINUTES: int = 60 * 24  # 24 小时
    
    class Config:
        env_file = ".env"


# 全局配置实例
settings = Settings()
