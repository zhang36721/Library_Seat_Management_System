# 娴嬭瘯璁板綍

## v0.8.4 鎸夐敭鑿滃崟鍔熻兘闂幆璁板綍

### 鍔熻兘鑼冨洿

v0.8.4 灏?v0.8.3 鑿滃崟涓殑 placeholder 琛ユ垚涓绘帶鏈湴鍔熻兘闂幆锛歍ime Set 鍐欏叆 DS1302锛孋ard Add/Del 鎿嶄綔 RAM 鍗¤〃锛孎F A1 鍒峰崱娴佺▼鎸夋湰鍦板崱琛ㄥ垽鏂槸鍚︽斁琛屻€傛湰闃舵浠嶄笉渚濊禆 ZigBee銆佷笉渚濊禆 ESP32S3銆佷笉鎺ュ悗绔拰鍓嶇銆?

### 鏂板/瀹屽杽鍔熻兘

| 鍔熻兘 | 鎿嶄綔 | 棰勬湡缁撴灉 |
|------|------|----------|
| 鏃堕棿璁剧疆鍐欏叆 | TIME SET 鐣岄潰 K6 鐭寜 | 璋冪敤 `kt_ds1302_set_time()`锛孫LED 鏄剧ず `TIME SAVED` 鎴?`TIME FAIL` |
| IC 鍗℃敞鍐?| CARD ADD 鐣岄潰 K6 鐭寜 | 璇诲彇 RC522 UID锛屽姞鍏?RAM 鍗¤〃锛涢噸澶嶅崱鏄剧ず `EXISTS`锛屾弧琛ㄦ樉绀?`FULL` |
| IC 鍗″垹闄?| CARD DEL 鐣岄潰 K6 鐭寜 | 璇诲彇 RC522 UID锛屼粠 RAM 鍗¤〃鍒犻櫎锛涙湭鎵惧埌鏄剧ず `NOT FOUND` |
| 鏈湴鍒峰崱鍒ゅ畾 | `FF A1 00 FF` | 鏈敞鍐屽崱鏄剧ず `CARD DENIED`锛屽凡娉ㄥ唽鍗℃樉绀?`CARD OK` |
| 鎵撳嵃鍗¤〃 | `FF B6 00 FF` | 鎵撳嵃鏈湴 RAM 鍗¤〃 |
| 娓呯┖鍗¤〃 | `FF B7 00 FF` | 娓呯┖鏈湴 RAM 鍗¤〃 |

### 寰呴獙鏀跺懡浠?

| 鍛戒护/鎿嶄綔 | 楠屾敹鐩爣 | 棰勬湡缁撴灉 |
|-----------|----------|----------|
| K6 in TIME SET | 鍐欏叆 DS1302 | OLED 鏄剧ず `TIME SAVED`锛宍FF 60 00 FF` 鍙鍥炴柊鏃堕棿 |
| K6 in CARD ADD | 娉ㄥ唽褰撳墠 IC 鍗?| OLED 鏄剧ず `CARD ADD / OK / UID` |
| K6 in CARD ADD锛屽悓涓€寮犲崱 | 閲嶅娉ㄥ唽淇濇姢 | OLED 鏄剧ず `CARD ADD / EXISTS / UID` |
| K6 in CARD DEL | 鍒犻櫎褰撳墠 IC 鍗?| OLED 鏄剧ず `CARD DEL / OK / UID` |
| K6 in CARD DEL锛屾湭娉ㄥ唽鍗?| 鍒犻櫎涓嶅瓨鍦ㄤ繚鎶?| OLED 鏄剧ず `CARD DEL / NOT FOUND / UID` |
| `FF B6 00 FF` | 鎵撳嵃鍗¤〃 | USART2 鎵撳嵃宸叉敞鍐?UID |
| `FF B7 00 FF` | 娓呯┖鍗¤〃 | USART2 鎵撳嵃 `Card DB cleared` |
| `FF A1 00 FF` 鏈敞鍐屽崱 | 鍒峰崱鎷掔粷 | OLED 鏄剧ず `CARD DENIED`锛屾棩蹇?`CARD EVENT: DENIED` |
| `FF A1 00 FF` 宸叉敞鍐屽崱 | 鍒峰崱閫氳繃 | OLED 鏄剧ず `CARD OK`锛屾棩蹇?`CARD EVENT: CHECK_IN OK` |

RAM 鍗¤〃褰撳墠瀹归噺涓?10 寮犲崱锛屼笉鍋?Flash 鎸佷箙鍖栵紱鏂數鍚庡崱琛ㄦ竻绌恒€?

## v0.8.3 8 璺嫭绔嬫寜閿笌 OLED 鑿滃崟璁板綍

### 鎺ョ嚎纭

褰撳墠鎸夐敭妯″潡瀹炵墿涓?8 浣嶇嫭绔嬫寜閿ā鍧楋紝涓嶆槸鐭╅樀閿洏銆傛ā鍧?VCC 鎺?STM32 3.3V锛孏ND 鎺?STM32 GND锛孠1~K8 鍒嗗埆鎺ョ嫭绔?GPIO 杈撳叆銆?

| 鎸夐敭 | STM32 寮曡剼 | 鍔熻兘 | 楠屾敹鍛戒护 |
|------|------------|------|----------|
| K1 | PA1 | MENU / 杩涘叆鑿滃崟 / 闀挎寜杩斿洖 | `FF B1/B2 00 FF` |
| K2 | PA8 | UP / 鏁板€煎姞 | `FF B1/B2 00 FF` |
| K3 | PA11 | DOWN / 鏁板€煎噺 | `FF B1/B2 00 FF` |
| K4 | PA12 | LEFT / 涓婁竴涓瓧娈?| `FF B1/B2 00 FF` |
| K5 | PA15 | RIGHT / 涓嬩竴涓瓧娈?| `FF B1/B2 00 FF` |
| K6 | PB3 | OK / 纭 | `FF B1/B2 00 FF` |
| K7 | PB4 | CARD ADD / 娉ㄥ唽 IC 鍗?| `FF B1/B2 00 FF` |
| K8 | PC14 | CARD DEL / 鍒犻櫎 IC 鍗?| `FF B1/B2 00 FF` |

鎸夐敭榛樿浣庣數骞虫湁鏁堬細鏈寜涓?raw=1锛屾寜涓?raw=0銆侾A15 / PB3 / PB4 宸插叧闂?JTAG銆佷繚鐣?SWD銆?

### v0.8.3 寰呴獙鏀跺懡浠?

| 鍛戒护 | 楠屾敹鐩爣 | 棰勬湡缁撴灉 |
|------|----------|----------|
| `FF B0 00 FF` | 鎵撳嵃鎸夐敭鏄犲皠 | K1~K8 鏄犲皠鍜屽姛鑳借鏄庢纭?|
| `FF B1 00 FF` | 鎵撳嵃鍘熷鐢靛钩 | K1~K8 raw/pressed 涓庡疄鐗╂寜涓嬬姸鎬佷竴鑷?|
| `FF B2 00 FF` | 鎵撳嵃鏈€杩戞寜閿簨浠?| 鐭寜鏄剧ず `Kx SHORT`锛孠1 闀挎寜鏄剧ず `K1 LONG` |
| `FF B3 00 FF` | 杩涘叆鏃堕棿璁剧疆鐣岄潰 | OLED 鏄剧ず `TIME SET` |
| `FF B4 00 FF` | 杩涘叆 IC 鍗℃敞鍐岀晫闈?| OLED 鏄剧ず `CARD ADD` |
| `FF B5 00 FF` | 杩涘叆 IC 鍗″垹闄ょ晫闈?| OLED 鏄剧ず `CARD DEL` |

### ESP32S3 USART3 鎺ョ嚎鏇存柊

STM32 涓绘帶渚т繚鎸?USART3锛歅B10 涓?TX锛孭B11 涓?RX銆侲SP32S3 渚ц鍒掍负 GPIO47 UART_TX / GPIO48 UART_RX銆?

| STM32 涓绘帶 | 鏂瑰悜 | ESP32S3 |
|------------|------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | -> | GND |

USART2 debug 淇濇寔鐙珛锛屼笉鍏佽 ESP32S3 鍗犵敤 USART2銆?

## v0.8.2 涓绘帶鏈湴鍒峰崱涓庢樉绀洪棴鐜褰?

### 宸插畬鎴愬疄娴?

| 妯″潡 | 瀹炴祴缁撴灉 | 缁撹 |
|------|----------|------|
| RC522 鏈夊崱璇诲彇 | `[INFO] RC522 UID: BF A4 A5 1F BCC=A1` | 閫氳繃 |
| RC522 鏃犲崱妫€娴?| `[WARN] RC522 no card detected` | 閫氳繃 |
| DS1302 鍚姩璇绘椂 | `[INFO] DS1302 init: OK, time=2026-04-28 12:59:36` | 閫氳繃 |
| OLED | 鍒濆鍖?OK锛屽睆骞曞彲鏄剧ず | 閫氳繃 |
| 铚傞福鍣?| Debug 鍛戒护瀹炴祴鍙搷 | 閫氳繃 |
| 鎸夐敭 | Debug 鍛戒护瀹炴祴鍙 | 閫氳繃 |
| Stepper | `FF 70 / FF 71 / FF 72` 鍛戒护閾捐矾姝ｅ父锛孶LN2003 鎸囩ず鐏湁鐏晥锛岀數鏈烘湰浣撲笉杞?| 涓嶉樆濉?v0.8.2锛屽緟澶嶆煡渚涚數/鐩稿簭/绾垮簭/椹卞姩鏉?鐢垫満鎺ュ彛 |

### v0.8.2 寰呴獙鏀跺懡浠?

| 鍛戒护 | 楠屾敹鐩爣 | 棰勬湡缁撴灉 |
|------|----------|----------|
| `FF A0 00 FF` | 鎵撳嵃褰撳墠涓绘帶涓氬姟鐘舵€?| 鎵撳嵃妯℃嫙搴т綅鐘舵€佸拰鏈€杩戝埛鍗＄粨鏋?|
| `FF A1 00 FF` | 鏈湴鍒峰崱娴佺▼娴嬭瘯 | 鏈夊崱鏃惰 UID銆佽 DS1302 鏃堕棿銆丱LED 鏄剧ず鎴愬姛銆佽渹楦ｅ櫒鎴愬姛鎻愮ず銆乁SART2 鎵撳嵃浜嬩欢 |
| `FF A1 00 FF` | 鏈湴鍒峰崱澶辫触娴嬭瘯 | 鏃犲崱鏃?OLED 鏄剧ず澶辫触銆佽渹楦ｅ櫒澶辫触鎻愮ず銆乁SART2 鎵撳嵃 `CARD EVENT: NO_CARD` |
| `FF A2 00 FF` | OLED 棣栭〉 | 鏄剧ず `KENTO LIB`銆佹椂闂淬€丼1/S2/S3 妯℃嫙鐘舵€?|
| `FF A3 00 FF` | OLED 鏈€杩戝埛鍗＄粨鏋?| 鏄剧ず鏈€杩戜竴娆℃垚鍔熸垨澶辫触缁撴灉 |
| `FF A4 00 FF` | 铚傞福鍣ㄦ垚鍔熸彁绀?| 80ms 鐭搷 |
| `FF A5 00 FF` | 铚傞福鍣ㄥけ璐ユ彁绀?| 200ms 鐭搷 |
| `FF A6 00 FF` | 妯℃嫙搴т綅鐘舵€佸彉鍖?| S1/S2/S3 FREE/OCCUPIED 妯℃嫙鐘舵€佸垏鎹?|

### DS1302 鏃堕棿鏍″噯鍛戒护

`DATA` 鏄竴涓瓧鑺傦紝涓插彛宸ュ叿鎸?HEX 鍙戦€併€備緥濡傚崄杩涘埗 29 闇€瑕佸彂閫?`1D`銆?

| 鍛戒护 | 鐢ㄩ€?| 鑼冨洿 |
|------|------|------|
| `FF 62 YY FF` | 璁剧疆寰呭啓鍏ュ勾浠斤紝琛ㄧず 20YY | 0~99 |
| `FF 63 MM FF` | 璁剧疆寰呭啓鍏ユ湀浠?| 1~12 |
| `FF 64 DD FF` | 璁剧疆寰呭啓鍏ユ棩鏈?| 1~31 |
| `FF 65 hh FF` | 璁剧疆寰呭啓鍏ュ皬鏃?| 0~23 |
| `FF 66 mm FF` | 璁剧疆寰呭啓鍏ュ垎閽?| 0~59 |
| `FF 67 ss FF` | 璁剧疆寰呭啓鍏ョ | 0~59 |
| `FF 68 00 FF` | 鏍￠獙骞舵彁浜ゅ啓鍏?DS1302 | 鍚堟硶鏃堕棿鎵嶅啓鍏?|
| `FF 69 00 FF` | 鎵撳嵃褰撳墠寰呭啓鍏ユ椂闂寸紦瀛?| 鍙墦鍗颁笉鍐欏叆 |

璁剧疆 `2026-04-29 11:28:00` 鐨勫彂閫侀『搴忥細

```text
FF 62 1A FF
FF 63 04 FF
FF 64 1D FF
FF 65 0B FF
FF 66 1C FF
FF 67 00 FF
FF 69 00 FF
FF 68 00 FF
FF 60 00 FF
```

`FF 61 00 FF` 淇濈暀涓哄浐瀹氭祴璇曞啓鍏ュ懡浠わ細`2026-04-28 12:34:00`銆?

## v0.8.5 涓绘帶鏈湴閫氳璁板綍涓庤嚜鍔ㄥ埛鍗?

鏈樁娈典笉渚濊禆 ZigBee銆佷笉渚濊禆 ESP32S3銆佷笉鎺ュ悗绔拰鍓嶇锛岀洰鏍囨槸涓绘帶鏈湴鐪熷疄鍒峰崱浣撻獙銆?

| 楠屾敹椤?| 鎿嶄綔 | 棰勬湡缁撴灉 | 瀹炴満缁撴灉 |
|--------|------|----------|----------|
| OLED 棣栭〉 | 涓婄數 | 鏄剧ず `KENTO LIBRARY`銆乣SWIPE CARD`銆佸崱鏁般€佹棩蹇楁暟鍜屾椂闂?| 寰呮祴 |
| 鑷姩鍒峰崱 | 宸叉敞鍐屽崱闈犺繎 RC522 | 鏃犻渶 `FF A1`锛岃嚜鍔ㄦ樉绀?`ACCESS OK` 骞惰褰曟棩蹇?| 寰呮祴 |
| 闃查噸澶嶅埛 | 鍚屼竴寮犲崱鎸佺画鏀惧湪璇诲崱鍣ㄤ笂 | 涓嶈繛缁噸澶嶈Е鍙?| 寰呮祴 |
| 鎷垮紑鍐嶅埛 | 鎷垮紑鍗″悗鍐嶆闈犺繎 | 鍏佽涓嬩竴娆″埛鍗?| 寰呮祴 |
| 鏈敞鍐屽崱 | 鏈敞鍐屽崱闈犺繎 RC522 | OLED 鏄剧ず `ACCESS DENIED`锛岀數鏈轰笉鍔ㄤ綔 | 寰呮祴 |
| 闂搁棬鑱斿姩 | 宸叉敞鍐屽崱鍒峰崱鎴愬姛 | 姝ヨ繘鐢垫満寮€闂搞€佷繚鎸併€佸叧闂搞€佸仠姝㈢嚎鍦?| 寰呮祴 |
| 鎵撳嵃璁板綍 | `FF C0 00 FF` | 鎵撳嵃鏈€杩戦€氳璁板綍鎴?`ACCESS LOG EMPTY` | 寰呮祴 |
| 娓呯┖璁板綍 | `FF C1 00 FF` | 鎵撳嵃 `ACCESS LOG CLEARED` | 寰呮祴 |
| 璁板綍缁熻 | `FF C2 00 FF` | 鎵撳嵃璁板綍鎬绘暟銆丱K 鏁般€丏ENIED 鏁?| 寰呮祴 |
| 鎵嬪姩鍒峰崱 | `FF A1 00 FF` | 浠嶅彲鎵嬪姩鎵ц鏈湴鍒峰崱娴嬭瘯 | 寰呮祴 |
| 鑿滃崟鏆傚仠杞 | 杩涘叆 TIME SET / CARD ADD / CARD DEL / 鑿滃崟 | 鑷姩鍒峰崱鏆傚仠锛屾斁鍗′笉浼氳Е鍙?ACCESS DENIED 鎴栨甯搁€氳 | 寰呮祴 |
| 閫€鍑烘仮澶嶈疆璇?| K1 闀挎寜杩斿洖棣栭〉 | 鑷姩鍒峰崱鎭㈠ | 寰呮祴 |
| 鎺夌數淇濇姢 | 浜х敓璁板綍鍚庢柇鐢甸噸鍚紝鍐嶅彂 `FF C0 00 FF` | 鏈€杩?50 鏉¤褰曚粠 Flash 鎭㈠ | 寰呮祴 |
| 鍗¤〃鎺夌數淇濇姢 | 娉ㄥ唽 IC 鍗″悗鏂數閲嶅惎锛屽啀鍙?`FF B6 00 FF` | 宸叉敞鍐屽崱鍒楄〃浠?Flash 鎭㈠ | 寰呮祴 |

浠ｇ爜渚?Keil Rebuild All 宸查€氳繃锛歚0 Error(s), 0 Warning(s)`銆傛渶缁堝叧闂増鏈粛浠ュ疄鏈洪獙鏀剁粨鏋滀负鍑嗐€?

鏈湴鏁版嵁鎸佷箙鍖栬鏄庯細宸叉敞鍐屽崱琛ㄥ拰鏈€杩?50 鏉￠€氳璁板綍鍏卞悓淇濆瓨鍒?STM32 鍐呴儴 Flash `0x0800F800`锛宐oot count 浠嶄娇鐢?`0x0800FC00`銆侹eil IROM 宸查鐣欐渶鍚庝袱椤碉紝閬垮厤绋嬪簭鍖鸿鐩栨寔涔呭寲鏁版嵁銆?
### 搴т綅绔帇鍔?閲嶅姏妫€娴嬫洿鏂?
| 娴嬭瘯椤?| 鎿嶄綔 | 棰勬湡缁撴灉 | 褰撳墠缁撴灉 |
|------|------|----------|----------|
| 搴т綅 1 HX711 + 绾㈠ | 鎺ュ叆鍘嬪姏浼犳劅鍣ㄥ拰 PB9 绾㈠锛屽彂閫?`FF 30 00 FF` | 鎵撳嵃 HX711 raw/delta 鍜?PB9 绾㈠ raw锛涙湭鎺ユ垨鏈?ready 鏃舵樉绀?`NOT_READY` | 寰呭疄鏈哄帇娴?|
| 搴т綅 1 鐘舵€?| 鍙戦€?`FF 31 00 FF` | 閲嶉噺 `> 50g` 涓?PB9 绾㈠瑙﹀彂鏃朵负 `OCCUPIED`锛屽惁鍒?`FREE`锛汬X711 鏈?ready 涓?`UNKNOWN` | 寰呭疄鏈哄帇娴嬨€?0g 闃堝€煎拰绾㈠鏈夋晥鐢靛钩璋冩暣 |
| HX711 鍘荤毊 | 绌哄骇鍙戦€?`FF 32 00 FF` | 鎵撳嵃 `HX711 tare OK`锛屾洿鏂扮┖搴?offset | 寰呭疄鏈洪獙璇?|
| HX711 鍏嬮噸鏍囧畾 | 鏀?50g 宸茬煡閲嶉噺锛屽彂閫?`FF 33 05 FF`锛涜嫢 500g 鍒欏彂閫?`FF 33 32 FF` | 鎵撳嵃 `HX711 calib OK` 鍜?scale锛屽苟淇濆瓨鍒?Flash | 寰呭疄鏈洪獙璇?|
| HX711 鏍囧畾鎺夌數淇濆瓨 | 鏍囧畾鍚庢柇鐢甸噸鍚?| 涓婄數鎵撳嵃 `HX711 calib loaded from Flash`锛屾棤闇€閲嶆柊鏍囧畾 | 寰呭疄鏈洪獙璇?|
| HX711 榛樿鍙傛暟 | 鎿﹂櫎 Flash 鎴?Flash 鏍￠獙澶辫触鍚庝笂鐢?| 浣跨敤鍥轰欢榛樿 `offset=-250747`锛宍scale=40667 counts/g x100` | 宸插啓鍏ュ浐浠讹紝寰呮摝闄ら獙璇?|
| HX711 瀹炴祴鍏嬮噸 | 230g / 绾?11g / 绾?3g 鏍峰搧 | 杈撳嚭绾?229~230g銆?10~111g銆?2~33g | 宸插疄娴嬮€氳繃 |
| HX711 鍏嬮噸璇诲彇 | 鏍囧畾鍚庡彂閫?`FF 34 00 FF` | 鎵撳嵃 `HX711 weight=... g` | 寰呭疄鏈洪獙璇?|
| 搴т綅 2 妯℃嫙 | PB8 杈撳叆楂樼數骞筹紝鍙戦€?`FF 31 00 FF` | 搴т綅 2 鏄剧ず `OCCUPIED`锛涗綆鐢靛钩/鎮┖涓嬫媺鏄剧ず `FREE` | 寰呭疄鏈洪獙璇?|
| 搴т綅 3 妯℃嫙 | PB7 杈撳叆楂樼數骞筹紝鍙戦€?`FF 31 00 FF` | 搴т綅 3 鏄剧ず `OCCUPIED`锛涗綆鐢靛钩/鎮┖涓嬫媺鏄剧ず `FREE` | 寰呭疄鏈洪獙璇?|
| 搴т綅鐘舵€佸彉鍖栨棩蹇?| 淇濇寔搴т綅鐘舵€佷笉鍙橈紝鍐嶆ā鎷熸潵浜?绂诲紑 | 鐘舵€佷笉鍙樻椂涓嶅埛灞忥紱杩炵画 3 娆￠噰鏍蜂竴鑷村悗纭鍙樺寲锛孲eat1/Seat2/Seat3 鍙墦鍗颁竴娆″彉鍖栨棩蹇?| 寰呭疄鏈洪獙璇?|
| ZigBee 鐐瑰鐐逛笂鎶?| 鍙戦€?`FF 80 00 FF` | 閫氳繃 `FA 00 00 04 10 S1 S2 S3 F5` 鍙戦€佷簩杩涘埗搴т綅鐘舵€?| 寰呭疄鏈洪獙璇?|

### CC2530 ZigBee 鐐瑰鐐归€氫俊楠屾敹

| 娴嬭瘯椤?| 鎿嶄綔 | 棰勬湡缁撴灉 | 褰撳墠缁撴灉 |
|------|------|----------|----------|
| 涓绘帶鍙戝骇浣嶇 hello | 涓绘帶 debug 鍙戦€?`FF 80 00 FF` | 涓绘帶閫氳繃 USART1 鍙?`FA 1E 30 05 68 65 6C 6C 6F F5`锛屽骇浣嶇鏀跺埌 `hello` 鎴栧畬鏁村抚 | 浠ｇ爜瀹屾垚锛屽緟瀹炴満楠岃瘉 |
| 涓绘帶 PING | 涓绘帶 debug 鍙戦€?`FF 86 00 FF` | 涓绘帶鍙?`FA 1E 30 02 01 seq F5` 鍒?`0x301E`锛屽骇浣嶇鏀跺埌鍚庡洖 `FA 00 00 02 02 seq F5` 鍒?`0x0000` | 浠ｇ爜瀹屾垚锛屽緟瀹炴満楠岃瘉 |
| 搴т綅绔姸鎬佷笂鎶?| 搴т綅绔笂鐢垫垨搴т綅鐘舵€佺ǔ瀹氬彉鍖?| 鍙戦€?`FA 00 00 04 10 S1 S2 S3 F5`锛涚姸鎬佷笉鍙樻椂涓嶉噸澶嶄笂鎶?| 浠ｇ爜瀹屾垚锛屽緟瀹炴満楠岃瘉 |
| 涓绘帶瑙ｆ瀽搴т綅鐘舵€?| 涓绘帶鏀跺埌搴т綅绔笂鎶?| 浠呯姸鎬佸彉鍖栨椂鎵撳嵃 `[ZIGBEE] RX SEAT: S1=x S2=x S3=x FREE=x`锛屽苟鏇存柊 Web 渚у骇浣嶇姸鎬?| 浠ｇ爜瀹屾垚锛屽緟瀹炴満楠岃瘉 |
| USART2 鐙珛鎬?| ZigBee 閫氫俊鏈熼棿缁х画鍙戦€?debug 鍛戒护 | USART2 `FF CMD DATA FF` 涓嶈 ZigBee 鏁版嵁姹℃煋 | 寰呭疄鏈洪獙璇?|

### v0.8.2 闃舵杈圭晫

- 涓嶄緷璧?ZigBee銆?
- 涓嶄緷璧?ESP32S3銆?
- 涓嶆帴鍚庣鍜屽墠绔€?
- Stepper 淇濈暀鎺ュ彛锛屼笉浣滀负鏈樁娈甸樆濉為」銆?

## 1. 娴嬭瘯鐜

### 1.1 纭欢鐜
- 搴т綅妫€娴嬭妭鐐?脳 N
- 涓绘帶鑺傜偣 脳 1
- ESP32S3 缃戝叧 脳 1
- 娴嬭瘯鐢?PC

### 1.2 杞欢鐜
- 鍚庣鏈嶅姟鐗堟湰锛歷0.1.0
- 鍓嶇鐗堟湰锛歷0.1.0
- 鏁版嵁搴擄細MySQL 8.0
- 娴嬭瘯宸ュ叿锛歅ostman / JMeter

## 2. 娴嬭瘯绫诲瀷

### 2.1 鍔熻兘娴嬭瘯
| 娴嬭瘯椤?| 娴嬭瘯鏂规硶 | 棰勬湡缁撴灉 | 瀹為檯缁撴灉 | 娴嬭瘯鏃ユ湡 | 娴嬭瘯浜?|
|--------|----------|----------|----------|----------|--------|
| 搴т綅鐘舵€佹娴?| 妯℃嫙浜哄憳鍧愪笅/绂诲紑 | 鐘舵€佹纭彉鍖?| 寰呮祴璇?| TBD | TBD |
| 鏁版嵁涓婁紶 | 妫€鏌ユ暟鎹槸鍚︽纭笂浼?| 鏈嶅姟鍣ㄦ敹鍒版暟鎹?| 寰呮祴璇?| TBD | TBD |
| 棰勭害鍔熻兘 | 鐢ㄦ埛棰勭害搴т綅 | 棰勭害鎴愬姛 | 寰呮祴璇?| TBD | TBD |
| 绛惧埌鍔熻兘 | 鐢ㄦ埛鎵爜绛惧埌 | 绛惧埌鎴愬姛 | 寰呮祴璇?| TBD | TBD |

### 2.2 鎬ц兘娴嬭瘯
| 娴嬭瘯椤?| 娴嬭瘯鏂规硶 | 棰勬湡鎸囨爣 | 瀹為檯缁撴灉 | 娴嬭瘯鏃ユ湡 | 娴嬭瘯浜?|
|--------|----------|----------|----------|----------|--------|
| 骞跺彂鐢ㄦ埛鏁?| 妯℃嫙澶氱敤鎴峰悓鏃舵搷浣?| 鈮?00 骞跺彂 | 寰呮祴璇?| TBD | TBD |
| 鍝嶅簲鏃堕棿 | 娴嬮噺 API 鍝嶅簲鏃堕棿 | <2 绉?| 寰呮祴璇?| TBD | TBD |
| 鏁版嵁鍑嗙‘鎬?| 闀挎椂闂磋繍琛屾娴?| 鍑嗙‘鐜?>95% | 寰呮祴璇?| TBD | TBD |

### 2.3 绋冲畾鎬ф祴璇?
| 娴嬭瘯椤?| 娴嬭瘯鏂规硶 | 棰勬湡缁撴灉 | 瀹為檯缁撴灉 | 娴嬭瘯鏃ユ湡 | 娴嬭瘯浜?|
|--------|----------|----------|----------|----------|--------|
| 闀挎椂闂磋繍琛?| 杩炵画杩愯 7 澶?| 鏃犲穿婧冦€佹棤鍐呭瓨娉勬紡 | 寰呮祴璇?| TBD | TBD |
| 缃戠粶寮傚父 | 妯℃嫙缃戠粶涓柇鎭㈠ | 鏁版嵁涓嶄涪澶?| 寰呮祴璇?| TBD | TBD |
| 鏂數鎭㈠ | 妯℃嫙璁惧鏂數閲嶅惎 | 鑷姩鎭㈠宸ヤ綔 | 寰呮祴璇?| TBD | TBD |

## 3. Bug 璁板綍

| Bug ID | 鎻忚堪 | 涓ラ噸绋嬪害 | 鐘舵€?| 淇鏃ユ湡 | 淇浜?|
|--------|------|----------|------|----------|--------|
| BUG-001 | 寰呰褰?| 寰呰瘎浼?| 寰呬慨澶?| TBD | TBD |

## 4. 娴嬭瘯鎬荤粨

### 4.1 娴嬭瘯瑕嗙洊鐜?
- 鍔熻兘娴嬭瘯瑕嗙洊鐜囷細寰呯粺璁?
- 浠ｇ爜瑕嗙洊鐜囷細寰呯粺璁?

### 4.2 閬楃暀闂
- 寰呰ˉ鍏?

### 4.3 鏀硅繘寤鸿
- 寰呰ˉ鍏?
# v0.9 ESP32S3 Binary Link Test Record

| Test | Operation | Expected result | Result |
|------|-----------|-----------------|--------|
| ESP32S3 WiFi | Boot ESP32S3 gateway | Try configured WiFi profiles; print connected SSID/IP/RSSI or offline | Pending |
| STM32 PING | `FF D0 00 FF` | STM32 sends `PING`, ESP32 replies `PONG` | Pending |
| WiFi status | `FF D1 00 FF` after ESP32 sends status | STM32 prints WiFi state, SSID, IP, RSSI | Pending |
| Link status | `FF D2 00 FF` | STM32 prints ESP32 link and WiFi status | Pending |
| Mock card event | `FF D3 00 FF` | ESP32 parses `CARD_EVENT` and replies `ACK` | Pending |
| Real card event | Swipe a registered or denied card | Local access still works; ESP32 receives `CARD_EVENT` | Pending |
| Bad CRC | `FF D5 00 FF` | ESP32 replies `ERR` with CRC reason | Pending |
| USART2 isolation | Send debug commands while UART3 traffic exists | USART2 `FF CMD DATA FF` remains independent | Pending |

## v0.9.1 ESP32 Active Heartbeat Test Record

| Test | Operation | Expected result | Result |
|------|-----------|-----------------|--------|
| Bright LED | ESP32 boot | RGB LED is requested off, or PWR LED is confirmed not software controllable | Pending |
| GPIO48 conflict | ESP32 UART traffic on GPIO48 | Confirm whether board RGB LED flickers or interferes with UART RX | Pending |
| Active heartbeat | ESP32 powered | `[HB TX] seq=...` every 3 seconds | Pending |
| STM32 offline | ESP32 powered while STM32 disconnected | After 3 missed ACKs, ESP32 prints `[STM32] offline` | Pending |
| STM32 heartbeat ACK | STM32 connected and powered | STM32 replies `HEARTBEAT_ACK`; ESP32 prints `[HB ACK]` | Pending |
| PC13 pulse | ESP32 heartbeat received | STM32 PC13 pulses for about 50 ms | Pending |
| Recovery | Connect/reset STM32 after ESP32 offline | ESP32 prints `[STM32] recovered` | Pending |
| Initial sync | First heartbeat after link capture | ESP32 prints `[SYNC RX] cards=... logs=... time=...` | Pending |
| FF D2 status | `FF D2 00 FF` | STM32 prints ONLINE/OFFLINE, heartbeat count, WiFi and last RX type | Pending |
| EOF check | Send malformed frame tail | Parser rejects frame and records EOF/format error | Pending |

## v1.2.1 Active Upload Demo Test Record

| Test | Operation | Expected result | Result |
|------|-----------|-----------------|--------|
| Backend listen address | Start backend with `uvicorn backend.main:app --host 0.0.0.0 --port 18080` | ESP32 can reach backend by PC/server LAN IP, not `localhost` | Code verified; real hardware pending |
| ESP32 backend URL | Select URL by connected WiFi SSID | `B4-3408_2.4G -> 192.168.1.100`, `301 -> 192.168.141.236`, `310 -> 192.168.223.93` | Code verified; real hardware pending |
| Cloud heartbeat | ESP32 powered and WiFi connected | POST `/api/iot/heartbeat` every 2 seconds; log OK/FAIL code and failed count | Pending real hardware |
| ESP32 online display | Web reads `/api/iot/status` | Shows ESP32 online after heartbeat, connection lost after about 10 seconds without heartbeat | API simulated OK; real hardware pending |
| STM32 online display | STM32 and ESP32 UART link active | Web shows STM32 online from uploaded status | Pending real hardware |
| Seat status | STM32 sends `DEVICE_STATUS` | Web shows seat 1 free, seat 2 occupied, seat 3 free from temporary main-controller simulation | API simulated OK; real hardware pending |
| Gate status | Swipe registered card | Web shows gate state sequence: opening -> open -> closing -> closed | Code verified; real hardware pending |
| Card event | Swipe a registered/denied card | ESP32 POSTs `/api/iot/card-event`; Web table shows record | API simulated OK; real hardware pending |
| Clear events | Click Web clear button | Backend RAM card-event cache is cleared | API available; real hardware pending |
## v1.2.1 瀹炴満閾捐矾楠屾敹璁板綍

| 楠屾敹椤?| 鎿嶄綔 | 鏈熸湜缁撴灉 | 褰撳墠缁撴灉 |
|------|------|----------|----------|
| ESP32 heartbeat POST | ESP32 涓婄數骞惰繛鎺?WiFi | 鍚庣鏀跺埌 `/api/iot/heartbeat`锛孒TTP 200 OK | 鎺ュ彛妯℃嫙 200 OK锛屽疄鏈哄緟纭 |
| ESP32 card-event POST | STM32 鍒峰崱鍚?ESP32 涓婃姤 | 鍚庣鏀跺埌 `/api/iot/card-event`锛孒TTP 200 OK | 鎺ュ彛妯℃嫙 200 OK锛屽疄鏈哄緟纭 |
| Web GET status | 娴忚鍣ㄦ墦寮€鍓嶇椤甸潰 | `GET /api/iot/status` 200 OK | 鏈湴鎺ュ彛 200 OK |
| Web GET card-events | 椤甸潰鍒锋柊鍒峰崱璁板綍 | `GET /api/iot/card-events` 200 OK | 鏈湴鎺ュ彛鍙敤 |
| ESP32 鍦ㄧ嚎鏄剧ず | heartbeat 鎸佺画涓婃姤 | Web 鏄剧ず ESP32 缃戝叧鍦ㄧ嚎 | 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| ESP32 杩炴帴涓㈠け | 鏂紑 ESP32 鎴栧叧闂?WiFi | 绾?10 绉掑悗 Web 鏄剧ず杩炴帴涓㈠け | 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| 鍒峰崱璁板綍鏄剧ず | 鍒峰凡娉ㄥ唽鎴栨湭娉ㄥ唽鍗?| Web 鏄剧ず UID銆佸叆棣?绂婚/鎷掔粷銆侀€氳繃/鎷掔粷 | 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| 搴т綅鐘舵€佹樉绀?| STM32 涓婃姤 `DEVICE_STATUS` | Web 鏄剧ず搴т綅 1/2/3 绌洪棽/鍗犵敤/鏈煡 | 鎺ュ彛妯℃嫙閫氳繃锛屽疄鏈哄緟纭 |
| 闂搁棬鐘舵€佹樉绀?| 宸叉敞鍐屽崱瑙﹀彂寮€闂ㄦ祦绋?| Web 鏄剧ず寮€闂ㄤ腑 -> 宸插紑鍚?-> 鍏抽棬涓?-> 宸插叧闂?| 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| Clear Events | 鐐瑰嚮鍓嶇娓呯┖璁板綍 | 鍚庣鏈€杩戝埛鍗¤褰曟竻绌?| 鎺ュ彛鍙敤 |

鍚庣瀹炴満鍚姩鍛戒护锛?
```bash
python -m uvicorn backend.main:app --host 0.0.0.0 --port 18080
```

ESP32 `SERVER_BASE_URL` 蹇呴』浣跨敤鍚庣鐢佃剳灞€鍩熺綉 IP 鎴栧叕缃戝湴鍧€锛屼笉鑳戒娇鐢?`localhost` / `127.0.0.1`銆?
## v1.3 Final Demo Acceptance Record

| Step | Operation | Expected result | Result |
|------|-----------|-----------------|--------|
| 1 | Start backend | `python -m uvicorn backend.main:app --host 0.0.0.0 --port 18080` starts successfully | Pending real demo |
| 2 | Start frontend | `npm run dev` starts and browser opens `http://localhost:15173/` | Pending real demo |
| 3 | Power STM32 | Main controller boots, local OLED/home screen is shown | Pending real demo |
| 4 | Power ESP32S3 | ESP32 connects WiFi and uploads heartbeat | Pending real demo |
| 5 | Web ESP32 status | Web shows ESP32 gateway online | Pending real demo |
| 6 | Web STM32 status | Web shows STM32 main controller online | Pending real demo |
| 7 | Web seat status | Web shows seat 1/2/3 status | Pending real demo |
| 8 | Web gate status | Web shows gate state | Pending real demo |
| 9 | Registered card | OLED, buzzer, gate and Web record all show access allowed | Pending real demo |
| 10 | Unregistered card | Web shows denied card event, gate does not open | Pending real demo |
| 11 | Disconnect ESP32 | Web shows ESP32 connection lost within about 10 seconds | Pending real demo |
| 12 | Restore ESP32 | Web returns to ESP32 online | Pending real demo |

Stability checks:

| Check | Expected result | Result |
|-------|-----------------|--------|
| ESP32 PlatformIO build | `pio run` succeeds on a machine with PlatformIO installed | Pending external build machine |
| ESP32 backend URL selection | SSID selects the intended backend URL | Pending real WiFi test |
| Web 10-minute refresh | No console/runtime errors while polling | Pending real demo |
| Backend restart recovery | Web recovers after backend restart | Pending real demo |
| STM32 restart recovery | ESP32 receives new `DEVICE_STATUS` after STM32 reset | Pending real demo |
| Clear Events recovery | Clearing events does not block later card-event uploads | Pending real demo |
