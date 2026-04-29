"""
数据模型 - JSON 文件操作层
"""

import json
import os
import shutil
import threading
from pathlib import Path
from datetime import datetime, timezone, timedelta
from typing import Optional, List, Dict, Any

# ============================================================
# 路径配置: runtime_data 存放实际数据，data_templates 存放模板
# ============================================================
BASE_DIR = Path(__file__).resolve().parent.parent.parent
TEMPLATES_DIR = BASE_DIR / "data_templates"
RUNTIME_DIR = BASE_DIR / "runtime_data"
LOCK = threading.Lock()

TZ = timezone(timedelta(hours=8))  # Asia/Shanghai UTC+8


def _now_iso() -> str:
    """返回当前 ISO 时间字符串（东八区）"""
    return datetime.now(TZ).isoformat(timespec="seconds")


def _ensure_runtime():
    """确保 runtime_data 目录存在，不存在则从 templates 拷贝"""
    RUNTIME_DIR.mkdir(parents=True, exist_ok=True)
    for fname in ["seats.json", "cards.json", "card_logs.json",
                  "device_status.json", "system_config.json", "admin_users.json"]:
        target = RUNTIME_DIR / fname
        if not target.exists():
            tpl = TEMPLATES_DIR / (fname.replace(".json", ".example.json"))
            if tpl.exists():
                shutil.copy(tpl, target)
            else:
                target.write_text("[]" if fname not in ("system_config.json",) else "{}", encoding="utf-8")


def _read_json(filename: str) -> Any:
    """读取运行时 JSON 文件"""
    _ensure_runtime()
    path = RUNTIME_DIR / filename
    with LOCK:
        if not path.exists():
            return {} if filename == "system_config.json" else []
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)


def _write_json(filename: str, data: Any):
    """写入运行时 JSON 文件"""
    _ensure_runtime()
    path = RUNTIME_DIR / filename
    with LOCK:
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)


# ============================================================
# Seat 模型操作
# ============================================================
class SeatModel:
    @staticmethod
    def get_all() -> List[Dict]:
        raw = _read_json("seats.json")
        if isinstance(raw, dict):
            return raw.get("seats", [])
        return raw

    @staticmethod
    def get_by_id(seat_id: int) -> Optional[Dict]:
        seats = SeatModel.get_all()
        for s in seats:
            if s.get("id") == seat_id:
                return s
        return None

    @staticmethod
    def save_all(seats: List[Dict]):
        _write_json("seats.json", {"seats": seats, "updated_at": _now_iso()})

    @staticmethod
    def update_status(seat_id: int, status: int, current_user: Optional[str] = None):
        seats = SeatModel.get_all()
        for s in seats:
            if s["id"] == seat_id:
                s["status"] = status
                if current_user is not None:
                    s["current_user"] = current_user
                s["last_update"] = _now_iso()
                break
        else:
            return None
        SeatModel.save_all(seats)
        return seats

    @staticmethod
    def create(seat_data: Dict) -> Dict:
        seats = SeatModel.get_all()
        new_id = max([s["id"] for s in seats], default=0) + 1
        new_seat = {
            "id": new_id,
            "name": seat_data["name"],
            "node_id": seat_data["node_id"],
            "description": seat_data.get("description", ""),
            "status": 0,
            "current_user": None,
            "last_update": _now_iso()
        }
        seats.append(new_seat)
        SeatModel.save_all(seats)
        return new_seat

    @staticmethod
    def update(seat_id: int, data: Dict) -> Optional[Dict]:
        seats = SeatModel.get_all()
        for s in seats:
            if s["id"] == seat_id:
                for k, v in data.items():
                    if v is not None:
                        s[k] = v
                s["last_update"] = _now_iso()
                SeatModel.save_all(seats)
                return s
        return None

    @staticmethod
    def delete(seat_id: int) -> bool:
        seats = SeatModel.get_all()
        new_seats = [s for s in seats if s["id"] != seat_id]
        if len(new_seats) == len(seats):
            return False
        SeatModel.save_all(new_seats)
        return True


# ============================================================
# Card 模型操作
# ============================================================
class CardModel:
    @staticmethod
    def get_all() -> List[Dict]:
        raw = _read_json("cards.json")
        if isinstance(raw, dict):
            return raw.get("cards", [])
        return raw

    @staticmethod
    def get_by_card_id(card_id: str) -> Optional[Dict]:
        cards = CardModel.get_all()
        for c in cards:
            if c.get("card_id") == card_id:
                return c
        return None

    @staticmethod
    def save_all(cards: List[Dict]):
        _write_json("cards.json", {"cards": cards, "updated_at": _now_iso()})

    @staticmethod
    def create(data: Dict) -> Dict:
        cards = CardModel.get_all()
        new_card = {
            "card_id": data["card_id"],
            "user_name": data["user_name"],
            "user_id": data.get("user_id", ""),
            "status": "active",
            "created_at": _now_iso()
        }
        # 去重：已存在则返回已有
        existing = CardModel.get_by_card_id(data["card_id"])
        if existing:
            return existing
        cards.append(new_card)
        CardModel.save_all(cards)
        return new_card

    @staticmethod
    def update(card_id: str, data: Dict) -> Optional[Dict]:
        cards = CardModel.get_all()
        for c in cards:
            if c["card_id"] == card_id:
                for k, v in data.items():
                    if v is not None:
                        c[k] = v
                CardModel.save_all(cards)
                return c
        return None

    @staticmethod
    def delete(card_id: str) -> bool:
        cards = CardModel.get_all()
        new_cards = [c for c in cards if c["card_id"] != card_id]
        if len(new_cards) == len(cards):
            return False
        CardModel.save_all(new_cards)
        return True


# ============================================================
# CardLog 模型操作
# ============================================================
class CardLogModel:
    @staticmethod
    def get_all(limit: int = 100, offset: int = 0) -> List[Dict]:
        raw = _read_json("card_logs.json")
        if isinstance(raw, dict):
            logs = raw.get("logs", [])
        else:
            logs = raw
        return logs[::-1][offset:offset + limit]

    @staticmethod
    def add(data: Dict) -> Dict:
        raw = _read_json("card_logs.json")
        if isinstance(raw, dict):
            logs = raw.get("logs", [])
        else:
            logs = raw
        new_id = max([l["id"] for l in logs], default=0) + 1
        entry = {
            "id": new_id,
            "card_id": data["card_id"],
            "user_name": data.get("user_name", ""),
            "action": data["action"],
            "seat_id": data["seat_id"],
            "result": data.get("result", "success"),
            "timestamp": _now_iso()
        }
        logs.append(entry)
        _write_json("card_logs.json", {"logs": logs, "updated_at": _now_iso()})
        return entry

    @staticmethod
    def count() -> int:
        raw = _read_json("card_logs.json")
        if isinstance(raw, dict):
            return len(raw.get("logs", []))
        return len(raw)

    @staticmethod
    def get_by_seat(seat_id: int, limit: int = 20) -> List[Dict]:
        raw = _read_json("card_logs.json")
        if isinstance(raw, dict):
            logs = raw.get("logs", [])
        else:
            logs = raw
        filtered = [l for l in logs if l.get("seat_id") == seat_id]
        return filtered[::-1][:limit]

    @staticmethod
    def get_by_card(card_id: str, limit: int = 20) -> List[Dict]:
        raw = _read_json("card_logs.json")
        if isinstance(raw, dict):
            logs = raw.get("logs", [])
        else:
            logs = raw
        filtered = [l for l in logs if l.get("card_id") == card_id]
        return filtered[::-1][:limit]


# ============================================================
# DeviceStatus 模型操作
# ============================================================
class DeviceModel:
    @staticmethod
    def get_all() -> List[Dict]:
        raw = _read_json("device_status.json")
        if isinstance(raw, dict):
            return raw.get("devices", [])
        return raw

    @staticmethod
    def update_heartbeat(device_id: int, status: str = "online") -> Dict:
        raw = _read_json("device_status.json")
        if isinstance(raw, dict):
            devices = raw.get("devices", [])
        else:
            devices = raw
        for d in devices:
            if d["device_id"] == device_id:
                d["status"] = status
                d["last_heartbeat"] = _now_iso()
                _write_json("device_status.json", {"devices": devices})
                return d
        # 不存在则创建
        new_dev = {
            "device_id": device_id,
            "name": f"Device-{device_id}",
            "type": "esp32s3_gateway",
            "status": status,
            "last_heartbeat": _now_iso(),
            "firmware_version": "1.0.0",
            "ip_address": ""
        }
        devices.append(new_dev)
        _write_json("device_status.json", {"devices": devices})
        return new_dev


# ============================================================
# SystemConfig 模型操作
# ============================================================
class SystemConfigModel:
    @staticmethod
    def get() -> Dict:
        raw = _read_json("system_config.json")
        if not raw or not raw.get("config"):
            # 返回默认配置
            return {
                "config": {
                    "system_name": "图书馆座位管理系统",
                    "total_seats": 3,
                    "check_in_timeout": 15,
                    "auto_release_time": 30,
                    "gate_open_duration": 3,
                    "maintenance_mode": False,
                    "log_retention_days": 30,
                    "time_settings": {
                        "timezone": "Asia/Shanghai",
                        "opening_time": "08:00",
                        "closing_time": "22:00"
                    },
                    "notification_settings": {
                        "email_enabled": False,
                        "sms_enabled": False
                    }
                },
                "version": "1.0"
            }
        return raw

    @staticmethod
    def update(data: Dict) -> Dict:
        current = SystemConfigModel.get()
        config = current.get("config", {})

        # 深层合并
        if "system_name" in data and data["system_name"] is not None:
            config["system_name"] = data["system_name"]
        if "total_seats" in data and data["total_seats"] is not None:
            config["total_seats"] = data["total_seats"]
        if "check_in_timeout" in data and data["check_in_timeout"] is not None:
            config["check_in_timeout"] = data["check_in_timeout"]
        if "auto_release_time" in data and data["auto_release_time"] is not None:
            config["auto_release_time"] = data["auto_release_time"]
        if "gate_open_duration" in data and data["gate_open_duration"] is not None:
            config["gate_open_duration"] = data["gate_open_duration"]
        if "maintenance_mode" in data and data["maintenance_mode"] is not None:
            config["maintenance_mode"] = data["maintenance_mode"]
        if "log_retention_days" in data and data["log_retention_days"] is not None:
            config["log_retention_days"] = data["log_retention_days"]
        if data.get("time_settings"):
            config["time_settings"] = data["time_settings"]
        if data.get("notification_settings"):
            config["notification_settings"] = data["notification_settings"]

        current["config"] = config
        current["version"] = "1.0"
        _write_json("system_config.json", current)
        return current


# ============================================================
# AdminUsers 模型操作
# ============================================================
class AdminUserModel:
    @staticmethod
    def get_all() -> List[Dict]:
        _ensure_runtime()
        path = RUNTIME_DIR / "admin_users.json"
        with LOCK:
            try:
                with open(path, "r", encoding="utf-8") as f:
                    users = json.load(f)
            except (FileNotFoundError, json.JSONDecodeError):
                users = []
            if not users:
                import hashlib
                users = [{
                    "id": 1,
                    "username": "admin",
                    "password": hashlib.sha256("admin123".encode()).hexdigest(),
                    "role": "superadmin",
                    "created_at": _now_iso()
                }]
                with open(path, "w", encoding="utf-8") as f:
                    json.dump(users, f, ensure_ascii=False, indent=2)
            return users

    @staticmethod
    def save_all(users: List[Dict]):
        path = RUNTIME_DIR / "admin_users.json"
        with LOCK:
            with open(path, "w", encoding="utf-8") as f:
                json.dump(users, f, ensure_ascii=False, indent=2)

    @staticmethod
    def get_by_username(username: str) -> Optional[Dict]:
        for u in AdminUserModel.get_all():
            if u["username"] == username:
                return u
        return None

    @staticmethod
    def create(username: str, password: str, role: str = "admin") -> Dict:
        import hashlib
        users = AdminUserModel.get_all()
        new_id = max([u["id"] for u in users], default=0) + 1
        new_user = {
            "id": new_id,
            "username": username,
            "password": hashlib.sha256(password.encode()).hexdigest(),
            "role": role,
            "created_at": _now_iso()
        }
        users.append(new_user)
        AdminUserModel.save_all(users)
        return new_user

    @staticmethod
    def verify(username: str, password: str) -> Optional[Dict]:
        import hashlib
        user = AdminUserModel.get_by_username(username)
        if not user:
            return None
        if user["password"] == hashlib.sha256(password.encode()).hexdigest():
            return user
        return None