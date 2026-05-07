# Python 鍚庣鏈嶅姟

鍩轰簬 FastAPI 鐨勫浘涔﹂搴т綅绠＄悊绯荤粺鍚庣鏈嶅姟銆?

## 鎶€鏈爤

- **妗嗘灦**: FastAPI
- **鏁版嵁瀛樺偍**: JSON 鏂囦欢锛堣交閲忕骇锛屾棤闇€鏁版嵁搴擄級
- **楠岃瘉**: Pydantic

## 鐩綍缁撴瀯

```
backend/
鈹溾攢鈹€ app/
鈹?  鈹溾攢鈹€ api/          # API 璺敱
鈹?  鈹溾攢鈹€ models/       # 鏁版嵁妯″瀷锛圝SON 鏂囦欢鎿嶄綔锛?
鈹?  鈹溾攢鈹€ schemas/      # Pydantic 妯″瀷
鈹?  鈹溾攢鈹€ services/     # 涓氬姟閫昏緫
鈹?  鈹斺攢鈹€ core/         # 鏍稿績閰嶇疆
鈹溾攢鈹€ data_templates/   # JSON 鏁版嵁妯℃澘锛堝彲鎻愪氦鍒?Git锛?
鈹溾攢鈹€ runtime_data/     # 杩愯鏃舵暟鎹紙涓嶆彁浜ゅ埌 Git锛?
鈹溾攢鈹€ main.py           # 搴旂敤鍏ュ彛
鈹溾攢鈹€ requirements.txt  # 渚濊禆
鈹斺攢鈹€ README.md         # 鏈枃浠?
```

## 鏁版嵁瀛樺偍鏂规

### JSON 鏂囦欢瀛樺偍
- 褰撳墠鐗堟湰浣跨敤 JSON 鏂囦欢浣滀负杞婚噺鏁版嵁琛?
- 鏃犻渶瀹夎鍜岄厤缃暟鎹簱锛岄檷浣庨儴缃插鏉傚害
- 閫傚悎褰撳墠 3 涓骇浣嶇殑灏忚妯″満鏅?

### 鏁版嵁妯℃澘锛坉ata_templates/锛?
- 鍙彁浜ゅ埌 GitHub 鐨勭ず渚?JSON 妯℃澘
- 鍖呭惈鏁版嵁缁撴瀯瀹氫箟鍜岀ず渚嬫暟鎹?
- 鐢ㄤ簬鍒濆鍖栨柊鐜

### 杩愯鏃舵暟鎹紙runtime_data/锛?
- 鏈湴鎴栦簯绔湡瀹炶繍琛屾暟鎹?
- **涓嶆彁浜ゅ埌 GitHub**
- 閫氳繃鐜鍙橀噺 `DATA_DIR` 鎸囧畾瀹為檯鏁版嵁鐩綍
- 浜戠寤鸿璺緞锛歚/var/lib/library_seat/data/`

## 涓昏鍔熻兘

- 鎺ユ敹 ESP32S3 涓婁紶鐨勫骇浣嶇姸鎬佸拰鍒峰崱浜嬩欢
- 璇诲彇 JSON 鏁版嵁鏂囦欢
- 鏇存柊 seat 鐘舵€?
- 杩藉姞 card_logs 鍒峰崱璁板綍
- 鎻愪緵 Web 鍓嶇鏌ヨ鎺ュ彛
- 鎻愪緵璁惧鐘舵€佹煡璇㈡帴鍙?

## 蹇€熷紑濮?

### 1. 瀹夎渚濊禆

```bash
pip install -r requirements.txt
```

### 2. 閰嶇疆鐜鍙橀噺

澶嶅埗 `.env.example` 涓?`.env` 骞朵慨鏀归厤缃細

```bash
cp .env.example .env
```

閰嶇疆 `DATA_DIR` 鎸囧畾 JSON 鏁版嵁鐩綍銆?

### 3. 鍒濆鍖栨暟鎹?

棣栨杩愯鏃讹紝鍚庣浼氳嚜鍔ㄤ粠 `data_templates/` 澶嶅埗妯℃澘鏂囦欢鍒版鐩綍銆?

### 4. 鍚姩鏈嶅姟

```bash
# 寮€鍙戞ā寮?
python -m uvicorn backend.main:app --reload --port 18080

# 鐢熶骇妯″紡
python -m uvicorn backend.main:app --host 0.0.0.0 --port 18080
```

## API 鏂囨。

鍚姩鏈嶅姟鍚庤闂細
- Swagger UI: http://localhost:18080/docs
- ReDoc: http://localhost:18080/redoc

## 寮€鍙戣鑼?

- 閬靛惊 PEP 8 浠ｇ爜椋庢牸
- 浣跨敤绫诲瀷娉ㄨВ
- 缂栧啓鍗曞厓娴嬭瘯
- API 閬靛惊 RESTful 瑙勮寖

## 鍚庢湡鎵╁睍

濡傛灉鏁版嵁閲忓彉澶э紝鍙縼绉诲埌 SQLite 鎴?MySQL 鏁版嵁搴撱€
