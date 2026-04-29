"""
卡片管理 API 路由
"""

from fastapi import APIRouter, Depends, HTTPException, status, Query

from app.schemas.card import CardCreate, CardUpdate, CardResponse
from app.schemas.common import APIResponse
from app.services import CardService
from app.core import require_admin

router = APIRouter()


@router.get("", response_model=APIResponse[list[CardResponse]])
def list_cards(search: str = Query(default="", description="搜索关键词")):
    """获取所有卡片（支持搜索）"""
    cards = CardService.list_cards(search)
    return APIResponse(data=[CardResponse(**c) for c in cards])


@router.get("/{card_id}", response_model=APIResponse[CardResponse])
def get_card(card_id: str):
    """获取指定卡片信息"""
    card = CardService.get_card(card_id)
    if not card:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="卡片不存在")
    return APIResponse(data=CardResponse(**card))


@router.post("", response_model=APIResponse[CardResponse], status_code=201)
def create_card(req: CardCreate, user=Depends(require_admin)):
    """注册新卡片（需管理员权限）"""
    new_card = CardService.create_card(req.card_id, req.user_name, req.user_id)
    return APIResponse(data=CardResponse(**new_card), message="注册成功")


@router.put("/{card_id}", response_model=APIResponse[CardResponse])
def update_card(card_id: str, data: CardUpdate, user=Depends(require_admin)):
    """更新卡片信息（需管理员权限）"""
    updated = CardService.update_card(card_id, data.model_dump(exclude_unset=True, exclude_none=True))
    if not updated:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="卡片不存在")
    return APIResponse(data=CardResponse(**updated), message="更新成功")


@router.delete("/{card_id}", response_model=APIResponse)
def delete_card(card_id: str, user=Depends(require_admin)):
    """删除卡片（需管理员权限）"""
    deleted = CardService.delete_card(card_id)
    if not deleted:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="卡片不存在")
    return APIResponse(message="删除成功")