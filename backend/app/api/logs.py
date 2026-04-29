"""
刷卡记录 API 路由（公开）
"""

from fastapi import APIRouter, Query

from app.schemas.card_log import CardLogResponse
from app.schemas.common import APIResponse

router = APIRouter()


@router.get("", response_model=APIResponse[list[CardLogResponse]])
def list_logs(
        limit: int = Query(default=100, ge=1, le=500, description="返回条数")
):
    """获取最近刷卡记录（公开）"""
    items = _CardLogService.list_recent(limit)
    return APIResponse(data=[CardLogResponse(**it) for it in items])


# ---------- simple in‑module service (avoids importing removed dashboard endpoints) ----------

import json, os

class _CardLogService:
    DATA_FILE = os.path.join(os.path.dirname(__file__), "..", "..", "runtime_data", "card_logs.json")

    @classmethod
    def _ensure_file(cls):
        os.makedirs(os.path.dirname(cls.DATA_FILE), exist_ok=True)
        if not os.path.exists(cls.DATA_FILE):
            with open(cls.DATA_FILE, "w", encoding="utf-8") as f:
                json.dump([], f)

    @classmethod
    def _read(cls):
        cls._ensure_file()
        with open(cls.DATA_FILE, "r", encoding="utf-8") as f:
            try:
                return json.load(f)
            except json.JSONDecodeError:
                return []

    @classmethod
    def list_recent(cls, limit: int = 100):
        all_logs = cls._read()
        # sort by recorded_at descending
        all_logs.sort(key=lambda x: x.get("recorded_at", ""), reverse=True)
        return all_logs[:limit]