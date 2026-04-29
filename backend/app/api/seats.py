"""
座位管理 API 路由
"""

from fastapi import APIRouter, Depends, HTTPException, status, Body
from typing import Optional

from app.schemas.seat import SeatCreate, SeatUpdate, SeatResponse
from app.schemas.common import APIResponse
from app.services import SeatService
from app.core import get_current_user, require_admin

router = APIRouter()


@router.get("", response_model=APIResponse[list[SeatResponse]])
def list_seats():
    """获取所有座位列表（公开）"""
    seats = SeatService.list_seats()
    return APIResponse(data=[SeatResponse(**s) for s in seats])


@router.get("/{seat_id}", response_model=APIResponse[SeatResponse])
def get_seat(seat_id: int):
    """获取单个座位信息"""
    seat = SeatService.get_seat(seat_id)
    if not seat:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="座位不存在")
    return APIResponse(data=SeatResponse(**seat))


@router.post("", response_model=APIResponse[SeatResponse], status_code=201)
def create_seat(req: SeatCreate, user=Depends(require_admin)):
    """创建座位（需管理员权限）"""
    new_seat = SeatService.create_seat(req.name, req.node_id, req.description)
    return APIResponse(data=SeatResponse(**new_seat), message="创建成功")


@router.put("/{seat_id}", response_model=APIResponse[SeatResponse])
def update_seat(seat_id: int, data: SeatUpdate, user=Depends(require_admin)):
    """更新座位信息（需管理员权限）"""
    updated = SeatService.update_seat(seat_id, data.model_dump(exclude_unset=True, exclude_none=True))
    if not updated:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="座位不存在")
    return APIResponse(data=SeatResponse(**updated), message="更新成功")


@router.delete("/{seat_id}", response_model=APIResponse)
def delete_seat(seat_id: int, user=Depends(require_admin)):
    """删除座位（需管理员权限）"""
    deleted = SeatService.delete_seat(seat_id)
    if not deleted:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="座位不存在")
    return APIResponse(message="删除成功")