# Vue3 鍓嶇

鍩轰簬 Vue 3 鐨勫浘涔﹂搴т綅绠＄悊绯荤粺鍓嶇鐣岄潰銆?

## 鎶€鏈爤

- **妗嗘灦**: Vue 3 (Composition API)
- **鏋勫缓宸ュ叿**: Vite
- **UI 缁勪欢搴?*: Element Plus
- **鐘舵€佺鐞?*: Pinia
- **璺敱**: Vue Router 4
- **HTTP 瀹㈡埛绔?*: Axios
- **鍥捐〃**: ECharts

## 鐩綍缁撴瀯

```
frontend/
鈹溾攢鈹€ public/           # 闈欐€佽祫婧?
鈹溾攢鈹€ src/
鈹?  鈹溾攢鈹€ api/          # API 璇锋眰灏佽
鈹?  鈹溾攢鈹€ assets/       # 璧勬簮鏂囦欢锛堝浘鐗囥€佹牱寮忕瓑锛?
鈹?  鈹溾攢鈹€ components/   # 鍏叡缁勪欢
鈹?  鈹溾攢鈹€ views/        # 椤甸潰缁勪欢
鈹?  鈹溾攢鈹€ router/       # 璺敱閰嶇疆
鈹?  鈹斺攢鈹€ stores/       # Pinia 鐘舵€佺鐞?
鈹斺攢鈹€ README.md         # 鏈枃浠?
```

## 蹇€熷紑濮?

### 1. 瀹夎渚濊禆

```bash
npm install
# 鎴?
pnpm install
# 鎴?
yarn install
```

### 2. 閰嶇疆鐜鍙橀噺

澶嶅埗 `.env.example` 涓?`.env.local` 骞朵慨鏀归厤缃細

```bash
cp .env.example .env.local
```

### 3. 鍚姩寮€鍙戞湇鍔″櫒

```bash
npm run dev
# 鎴?
pnpm dev
# 鎴?
yarn dev
```

璁块棶 http://localhost:15173

### 4. 鏋勫缓鐢熶骇鐗堟湰

```bash
npm run build
# 鎴?
pnpm build
# 鎴?
yarn build
```

## 涓昏椤甸潰

- **鐧诲綍椤?*: 鐢ㄦ埛鐧诲綍
- **搴т綅绠＄悊**: 绠＄悊鍛樻煡鐪嬪拰绠＄悊搴т綅
- **搴т綅棰勭害**: 鐢ㄦ埛棰勭害搴т綅
- **缁熻鎶ヨ〃**: 鏁版嵁缁熻鍜屽彲瑙嗗寲
- **绯荤粺璁剧疆**: 绯荤粺鍙傛暟閰嶇疆

## 寮€鍙戣鑼?

- 浣跨敤 Composition API
- 缁勪欢閲囩敤 `<script setup>` 璇硶
- 浣跨敤 TypeScript锛堝彲閫夛級
- 閬靛惊 Vue 3 鏈€浣冲疄璺?
- 缁勪欢鍛藉悕浣跨敤 PascalCase
