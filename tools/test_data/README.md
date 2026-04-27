# 测试数据

本目录用于存放测试用的模拟数据文件。

## 数据文件（待添加）

### 座位状态数据
- `seat_status_sample.json` - 座位状态模拟数据
- `seat_status_sample.csv` - 座位状态 CSV 格式

### 用户数据
- `users_sample.json` - 用户信息模拟数据
- `users_sample.csv` - 用户信息 CSV 格式

### 预约数据
- `reservations_sample.json` - 预约记录模拟数据
- `reservations_sample.csv` - 预约记录 CSV 格式

## 数据格式说明

### 座位状态 JSON 格式
```json
{
  "timestamp": 1698765432,
  "seats": [
    {"seat_id": "A001", "status": 1, "rssi": -65},
    {"seat_id": "A002", "status": 0, "rssi": -70}
  ]
}
```

### CSV 格式
```csv
seat_id,status,rssi,timestamp
A001,1,-65,1698765432
A002,0,-70,1698765432
```

## 使用说明

1. 测试数据仅用于开发和测试
2. 不要将测试数据用于生产环境
3. 定期更新测试数据以反映真实场景