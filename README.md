# 图书馆座位管理系统

基于 ZigBee 的图书馆座位管理系统，实现座位状态检测、数据上传和Web管理功能。

## 项目简介

本系统通过 ZigBee 无线网络连接多个座位检测节点，实时监测座位占用状态，并通过 ESP32S3 将数据上传至后端服务器，提供 Web 管理界面供管理员和用户使用。

## 功能模块

- **座位检测节点**：基于 STM32 + ZigBee，检测座位是否被占用
- **主控节点**：基于 STM32，收集所有座位节点数据
- **WiFi 网关**：基于 ESP32S3，将数据上传至服务器
- **后端服务**：基于 Python FastAPI，处理数据和提供 API
- **前端界面**：基于 Vue3，提供座位管理界面

## 技术栈

### 硬件
- STM32F103C8T6（座位节点/主控节点）
- ZigBee 模块（CC2530）
- ESP32S3（WiFi 通信）
- 红外/压力传感器（座位检测）

### 软件
- **前端**：Vue 3 + Vite + Element Plus
- **后端**：Python + FastAPI + SQLAlchemy
- **数据库**：MySQL/PostgreSQL
- **部署**：Nginx + Gunicorn + Systemd

## 目录说明

| 目录 | 说明 |
|------|------|
| `docs/` | 项目文档 |
| `hardware/` | 硬件设计资料 |
| `firmware/` | 嵌入式固件代码 |
| `backend/` | Python 后端代码 |
| `frontend/` | Vue3 前端代码 |
| `tools/` | 开发工具脚本 |
| `deployment/` | 服务器部署配置 |