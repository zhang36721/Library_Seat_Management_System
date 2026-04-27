# 资源文件

本目录用于存放静态资源文件。

## 目录结构（待实现）

```
assets/
├── images/         # 图片文件
├── styles/         # 全局样式
│   ├── variables.css   # CSS 变量
│   ├── global.css      # 全局样式
│   └── theme.css       # 主题样式
├── fonts/          # 字体文件
└── icons/          # 图标文件
```

## 使用方式

```javascript
// 在组件中引入图片
import logo from '@/assets/images/logo.png'

// 在样式中引入
@import '@/assets/styles/global.css';