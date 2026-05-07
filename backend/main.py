"""
图书馆座位管理系统 - 后端服务入口
"""

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

try:
    from app.api import router as api_router
    from app.api.iot import router as iot_router
    from app.core.config import settings
except ModuleNotFoundError:
    from backend.app.api import router as api_router
    from backend.app.api.iot import router as iot_router
    from backend.app.core.config import settings


def create_app() -> FastAPI:
    """创建 FastAPI 应用实例"""
    
    app = FastAPI(
        title=settings.PROJECT_NAME,
        description="基于 ZigBee 的图书馆座位管理系统后端服务",
        version="1.0.0",
        docs_url="/docs",
        redoc_url="/redoc",
        openapi_url="/openapi.json"
    )
    
    # 配置 CORS
    app.add_middleware(
        CORSMiddleware,
        allow_origins=settings.CORS_ORIGINS,
        allow_credentials=settings.CORS_ALLOW_CREDENTIALS,
        allow_methods=["*"],
        allow_headers=["*"],
    )
    
    # 注册路由
    app.include_router(api_router, prefix=settings.API_V1_PREFIX)
    app.include_router(iot_router, prefix="/api/iot", tags=["iot"])
    
    @app.get("/")
    def root():
        """根路径"""
        return {
            "message": "图书馆座位管理系统 API",
            "docs": "/docs",
            "redoc": "/redoc"
        }
    
    @app.get("/health")
    def health_check():
        """健康检查"""
        return {"status": "ok"}
    
    return app


# 创建应用实例
app = create_app()


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "backend.main:app",
        host=settings.HOST,
        port=settings.PORT,
        reload=settings.DEBUG
    )
