"""
业务逻辑服务层
"""

from typing import Optional, Dict, List, Tuple
from datetime import datetime, timezone, timedelta

from app.models import SeatModel, CardModel, CardLogModel, DeviceModel, SystemConfigModel, AdminUserModel, _now_iso

TZ = timezone(timedelta(hours=8))


class SeatService:
    """座位管理业务逻辑"""
    STATUS_MAP = {0: "空闲", 1: "占用", 2: "疑似占用"}

    @staticmethod
    def list_seats() -> List[Dict]:
        seats = SeatModel.get_all()
        return seats

    @staticmethod
    def get_seat(seat_id: int) -> Optional[Dict]:
        return SeatModel.get_by_id(seat_id)

    @staticmethod
    def create_seat(name: str, node_id: int, description: str = "") -> Dict:
        return SeatModel.create({
            "name": name,
            "node_id": node_id,
            "description": description
        })

    @staticmethod
    def update_seat(seat_id: int, data: Dict) -> Optional[Dict]:
        return SeatModel.update(seat_id, data)

    @staticmethod
    def delete_seat(seat_id: int) -> bool:
        return SeatModel.delete(seat_id)

    @staticmethod
    def process_device_upload(upload: Dict) -> Dict:
        """处理 ESP32S3 上传的座位状态数据"""
        seat_mapping = {1: upload.get("seat1"), 2: upload.get("seat2"), 3: upload.get("seat3")}
        for seat_id, status_val in seat_mapping.items():
            if status_val is not None:
                SeatModel.update_status(seat_id, status_val)

        # 处理刷卡事件
        card_id = upload.get("card_id", "")
        action = upload.get("action", 0)  # 0=无 1=签到 2=签退

        card_event = None
        if card_id and action > 0:
            action_map = {1: "check_in", 2: "check_out"}
            action_str = action_map.get(action, "check_in")

            # 查找卡片归属用户
            card_info = CardModel.get_by_card_id(card_id)
            user_name = card_info.get("user_name", "未知用户") if card_info else "未知用户"

            # 确定操作哪个座位（ESP32S上传的数据中 action 对应哪个座位由刷卡位置决定）
            # 简化处理：action 指向 seat1 座位号
            target_seat_id = 1

            # 更新座位状态和当前使用者
            if action_str == "check_in":
                SeatModel.update_status(target_seat_id, 1, user_name)  # 占用
                result = "success"
            elif action_str == "check_out":
                SeatModel.update_status(target_seat_id, 0, None)  # 释放
                result = "success"
            else:
                result = "failed"
                action_str = "check_in"

            card_event = CardLogModel.add({
                "card_id": card_id,
                "user_name": user_name,
                "action": action_str,
                "seat_id": target_seat_id,
                "result": result
            })

        # 更新设备心跳
        device_id = upload.get("device_id", 1)
        DeviceModel.update_heartbeat(device_id, "online")

        return {
            "received": True,
            "seats_updated": len(seat_mapping),
            "card_event": card_event,
            "timestamp": _now_iso()
        }


class CardService:
    """卡片管理业务逻辑"""

    @staticmethod
    def list_cards(search: str = "") -> List[Dict]:
        cards = CardModel.get_all()
        if search:
            search_lower = search.lower()
            cards = [c for c in cards if
                     search_lower in c.get("card_id", "").lower() or
                     search_lower in c.get("user_name", "").lower() or
                     search_lower in c.get("user_id", "").lower()]
        return cards

    @staticmethod
    def get_card(card_id: str) -> Optional[Dict]:
        return CardModel.get_by_card_id(card_id)

    @staticmethod
    def create_card(card_id: str, user_name: str, user_id: str) -> Dict:
        return CardModel.create({
            "card_id": card_id,
            "user_name": user_name,
            "user_id": user_id
        })

    @staticmethod
    def update_card(card_id: str, data: Dict) -> Optional[Dict]:
        return CardModel.update(card_id, data)

    @staticmethod
    def delete_card(card_id: str) -> bool:
        return CardModel.delete(card_id)


class CardLogService:
    """刷卡记录业务逻辑"""

    @staticmethod
    def list_logs(page: int = 1, page_size: int = 20,
                  seat_id: Optional[int] = None,
                  card_id: Optional[str] = None,
                  action: Optional[str] = None) -> Tuple[List[Dict], int]:
        """分页查询刷卡记录，支持筛选"""
        offset = (page - 1) * page_size

        if seat_id:
            all_logs = CardLogModel.get_by_seat(seat_id, 10000)
        elif card_id:
            all_logs = CardLogModel.get_by_card(card_id, 10000)
        else:
            all_logs = CardLogModel.get_all(10000)

        # 按 action 筛选
        if action:
            all_logs = [l for l in all_logs if l.get("action") == action]

        total = len(all_logs)
        items = all_logs[offset:offset + page_size]
        return items, total


class DeviceService:
    """设备管理业务逻辑"""

    @staticmethod
    def list_devices() -> List[Dict]:
        return DeviceModel.get_all()

    @staticmethod
    def process_heartbeat(device_id: int, status: str = "online") -> Dict:
        return DeviceModel.update_heartbeat(device_id, status)


class SystemConfigService:
    """系统配置业务逻辑"""

    @staticmethod
    def get_config() -> Dict:
        return SystemConfigModel.get()

    @staticmethod
    def update_config(data: Dict) -> Dict:
        # 将 Schema 转为 dict（排除 None 值）
        clean_data = {k: v for k, v in data.items() if v is not None}
        return SystemConfigModel.update(clean_data)


class AuthService:
    """认证业务逻辑"""

    @staticmethod
    def login(username: str, password: str) -> Optional[Dict]:
        user = AdminUserModel.verify(username, password)
        if not user:
            return None
        return {
            "id": user["id"],
            "username": user["username"],
            "role": user["role"]
        }

    @staticmethod
    def list_users() -> List[Dict]:
        users = AdminUserModel.get_all()
        # 不返回密码
        return [{"id": u["id"], "username": u["username"],
                 "role": u["role"], "created_at": u.get("created_at", "")} for u in users]

    @staticmethod
    def create_user(username: str, password: str, role: str = "admin") -> Dict:
        existing = AdminUserModel.get_by_username(username)
        if existing:
            raise ValueError(f"用户 {username} 已存在")
        new_user = AdminUserModel.create(username, password, role)
        return {"id": new_user["id"], "username": new_user["username"],
                "role": new_user["role"], "created_at": new_user["created_at"]}