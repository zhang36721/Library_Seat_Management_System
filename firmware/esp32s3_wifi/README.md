# ESP32S3 WiFi 上传端固件

## 概述

本工程为 ESP32S3 WiFi 上传端固件。

## 功能

- 接收主控 STM32 通过 UART 发送的数据
- 连接 WiFi 网络
- 通过 HTTP POST 上传到 Python FastAPI 后端

## 职责范围

ESP32S3 不负责座位判断和刷卡逻辑，只负责网络上传。