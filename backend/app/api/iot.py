from datetime import datetime, timezone
from typing import Any

from fastapi import APIRouter

router = APIRouter()

DEVICE_OFFLINE_TIMEOUT_MS = 10000
MAX_CARD_EVENTS = 50

device_status: dict[str, Any] = {
    "device_id": "kento-main-001",
    "stm32_online": False,
    "wifi_connected": False,
    "ssid": "",
    "rssi": 0,
    "card_count": 0,
    "log_count": 0,
    "ds1302_valid": False,
    "current_time": "",
    "device_status": {
        "seats": [
            {"id": 1, "state": "UNKNOWN"},
            {"id": 2, "state": "UNKNOWN"},
            {"id": 3, "state": "UNKNOWN"},
        ],
        "gate": {"state": "UNKNOWN"},
    },
    "last_heartbeat_at": None,
    "last_heartbeat_at_ms": 0,
    "last_stm32_frame_at_ms": 0,
    "esp_uptime_ms": 0,
}
card_events: list[dict[str, Any]] = []


def now_ms() -> int:
    return int(datetime.now(timezone.utc).timestamp() * 1000)


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def esp32_online() -> bool:
    last_ms = int(device_status.get("last_heartbeat_at_ms") or 0)
    return last_ms > 0 and now_ms() - last_ms < DEVICE_OFFLINE_TIMEOUT_MS


def stm32_online() -> bool:
    last_ms = int(device_status.get("last_stm32_frame_at_ms") or 0)
    if bool(device_status.get("stm32_online", False)):
        return True
    return last_ms > 0 and now_ms() - last_ms < DEVICE_OFFLINE_TIMEOUT_MS


def current_seats() -> list[dict[str, Any]]:
    return device_status.get("device_status", {}).get("seats", [])


def current_gate() -> dict[str, Any]:
    return device_status.get("device_status", {}).get("gate", {"state": "UNKNOWN"})


def sync_event_count() -> None:
    device_status["log_count"] = max(int(device_status.get("log_count") or 0), len(card_events))


@router.post("/heartbeat")
async def post_heartbeat(payload: dict[str, Any]):
    device_status.update(
        {
            "device_id": payload.get("device_id", device_status["device_id"]),
            "esp_uptime_ms": payload.get("esp_uptime_ms", 0),
            "stm32_online": bool(payload.get("stm32_online", False)),
            "wifi_connected": bool(payload.get("wifi_connected", False)),
            "ssid": payload.get("ssid", ""),
            "rssi": payload.get("rssi", 0),
            "last_heartbeat_at": now_iso(),
            "last_heartbeat_at_ms": now_ms(),
        }
    )
    return {"ok": True}


@router.post("/device-status")
async def post_device_status(payload: dict[str, Any]):
    frame_ms = now_ms()
    device_status.update(
        {
            "device_id": payload.get("device_id", device_status["device_id"]),
            "stm32_online": bool(payload.get("stm32_online", True)),
            "last_stm32_frame_at_ms": frame_ms,
            "card_count": payload.get("card_count", device_status["card_count"]),
            "log_count": payload.get("log_count", device_status["log_count"]),
            "ds1302_valid": bool(payload.get("ds1302_valid", device_status["ds1302_valid"])),
            "current_time": payload.get("current_time", device_status["current_time"]),
            "device_status": {
                "seats": payload.get("seats", device_status["device_status"]["seats"]),
                "gate": payload.get("gate", device_status["device_status"]["gate"]),
            },
        }
    )
    return {"ok": True}


@router.post("/card-event")
async def post_card_event(payload: dict[str, Any]):
    device_status["stm32_online"] = True
    device_status["last_stm32_frame_at_ms"] = now_ms()
    event = {
        "device_id": payload.get("device_id", device_status["device_id"]),
        "uid": payload.get("uid", ""),
        "type": payload.get("type", "DENIED"),
        "allowed": bool(payload.get("allowed", False)),
        "time": payload.get("time", ""),
        "received_ms": payload.get("received_ms", 0),
        "server_received_at": now_iso(),
    }
    card_events.append(event)
    if len(card_events) > MAX_CARD_EVENTS:
        del card_events[:-MAX_CARD_EVENTS]
    sync_event_count()
    return {"ok": True}


@router.get("/status")
async def get_status():
    stm32_is_online = stm32_online()
    return {
        **device_status,
        "stm32_online": stm32_is_online,
        "esp_online": esp32_online(),
        "esp32_online": esp32_online(),
        "seats": current_seats(),
        "gate": current_gate(),
        "esp_offline_timeout_ms": DEVICE_OFFLINE_TIMEOUT_MS,
        "server_time": now_iso(),
        "server_now_ms": now_ms(),
    }


@router.get("/card-events")
async def get_card_events():
    return {"count": len(card_events), "events": card_events}


@router.post("/clear-events")
async def clear_events():
    card_events.clear()
    return {"ok": True}
