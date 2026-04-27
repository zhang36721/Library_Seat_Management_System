# 实用脚本

本目录存放开发和维护过程中使用的实用脚本。

## 脚本列表（待实现）

### 数据库相关
- `init_db.py` - 数据库初始化脚本
- `migrate_data.py` - 数据迁移脚本
- `backup_db.sh` - 数据库备份脚本
- `restore_db.sh` - 数据库恢复脚本

### 设备相关
- `flash_stm32.sh` - STM32 固件烧录脚本
- `flash_esp32.sh` - ESP32S3 固件烧录脚本
- `scan_devices.py` - 设备扫描脚本

### 测试相关
- `test_api.py` - API 接口测试脚本
- `simulate_data.py` - 模拟数据生成脚本
- `load_test.py` - 压力测试脚本

### 部署相关
- `deploy.sh` - 一键部署脚本
- `update.sh` - 更新脚本
- `clean.sh` - 清理脚本

## 使用说明

1. 执行脚本前请确保有执行权限
2. Python 脚本需要安装相应依赖
3. Shell 脚本需要 bash 环境
4. 部分脚本需要管理员权限