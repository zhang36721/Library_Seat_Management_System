# v0.8.4 涓绘帶鏈湴鑿滃崟鍔熻兘闂幆娴嬭瘯鎶ュ憡

鏈枃妗ｈ褰?STM32 涓绘帶澶栬妯″潡銆乻eat_node_stm32 搴т綅绔噰闆嗘ā鍧椼€佷袱绔?ZigBee 涓插彛閫忎紶閾捐矾锛屼互鍙?v0.8.2 涓绘帶鏈湴鍒峰崱涓庢樉绀洪棴鐜€俇SART2 debug 鍗忚淇濇寔鐙珛锛屼笉鎺ュ悗绔€佷笉鎺ュ墠绔€?

瀹炴満缁撴灉蹇呴』鐢辩儳褰曞悗鐨勭湡瀹炵‖浠舵祴璇曡ˉ鍏咃紱褰撳墠浠ｇ爜渚у凡鎻愪緵鍛戒护銆佹棩蹇楀拰 Keil 缂栬瘧浜х墿銆?

## 1. 鍥轰欢涓庣紪璇?

| 椤圭洰 | 缁撴灉 |
|------|------|
| 涓绘帶鍥轰欢鐗堟湰 | v0.8.4 |
| 搴т綅绔浐浠剁増鏈?| v0.8 |
| 涓绘帶 Debug 涓插彛 | USART2锛?15200锛宍FF CMD DATA FF` |
| 搴т綅绔?Debug 涓插彛 | USART2锛?15200锛宍FF CMD DATA FF` |
| 涓绘帶 ZigBee 涓插彛 | USART1锛?8400 |
| 搴т綅绔?ZigBee 涓插彛 | USART1锛?8400 |
| ESP32S3 娴嬭瘯涓插彛 | 涓绘帶 USART3锛?15200锛汦SP32S3 渚?GPIO47 TX / GPIO48 RX |
| Keil Rebuild All | 涓绘帶 0 Error / 0 Warning锛涘骇浣嶇 0 Error / 0 Warning |

## 2. 涓绘帶涓婄數鑷

涓绘帶 `main.c` 鍦?HAL/CubeMX 鍒濆鍖栥€乁SART2 debug 鎺ユ敹鍚姩涔嬪悗璋冪敤 `kt_modules_init()`銆傝鍏ュ彛鍙仛纭欢鍒濆鍖栧拰鐘舵€佹墦鍗帮紝涓嶆帴涓氬姟閫昏緫銆?

| 妯″潡 | 涓婄數琛屼负 | 棰勬湡鏃ュ織 | 瀹炴満缁撴灉 |
|------|----------|----------|----------|
| RC522 | 鍒濆鍖栬蒋浠?SPI锛屽苟璇诲彇鐗堟湰瀵勫瓨鍣ㄦ帰娴?| `RC522 init: OK, version=0x..` 鎴?`RC522 init: PROBE_FAIL, version=0x..` | 宸插疄娴嬪彲璇?UID锛歚BF A4 A5 1F BCC=A1`锛涙棤鍗℃彁绀?`RC522 no card detected` |
| OLED | 鍒濆鍖?I2C OLED锛屾樉绀?`STM32 MAIN` / `v0.8.2` | `OLED init: OK` 鎴?`OLED init: ACK_FAIL` | 宸插疄娴嬪垵濮嬪寲 OK锛屽睆骞曞彲鏄剧ず |
| DS1302 | 璇诲彇涓€娆℃椂闂村苟鏍￠獙 BCD 杞崲鍚庤寖鍥?| `DS1302 init: OK, time=...` 鎴?`DS1302 init: INVALID_TIME` | 宸插疄娴嬪悎娉曪細`2026-04-28 12:59:36` |
| 姝ヨ繘鐢垫満 | 涓婄數榛樿鍋滄锛屽洓璺嚎鍦堝叧闂?| `Stepper init: STOP, coils off` | 寰呭疄鏈哄～鍐?|
| ZigBee USART1 | 鍚姩 USART1 RX 涓柇缂撳啿 | `USART1 ZigBee UART init: OK`锛屽苟鎻愮ず閾捐矾寰呭疄娴?| 寰呭疄鏈哄～鍐?|
| ESP32S3 USART3 | 纭 USART3 鍒濆鍖栬亴璐?| `USART3 ESP32S3 UART init: OK`锛屽苟鎻愮ず瀵圭寰呭疄娴?| 寰呭疄鏈哄～鍐?|

娉ㄦ剰锛歎SART1/USART3 鐨?`OK` 鍙〃绀?STM32 渚?UART 宸插垵濮嬪寲锛屼笉浠ｈ〃 ZigBee 鍏ョ綉銆佹棤绾块摼璺彲鐢紝鎴?ESP32S3 瀵圭閫氫俊鎴愬姛銆?

## 3. 涓绘帶 USART2 Debug 鍛戒护

| 妯″潡 | 鍛戒护 | 鐢ㄩ€?| 棰勬湡鏃ュ織/鐜拌薄 |
|------|------|------|---------------|
| RC522 | `FF 40 00 FF` | 鍒濆鍖?鎺㈡祴 RC522 | 鎵撳嵃 `OK` 鎴?`PROBE_FAIL` |
| RC522 | `FF 41 00 FF` | 璇诲彇鍗＄墖 UID | 鏈夊崱鏃舵墦鍗?UID |
| OLED | `FF 50 00 FF` | 鏄剧ず娴嬭瘯鏂囧瓧 | OLED 鏄剧ず鍚姩鏂囧瓧锛屾棩蹇?`OLED test text sent` |
| DS1302 | `FF 60 00 FF` | 璇诲彇褰撳墠鏃堕棿 | 鍚堢悊鏃堕棿鎴?`INVALID_TIME` |
| DS1302 | `FF 61 00 FF` | 鍐欏叆娴嬭瘯鏃堕棿 | 鍐欏叆 `2026-04-28 12:34:00` |
| DS1302 | `FF 62 YY FF` | 璁剧疆寰呭啓鍏ュ勾浠?| `YY` 涓?HEX 鏁版嵁锛岃寖鍥?0~99锛岃〃绀?20YY |
| DS1302 | `FF 63 MM FF` | 璁剧疆寰呭啓鍏ユ湀浠?| 鑼冨洿 1~12锛岄潪娉曞€兼嫆缁?|
| DS1302 | `FF 64 DD FF` | 璁剧疆寰呭啓鍏ユ棩鏈?| 鑼冨洿 1~31锛岄潪娉曞€兼嫆缁?|
| DS1302 | `FF 65 hh FF` | 璁剧疆寰呭啓鍏ュ皬鏃?| 鑼冨洿 0~23锛岄潪娉曞€兼嫆缁?|
| DS1302 | `FF 66 mm FF` | 璁剧疆寰呭啓鍏ュ垎閽?| 鑼冨洿 0~59锛岄潪娉曞€兼嫆缁?|
| DS1302 | `FF 67 ss FF` | 璁剧疆寰呭啓鍏ョ | 鑼冨洿 0~59锛岄潪娉曞€兼嫆缁?|
| DS1302 | `FF 68 00 FF` | 鎻愪氦鍐欏叆 DS1302 | 鏍￠獙缂撳瓨鍚堟硶鍚庡啓鍏?|
| DS1302 | `FF 69 00 FF` | 鎵撳嵃寰呭啓鍏ユ椂闂寸紦瀛?| 鍙墦鍗帮紝涓嶅啓鍏?|
| 姝ヨ繘鐢垫満 | `FF 70 00 FF` | 姝ｈ浆娴嬭瘯 | 鐢垫満姝ｈ浆涓€娈靛悗鍋滄 |
| 姝ヨ繘鐢垫満 | `FF 71 00 FF` | 鍙嶈浆娴嬭瘯 | 鐢垫満鍙嶈浆涓€娈靛悗鍋滄 |
| 姝ヨ繘鐢垫満 | `FF 72 00 FF` | 鍋滄骞跺叧闂嚎鍦?| 绾垮湀鍏抽棴锛屼笉鍐嶅姩浣?|
| ZigBee | `FF 80 00 FF` | USART1 鍙戦€佹祴璇曞瓧绗︿覆 | `USART1 ZigBee test TX: OK` |
| ZigBee | `FF 81 00 FF` | 鎵撳嵃 USART1 鏈€杩戞帴鏀剁紦鍐?| `ZigBee recent RX: ...` 鎴栨彁绀烘殏鏃犳暟鎹?|
| ZigBee | `FF 82/86 00 FF` | USART1 鎸?`FA 1E 30 02 01 seq F5` 鍙戦€佷簩杩涘埗 PING 娴嬭瘯甯?| 搴т綅绔敹鍒板悗鍥?`02 seq` PONG |
| ESP32S3 | `FF 90 00 FF` | USART3 鍙戦€佹祴璇曞瓧绗︿覆 | `USART3 ESP32S3 test TX: OK` |
| 涓绘帶鏈湴涓氬姟 | `FF A0 00 FF` | 鎵撳嵃褰撳墠涓绘帶涓氬姟鐘舵€?| 鎵撳嵃妯℃嫙搴т綅鐘舵€佸拰鏈€杩戝埛鍗＄粨鏋?|
| 涓绘帶鏈湴涓氬姟 | `FF A1 00 FF` | 鎵ц涓€娆℃湰鍦板埛鍗℃祦绋?| 鏈夊崱鎵撳嵃 UID/鏃堕棿/浜嬩欢锛孫LED 鏄剧ず鎴愬姛锛涙棤鍗℃樉绀哄け璐?|
| 涓绘帶鏈湴涓氬姟 | `FF A2 00 FF` | OLED 鏄剧ず棣栭〉 | `KENTO LIB`銆佹椂闂淬€丼1/S2/S3 妯℃嫙鐘舵€?|
| 涓绘帶鏈湴涓氬姟 | `FF A3 00 FF` | OLED 鏄剧ず鏈€杩戜竴娆″埛鍗＄粨鏋?| 鏄剧ず鏈€杩戞垚鍔?澶辫触缁撴灉 |
| 涓绘帶鏈湴涓氬姟 | `FF A4 00 FF` | 铚傞福鍣ㄦ垚鍔熸彁绀?| 80ms 鐭搷 |
| 涓绘帶鏈湴涓氬姟 | `FF A5 00 FF` | 铚傞福鍣ㄥけ璐ユ彁绀?| 200ms 鐭搷 |
| 涓绘帶鏈湴涓氬姟 | `FF A6 00 FF` | 妯℃嫙搴т綅鐘舵€佸彉鍖?| S1/S2/S3 妯℃嫙 FREE/OCCUPIED 鍒囨崲 |
| 鎸夐敭 | `FF B0 00 FF` | 鎵撳嵃 8 璺嫭绔嬫寜閿槧灏?| 鎵撳嵃 K1~K8 涓?PA/PB/PC 寮曡剼鍏崇郴 |
| 鎸夐敭 | `FF B1 00 FF` | 鎵撳嵃 K1~K8 鍘熷鐢靛钩 | 渚嬪 `K2 raw=0 pressed=1` |
| 鎸夐敭 | `FF B2 00 FF` | 鎵撳嵃鏈€杩戜竴娆℃寜閿簨浠?| 渚嬪 `Last key: K2 SHORT` |
| 鑿滃崟 | `FF B3 00 FF` | 杩涘叆鏃堕棿璁剧疆鐣岄潰 | OLED 鏄剧ず `TIME SET` |
| 鑿滃崟 | `FF B4 00 FF` | 杩涘叆 IC 鍗℃敞鍐岀晫闈?| OLED 鏄剧ず `CARD ADD` |
| 鑿滃崟 | `FF B5 00 FF` | 杩涘叆 IC 鍗″垹闄ょ晫闈?| OLED 鏄剧ず `CARD DEL` |
| 鏈湴鍗¤〃 | `FF B6 00 FF` | 鎵撳嵃鏈湴宸叉敞鍐屽崱鍒楄〃 | RAM 鍗¤〃鏈€澶?10 寮犲崱 |
| 鏈湴鍗¤〃 | `FF B7 00 FF` | 娓呯┖ RAM 鍗¤〃 | 娓呯┖鍚庢湭娉ㄥ唽鍗″埛鍗′細琚嫆缁?|

## 4. v0.8.2 涓绘帶鏈湴鍒峰崱闂幆

鏈樁娈典笉渚濊禆 ZigBee锛屼笉渚濊禆 ESP32S3锛屼笉鎺ュ悗绔拰鍓嶇銆備富鎺ф湰鍦伴棴鐜祦绋嬩负锛?

`RC522 璇诲崱 UID -> DS1302 璇诲彇褰撳墠鏃堕棿 -> OLED 鏄剧ず缁撴灉 -> 铚傞福鍣ㄦ彁绀?-> USART2 debug 鎵撳嵃浜嬩欢`

鎴愬姛娴佺▼棰勬湡鏃ュ織锛?

```text
[INFO] CMD 0xA1: Local card flow test
[INFO] CARD UID: BF A4 A5 1F
[INFO] TIME: 2026-04-28 12:59:36
[INFO] CARD EVENT: CHECK_IN_TEST OK
```

澶辫触娴佺▼棰勬湡鏃ュ織锛?

```text
[INFO] CMD 0xA1: Local card flow test
[WARN] RC522 no card detected
[WARN] CARD EVENT: NO_CARD
```

OLED 棣栭〉锛?

```text
KENTO LIB
TIME 12:59
S1 FREE
S2 OCC S3 FREE
```

OLED 鍒峰崱鎴愬姛锛?

```text
CARD OK
BFA4A51F
12:59:36
```

OLED 鍒峰崱澶辫触锛?

```text
CARD FAIL
NO CARD
```

Stepper 褰撳墠涓嶄綔涓?v0.8.2 闃诲椤广€傚綋鍓嶈褰曪細`FF 70 / FF 71 / FF 72` 鍛戒护閾捐矾姝ｅ父锛孶LN2003 鍥涗釜鎸囩ず鐏湁鐏晥锛涚數鏈烘湰浣撲笉杞紝鍚庣画澶嶆煡渚涚數銆佺浉搴忋€佺嚎搴忋€侀┍鍔ㄦ澘鍜岀數鏈烘帴鍙ｃ€?

## 5. v0.8.3 8 璺嫭绔嬫寜閿笌 OLED 鑿滃崟

褰撳墠鎸夐敭瀹炵墿涓?8 浣嶇嫭绔嬫寜閿ā鍧楋紝涓嶆槸鐭╅樀閿洏銆傛ā鍧?VCC 鎺?STM32 3.3V锛孏ND 鎺?STM32 GND銆侹1~K8 鍧囬厤缃负 GPIO 杈撳叆涓婃媺锛岄粯璁や綆鐢靛钩鎸変笅銆?

| 鎸夐敭 | STM32 寮曡剼 | 鍔熻兘 |
|------|------------|------|
| K1 | PA1 | MENU / 杩涘叆鑿滃崟 / 闀挎寜杩斿洖 |
| K2 | PA8 | UP / 鏁板€煎姞 |
| K3 | PA11 | DOWN / 鏁板€煎噺 |
| K4 | PA12 | LEFT / 涓婁竴涓瓧娈?|
| K5 | PA15 | RIGHT / 涓嬩竴涓瓧娈?|
| K6 | PB3 | OK / 纭 |
| K7 | PB4 | CARD ADD / 娉ㄥ唽 IC 鍗?|
| K8 | PC14 | CARD DEL / 鍒犻櫎 IC 鍗?|

PA15 / PB3 / PB4 宸查€氳繃鍏抽棴 JTAG銆佷繚鐣?SWD 閲婃斁銆傝嫢 PC14 琚?32.768k 鏅舵尟鍗犵敤锛孠8 鏀圭敤 PC15銆?

OLED 鑿滃崟鍏ュ彛锛?

```text
> CARD TEST
  TIME SET
  CARD ADD
  CARD DEL
```

鎸夐敭浠诲姟鍦ㄤ富寰幆浠诲姟璋冨害涓瘡 10ms 鎵弿涓€娆★紝娑堟姈 20ms锛岄暱鎸夐槇鍊?800ms銆備笉鍦ㄤ腑鏂腑鍋氭寜閿笟鍔°€?

ESP32S3 鎺ョ嚎瑙勫垝鍚屾璋冩暣锛歋TM32 PB10 USART3_TX -> ESP32S3 GPIO48 UART_RX锛汼TM32 PB11 USART3_RX <- ESP32S3 GPIO47 UART_TX锛沀SART2 debug 淇濇寔鐙珛銆?

## 6. v0.8.4 鎸夐敭鑿滃崟鍔熻兘闂幆

v0.8.4 灏嗚彍鍗?placeholder 琛ユ垚鐪熷疄鏈湴鍔熻兘锛屼笉渚濊禆 ZigBee銆佷笉渚濊禆 ESP32S3銆佷笉鎺ュ悗绔拰鍓嶇銆?

| 鍔熻兘 | 鎿嶄綔 | 棰勬湡 OLED | 棰勬湡鏃ュ織 |
|------|------|-----------|----------|
| 鏃堕棿璁剧疆鎻愪氦 | TIME SET 鐣岄潰 K6 | `TIME SAVED` / 鏃ユ湡 / 鏃堕棿 | `TIME SET: OK` |
| 鏃堕棿璁剧疆闈炴硶 | TIME SET 鐣岄潰 K6锛岄潪娉曟椂闂?| `TIME FAIL` / `INVALID` | `TIME SET: INVALID_TIME` |
| 娉ㄥ唽鍗℃垚鍔?| CARD ADD 鐣岄潰 K6 | `CARD ADD` / `OK` / UID | `CARD ADD: OK` |
| 閲嶅娉ㄥ唽 | CARD ADD 鐣岄潰 K6锛屽悓涓€寮犲崱 | `CARD ADD` / `EXISTS` / UID | `CARD ADD: EXISTS` |
| 娉ㄥ唽鏃犲崱 | CARD ADD 鐣岄潰 K6锛屾棤鍗?| `CARD ADD` / `NO CARD` | `CARD ADD: NO_CARD` |
| 鍒犻櫎鍗℃垚鍔?| CARD DEL 鐣岄潰 K6 | `CARD DEL` / `OK` / UID | `CARD DEL: OK` |
| 鍒犻櫎鏈壘鍒?| CARD DEL 鐣岄潰 K6锛屾湭娉ㄥ唽鍗?| `CARD DEL` / `NOT FOUND` / UID | `CARD DEL: NOT_FOUND` |
| 鍒峰崱宸叉敞鍐?| `FF A1 00 FF` | `CARD OK` / UID / 鏃堕棿 | `CARD EVENT: CHECK_IN OK UID=...` |
| 鍒峰崱鏈敞鍐?| `FF A1 00 FF` | `CARD DENIED` / UID | `CARD EVENT: DENIED UID=...` |

鏈湴鍗¤〃褰撳墠涓?RAM 琛紝瀹归噺 10 寮犲崱锛屾柇鐢典笉淇濈暀銆傚悗缁闇€瑕佹寔涔呭寲锛屽啀杩佺Щ鍒?Flash 鎴栧閮ㄥ瓨鍌ㄣ€?

## 7. ZigBee CC2530 閫忎紶娴嬭瘯

鎺ョ嚎锛?

| STM32 涓绘帶 | 鏂瑰悜 | CC2530 ZigBee |
|------------|------|---------------|
| PA9 USART1_TX | -> | P1.7 RX |
| PA10 USART1_RX | <- | P1.6 TX |
| GND | -> | GND |
| 3.3V | -> | VCC |
| GND | -> | P2.0 |

娴嬭瘯璇存槑锛?

- `FF 80 00 FF` 鍙兘璇佹槑 STM32 USART1 鍙戦€佹垚鍔燂紝涓嶈兘璇佹槑 ZigBee 妯″潡鍏ョ綉鎴栧绔敹鍒般€?
- `FF 82/86 00 FF` 鍚戝骇浣嶇 `0x301E` 鍙戦€佸湴鍧€甯э紝涓氬姟鏁版嵁涓轰簩杩涘埗 `01 seq`銆?- 涓绘帶鏀跺埌瀵圭鏁版嵁鏃讹紝鍦ㄤ富寰幆鎵撳嵃 `ZigBee RX: ...`銆?
- `FF 81 00 FF` 鎵撳嵃鏈€杩戜竴鏉?USART1 鎺ユ敹琛屻€?
- ZigBee 妯″潡棰戦亾銆丳AN ID銆佸崗璋冨櫒/缁堢瑙掕壊銆侀€忎紶鍙傛暟闇€瑕佹寜妯″潡璧勬枡鎴栭厤缃伐鍏峰崟鐙‘璁ゃ€?

## 8. seat_node_stm32 v0.8

### v1.3 搴т綅绔帇鍔?閲嶅姏璇嗗埆鏇存柊

| 鍔熻兘 | 褰撳墠瀹炵幇 |
|------|----------|
| 搴т綅妫€娴?1 | HX711 鍘嬪姏/閲嶅姏浼犳劅鍣?DT=PA4銆丼CK=PA5锛屽姞 PB9 绾㈠杈撳叆锛涢噸閲?`> 50g` 涓旂孩澶栬Е鍙戞墠鍒ゅ畾 `OCCUPIED` |
| 搴т綅妫€娴?2 | PB8 GPIO 杈撳叆涓嬫媺锛岄珮鐢靛钩妯℃嫙璇嗗埆鍒版湁閲嶅姏锛屽垽瀹?`OCCUPIED` |
| 搴т綅妫€娴?3 | PB7 GPIO 杈撳叆涓嬫媺锛岄珮鐢靛钩妯℃嫙璇嗗埆鍒版湁閲嶅姏锛屽垽瀹?`OCCUPIED` |
| 鏈氨缁姸鎬?| HX711 鏈帴鎴栨湭 ready 鏃讹紝搴т綅 1 杈撳嚭 `UNKNOWN`锛屽苟鍦?`FF 30 00 FF` 鎵撳嵃 `Seat1 HX711 raw: NOT_READY` |
| 鍏嬮噸杈撳嚭 | `FF 32 00 FF` 绌鸿浇鍘荤毊锛沗FF 33 NN FF` 鐢?`NN*10g` 宸茬煡閲嶉噺鏍囧畾锛沗FF 34 00 FF` 鎵撳嵃浼扮畻閲嶉噺 g |
| 鏍囧畾鎸佷箙鍖?| HX711 offset 鍜?scale 淇濆瓨鍒板骇浣嶇 Flash `0x0800FC00`锛屼笂鐢佃嚜鍔ㄥ姞杞?|
| 榛樿鏍囧畾 fallback | Flash 鏃犲悎娉曟暟鎹椂浣跨敤鍥轰欢榛樿鍙傛暟锛歚offset=-250747`锛宍scale=40667 counts/g x100` |
| 鐘舵€佸彉鍖栨棩蹇?| 涓诲惊鐜瘡 200ms 杞涓€娆★紝杩炵画 3 娆′竴鑷村悗纭鍙樺寲锛涘骇浣嶆潵浜?绂诲紑鏃跺彧鎵撳嵃涓€娆″彉鍖栨棩蹇楋紱鐘舵€佷笉鍙樻椂涓嶅埛灞?|
| ZigBee 鐐瑰鐐逛笂鎶?| 閫氳繃 `FA ADDRL ADDRH LEN DATA F5` 鍦板潃甯у彂閫佷簩杩涘埗 `10 S1 S2 S3` |

寰呭疄鏈洪獙鏀讹細搴т綅 1 闇€瑕佺‘璁?`> 50g` 鍒ゅ畾闃堝€兼槸鍚﹂€傚悎瀹為檯瀹夎缁撴瀯锛屽苟纭 PB9 绾㈠妯″潡鏈夋晥鐢靛钩锛涘骇浣?2/3 鍙仛楂樼數骞虫ā鎷燂紝涓嶆帴绾㈠銆?

搴т綅绔涓€闃舵鍙仛鏈€灏忓姛鑳斤細STM32 鍒濆鍖栥€乁SART2 debug銆乁SART1 ZigBee銆? 璺紶鎰熷櫒杈撳叆銆丗REE/OCCUPIED 鍒ゆ柇鍜屾枃鏈€忎紶銆?

| 鍔熻兘 | 褰撳墠瀹炵幇 |
|------|----------|
| Debug 涓插彛 | USART2锛?15200锛宍FF CMD DATA FF` |
| ZigBee 涓插彛 | USART1锛?8400 |
| 搴т綅妫€娴?1 | PA4/PA5 鎺?HX711锛孭B9 鎺ョ孩澶栵紱閲嶉噺 `> 50g` + 绾㈠鍏卞悓鍒ゆ柇 |
| 搴т綅妫€娴?2 | PB8锛岃緭鍏ヤ笅鎷夛紝楂樼數骞冲垽鏂负 `OCCUPIED` |
| 搴т綅妫€娴?3 | PB7锛岃緭鍏ヤ笅鎷夛紝楂樼數骞冲垽鏂负 `OCCUPIED` |
| Active level | 搴т綅 2/3 涓?`GPIO_PIN_SET`锛涘骇浣?1 绾㈠褰撳墠涓?`GPIO_PIN_SET`锛屽潎鍙€氳繃瀹忚皟鏁?|

搴т綅绔?USART2 debug 鍛戒护锛?

| 鍛戒护 | 鐢ㄩ€?| 棰勬湡鏃ュ織/鏁版嵁 |
|------|------|---------------|
| `FF 20 00 FF` | 鎵撳嵃搴т綅绔‖浠剁姸鎬?| 鐗堟湰銆佷紶鎰熷櫒寮曡剼銆乑igBee 鐘舵€?|
| `FF 24 00 FF` | 鎵撳嵃 USART 瑙掕壊 | USART1 ZigBee锛孶SART2 Debug |
| `FF 30 00 FF` | 璇诲彇 3 璺紶鎰熷櫒鍘熷鐢靛钩 | 鎵撳嵃 HX711銆丳B9銆丳B8銆丳B7 鍘熷鐘舵€?|
| `FF 31 00 FF` | 鎵撳嵃 3 璺?FREE/OCCUPIED 鍒ゆ柇 | 鎵撳嵃 Seat1/Seat2/Seat3 褰撳墠鐘舵€佸拰 `SEAT: ...` |
| `FF 32 00 FF` | HX711 绌鸿浇鍘荤毊 | `HX711 tare OK` |
| `FF 33 NN FF` | HX711 宸茬煡閲嶉噺鏍囧畾锛宍NN*10g`锛屽苟淇濆瓨鍒?Flash | `HX711 calib OK` |
| `FF 34 00 FF` | 鎵撳嵃搴т綅 1 浼扮畻閲嶉噺 | `HX711 weight=... g` |
| `FF 80 00 FF` | 閫氳繃 ZigBee 鍦板潃甯у彂閫佸綋鍓嶅骇浣嶇姸鎬?| 浜岃繘鍒?`10 S1 S2 S3` |
| `FF 81 00 FF` | 鍙戦€?ZigBee PONG 娴嬭瘯甯?| 浜岃繘鍒?`02 seq` |

搴т綅绔敹鍒颁富鎺т簩杩涘埗 `01 seq` PING 鏃讹紝浼氬湪涓诲惊鐜墦鍗?`ZigBee RX PING seq=...` 骞跺洖鍙?`02 seq` PONG銆?
### CC2530 鐐瑰鐐圭煭鍦板潃閫氫俊

| 椤圭洰 | 褰撳墠瀹炵幇 |
|------|----------|
| 甯ф牸寮?| `FA ADDRL ADDRH LEN DATA F5`锛屽湴鍧€浣庡瓧鑺傚湪鍓?|
| 涓绘帶绔湴鍧€ | 鍗忚皟鍣?`0x0000` |
| 搴т綅绔湴鍧€ | 缁堢鑺傜偣 `0x301E` |
| 涓绘帶鍙戦€佺洰鏍?| `ZIGBEE_ADDR_SEAT_NODE = 0x301E`锛屽疄闄呭抚鍦板潃 `1E 30` |
| 搴т綅绔彂閫佺洰鏍?| `ZIGBEE_ADDR_MAIN_CTRL = 0x0000`锛屽疄闄呭抚鍦板潃 `00 00` |
| 閫氱敤椹卞姩 | 涓や釜 STM32 宸ョ▼鍧囨柊澧?`ktlib/kt_zigbee/kt_zigbee.c/.h` |
| 搴т綅鐘舵€佷笂鎶?| 搴т綅绔笂鐢靛彂閫佷竴娆′簩杩涘埗 `10 S1 S2 S3`锛屼箣鍚庡彧鍦ㄧǔ瀹氬彉鍖栨椂涓婃姤 |
| 涓绘帶鎺ユ敹鍏煎 | 鍏煎瀹屾暣 `FA ... F5` 甯у拰绾枃鏈?`SEAT,...` |
| 涓绘帶鍔ㄤ綔 | 瑙ｆ瀽 S1/S2/S3锛屾洿鏂板唴閮ㄥ骇浣嶇姸鎬侊紝骞惰Е鍙?ESP32 `DEVICE_STATUS` 鏇存柊 |

娴嬭瘯鏍蜂緥锛?

```text
涓绘帶 -> 搴т綅绔?hello:
FA 1E 30 05 68 65 6C 6C 6F F5

搴т綅绔?-> 涓绘帶 hello:
FA 00 00 05 68 65 6C 6C 6F F5

搴т綅绔?-> 涓绘帶搴т綅鐘舵€?
FA 00 00 13 53 45 41 54 2C 53 31 3D 30 2C 53 32 3D 31 2C 53 33 3D 30 F5
```

## 9. 寰呭疄鏈哄～鍐欒褰?

| 妯″潡 | 鍛戒护/鍔ㄤ綔 | 瀹炴満鏃ュ織 | 瀹炴満鐜拌薄 | 缁撴灉 |
|------|-----------|----------|----------|------|
| 涓绘帶鍚姩 | 涓婄數 | 寰呭～鍐?| 寰呯‘璁ゅ叏閮ㄨ嚜妫€鏃ュ織 | 寰呴獙璇?|
| RC522 | `FF 40/41 00 FF` | 寰呭～鍐?| 鏈帴鏃?PROBE_FAIL锛屾帴涓婂悗鍙 UID | 寰呴獙璇?|
| OLED | 涓婄數 / `FF 50 00 FF` | 寰呭～鍐?| 鏄剧ず `STM32 MAIN` / `v0.7.1` | 寰呴獙璇?|
| DS1302 | 涓婄數 / `FF 60/61 00 FF` | 寰呭～鍐?| 寮傚父鏃舵姤 INVALID_TIME锛屽啓鍏ュ悗鍙鍚堢悊鏃堕棿 | 寰呴獙璇?|
| 姝ヨ繘鐢垫満 | `FF 70/71/72 00 FF` | 寰呭～鍐?| 姝ｈ浆銆佸弽杞€佸仠姝紝澶嶄綅涓嶄贡鍔?| 寰呴獙璇?|
| 涓绘帶 ZigBee TX | `FF 86 00 FF` | 寰呭～鍐?| 搴т綅绔敹鍒颁簩杩涘埗 `01 seq` | 寰呴獙璇?|
| 涓绘帶 ZigBee RX | `FF 81 00 FF` | 寰呭～鍐?| 鍙墦鍗板骇浣嶇鏈€杩戞暟鎹?| 寰呴獙璇?|
| 搴т綅绔紶鎰熷櫒 | `FF 30/31 00 FF` | 寰呭～鍐?| 鍘熷鐢靛钩鍜?FREE/OCCUPIED 涓庡疄鐗╀竴鑷?| 寰呴獙璇?|
| 搴т綅绔?ZigBee TX | `FF 80/81 00 FF` | 寰呭～鍐?| 涓绘帶鏀跺埌搴т綅鐘舵€佹垨 PONG | 寰呴獙璇?|
| USART2 鐙珛鎬?| 浠绘剰 ZigBee 鏀跺彂鏃舵墽琛?debug 鍛戒护 | 寰呭～鍐?| ZigBee 鏁版嵁涓嶆薄鏌?USART2 debug 鍗忚 | 寰呴獙璇?|
| 涓绘帶鏈湴鍒峰崱鎴愬姛 | `FF A1 00 FF`锛屾湁鍗?| 寰呭～鍐?| OLED 鏄剧ず `CARD OK`銆乁ID銆佹椂闂达紱铚傞福鍣?80ms | 寰呴獙璇?|
| 涓绘帶鏈湴鍒峰崱澶辫触 | `FF A1 00 FF`锛屾棤鍗?| 寰呭～鍐?| OLED 鏄剧ず `CARD FAIL` / `NO CARD`锛涜渹楦ｅ櫒 200ms | 寰呴獙璇?|
| 涓绘帶 OLED 棣栭〉 | `FF A2 00 FF` | 寰呭～鍐?| OLED 鏄剧ず棣栭〉鍜屾ā鎷熷骇浣嶇姸鎬?| 寰呴獙璇?|
| 涓绘帶鏈€杩戝埛鍗＄粨鏋?| `FF A3 00 FF` | 寰呭～鍐?| OLED 鏄剧ず鏈€杩戜竴娆″埛鍗＄粨鏋?| 寰呴獙璇?|
| 鎸夐敭鍘熷鐢靛钩 | `FF B1 00 FF` | 寰呭～鍐?| K1~K8 raw/pressed 涓庡疄鐗╀竴鑷?| 寰呴獙璇?|
| 鎸夐敭鐭寜浜嬩欢 | K1~K8 鐭寜鍚?`FF B2 00 FF` | 寰呭～鍐?| 鍙瘑鍒?`Kx SHORT` | 寰呴獙璇?|
| K1 闀挎寜浜嬩欢 | K1 闀挎寜鍚?`FF B2 00 FF` | 寰呭～鍐?| 鍙瘑鍒?`K1 LONG` | 寰呴獙璇?|
| OLED 鑿滃崟 | K1 / K2 / K3 / K4 / K5 / K6 | 寰呭～鍐?| 鍙繘鍏ヨ彍鍗曘€佷笂涓嬮€夋嫨銆佸垏鎹㈠瓧娈点€佺‘璁?| 寰呴獙璇?|
| IC 鍗¤彍鍗曞叆鍙?| K7 / K8 鎴?`FF B4/B5 00 FF` | 寰呭～鍐?| 鍙繘鍏?`CARD ADD` / `CARD DEL` 椤甸潰 | 寰呴獙璇?|
| 鏃堕棿璁剧疆鍐欏叆 | TIME SET 鐣岄潰 K6 | 寰呭～鍐?| `FF 60 00 FF` 鍙鍥炴柊鏃堕棿 | 寰呴獙璇?|
| IC 鍗℃敞鍐?| CARD ADD 鐣岄潰 K6 | 寰呭～鍐?| 鍗¤〃鍑虹幇 UID锛岄噸澶嶆敞鍐屾彁绀?EXISTS | 寰呴獙璇?|
| IC 鍗″垹闄?| CARD DEL 鐣岄潰 K6 | 寰呭～鍐?| 鍗¤〃鍒犻櫎 UID锛屾湭鎵惧埌鎻愮ず NOT FOUND | 寰呴獙璇?|
| 鍗¤〃鎵撳嵃 | `FF B6 00 FF` | 寰呭～鍐?| 鍙墦鍗版湰鍦板凡娉ㄥ唽鍗″垪琛?| 寰呴獙璇?|
| 鍗¤〃娓呯┖ | `FF B7 00 FF` | 寰呭～鍐?| 鍗¤〃娓呯┖ | 寰呴獙璇?|
| 鍗¤〃鍒峰崱鍒ゆ柇 | `FF A1 00 FF` | 寰呭～鍐?| 鏈敞鍐?DENIED锛屽凡娉ㄥ唽 CARD OK | 寰呴獙璇?|

## 10. 褰撳墠缁撹

褰撳墠闃舵鍙互浣滀负 v0.8.4 鍥轰欢鍊欓€夛細涓绘帶宸插叿澶囨椂闂磋缃啓鍏ャ€丷AM 鍗¤〃娉ㄥ唽/鍒犻櫎銆佸崱琛ㄦ墦鍗?娓呯┖锛屼互鍙婂埛鍗℃祦绋嬫寜鏈湴鍗¤〃鏀捐鎴栨嫆缁濄€傛渶缁堟槸鍚﹀叧闂増鏈紝浠ュ疄鏈鸿彍鍗?K6 鎿嶄綔銆乣FF A1`銆乣FF B6`銆乣FF B7` 楠屾敹缁撴灉涓哄噯銆?

## 11. v0.8.5 涓绘帶鍒峰崱杞銆侀椄闂ㄨ仈鍔ㄤ笌鏈湴閫氳璁板綍

v0.8.5 缁х画淇濇寔涓绘帶鏈湴闂幆锛屼笉渚濊禆 ZigBee銆佷笉渚濊禆 ESP32S3銆佷笉鎺ュ悗绔拰鍓嶇銆?

| 椤圭洰 | 褰撳墠瀹炵幇 |
|------|----------|
| RC522 鑷姩杞 | `main_controller_app_task()` 鍛ㄦ湡杞锛屽懆鏈熺敱 `MAIN_CARD_POLL_PERIOD_MS` 閰嶇疆 |
| 鏃犲崱鏃ュ織 | 鍚庡彴杞浣跨敤闈欓粯 UID 璇诲彇锛岄伩鍏嶆棤鍗℃椂鍒峰睆 |
| 閲嶅鍒峰崱淇濇姢 | 鍗＄墖鏈嬁寮€涓嶉噸澶嶈Е鍙戯紝鍚?UID 杩樻湁 `MAIN_CARD_REPEAT_GUARD_MS` 鑺傛祦 |
| 宸叉敞鍐屽崱 | OLED 鏄剧ず `ACCESS OK`锛岃渹楦ｅ櫒鎴愬姛鎻愮ず锛屽啓鍏ラ€氳璁板綍锛岃Е鍙戦椄闂ㄦ祦绋?|
| 鏈敞鍐屽崱 | OLED 鏄剧ず `ACCESS DENIED`锛岃渹楦ｅ櫒澶辫触鎻愮ず锛屽啓鍏ユ嫆缁濊褰曪紝涓嶈Е鍙戠數鏈?|
| 闂搁棬鍔ㄤ綔 | 姝ヨ繘鐢垫満姝ｈ浆銆佷繚鎸?`MAIN_GATE_HOLD_MS`銆佸弽杞€佸仠姝㈠苟鍏抽棴绾垮湀 |
| 閫氳璁板綍 | Flash 鎸佷箙鍖栫幆褰㈢紦鍐诧紝瀹归噺 `MAIN_ACCESS_LOG_MAX_COUNT`锛岃秴杩囧悗瑕嗙洊鏈€鏃ц褰?|
| 杩涘嚭鍒ゆ柇 | 宸叉敞鍐屽崱鎸?RAM 鐘舵€佷氦鏇胯褰?`CHECK_IN` / `CHECK_OUT` |
| OLED 棣栭〉 | 涓婄數鏄剧ず `KENTO LIBRARY`銆乣SWIPE CARD`銆佸崱琛ㄦ暟閲忋€佹棩蹇楁暟閲忓拰鏃堕棿 |
| 鎵嬪姩鍒峰崱 | `FF A1 00 FF` 淇濈暀涓烘湰鍦板埛鍗℃祴璇曞懡浠?|

鏂板 debug 鍛戒护锛?

| 鍛戒护 | 鐢ㄩ€?| 棰勬湡鏃ュ織 |
|------|------|----------|
| `FF C0 00 FF` | 鎵撳嵃鏈€杩戦€氳璁板綍 | `ACCESS LOG COUNT` 鎴?`ACCESS LOG EMPTY` |
| `FF C1 00 FF` | 娓呯┖鏈€杩戦€氳璁板綍 | `ACCESS LOG CLEARED` |
| `FF C2 00 FF` | 鎵撳嵃閫氳璁板綍缁熻 | 璁板綍鎬绘暟銆丱K 鏁般€丏ENIED 鏁?|

Keil Rebuild All锛歚0 Error(s), 0 Warning(s)`銆?

v0.8.5-fix2 鎸佷箙鍖栦慨澶嶏細

| 椤圭洰 | 褰撳墠瀹炵幇 |
|------|----------|
| 閫氳璁板綍瀛樺偍 | STM32 鍐呴儴 Flash 鍊掓暟绗簩椤?|
| 宸叉敞鍐屽崱琛ㄥ瓨鍌?| 涓庨€氳璁板綍鍏辩敤鍚屼竴椤垫湰鍦?Flash 闀滃儚 |
| Flash 鍦板潃 | `0x0800F800` |
| Boot count 鍦板潃 | `0x0800FC00`锛屼笌閫氳璁板綍鍒嗙 |
| Keil IROM | 宸茬缉灏忓埌 `0x08000000 + 0xF800`锛岄鐣欐渶鍚庝袱椤?|
| 涓婄數鎭㈠ | `main_access_log_init()` 鍚姩鏃跺姞杞?Flash 鍗¤〃鍜岄€氳璁板綍 |
| 鍐欏叆绛栫暐 | 姣忔娉ㄥ唽/鍒犻櫎鍗°€佹瘡娆℃柊澧?娓呯┖閫氳璁板綍鍚庡啓鍥?Flash |

娉ㄦ剰锛歋TM32F103 鍐呴儴 Flash 鏈夋摝鍐欏鍛介檺鍒讹紝鏈柟妗堥€傚悎浣滀负鏈湴楠屾敹鍜屼綆棰戦€氳璁板綍鎸佷箙鍖栥€傚悗缁嫢鍒峰崱棰戠巼杈冮珮锛屽缓璁縼绉诲埌澶栭儴 EEPROM/FRAM 鎴栧仛 Flash 澶氶〉纾ㄦ崯鍧囪　銆?
## 12. v0.9 ESP32S3 Binary UART Link

Scope: STM32 USART3 and ESP32S3 UART binary protocol only. No HTTP, no backend,
and USART2 debug remains independent.

Wiring:

| STM32 | Direction | ESP32S3 |
|-------|-----------|---------|
| PB10 USART3_TX | -> | GPIO48 UART_RX |
| PB11 USART3_RX | <- | GPIO47 UART_TX |
| GND | <-> | GND |

Frame:

```text
A5 5A VER TYPE SEQ_L SEQ_H LEN_L LEN_H PAYLOAD... CRC_L CRC_H 0D
```

CRC: CRC16-Modbus, init `0xFFFF`, polynomial `0xA001`, calculated from `VER`
through payload. The final `0D` tail byte is transmitted after CRC.

STM32 debug commands:

| Command | Purpose |
|---------|---------|
| `FF D0 00 FF` | Send ESP32 binary `PING` |
| `FF D1 00 FF` | Print recent ESP32 received frame |
| `FF D2 00 FF` | Print ESP32 link and WiFi status |
| `FF D3 00 FF` | Send mock `CARD_EVENT` |
| `FF D4 00 FF` | Send latest real `CARD_EVENT` |
| `FF D5 00 FF` | Send a bad CRC test frame |

Current code-side validation:

| Target | Result |
|--------|--------|
| STM32 Keil Rebuild All | `0 Error(s), 0 Warning(s)` |
| ESP32S3 project | Added at `firmware/esp32s3_gateway` |
| ESP32S3 local build | Not run in this environment because PlatformIO is not installed |

## 13. v0.9.1 ESP32 Heartbeat And Link State

v0.9.1 changes the heartbeat direction: ESP32S3 actively sends `HEARTBEAT`
every 3000 ms and STM32 replies with `HEARTBEAT_ACK`.

| Item | Current implementation |
|------|------------------------|
| ESP32 heartbeat | `KT_MSG_HEARTBEAT` every 3000 ms |
| STM32 ACK | `KT_MSG_HEARTBEAT_ACK` with `ack_seq` and `ack_type=HEARTBEAT` |
| ESP32 offline rule | 3 consecutive missed heartbeat ACKs |
| STM32 link LED | PC13 pulses for 50 ms on received heartbeat; no blocking delay |
| STM32 initial sync | First heartbeat or recovered link sends `DEVICE_STATUS` |
| EOF check | Both parsers explicitly require final `0x0D`; `0x0D` is not in CRC |
| Board LED | ESP32 requests GPIO48 low before UART init; GPIO48/RGB conflict pending real-board confirmation |

STM32 `FF D2 00 FF` now prints heartbeat count, last heartbeat tick, recovered
count, EOF errors, WiFi state, SSID/RSSI, and last RX/TX tick.

Code-side validation:

| Target | Result |
|--------|--------|
| STM32 Keil Rebuild All | `0 Error(s), 0 Warning(s)` |
| ESP32S3 PlatformIO build | Not run here because `pio` is not installed |
## v1.2.1 涓婚摼璺姸鎬佸睍绀轰笌涓诲姩涓婃姤楠屾敹

閾捐矾锛歚STM32 -> ESP32S3 -> 鍚庣 -> Web`銆?

| 椤圭洰 | 褰撳墠瀹炵幇 | 楠屾敹缁撴灉 |
|------|----------|----------|
| ESP32 heartbeat | 姣?2 绉?POST `/api/iot/heartbeat`锛屾棩蹇楁墦鍗?OK/FAIL銆丠TTP code銆乫ailed_count | 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| ESP32 device-status | 鏀跺埌 STM32 `DEVICE_STATUS` 鍚庣珛鍗?POST `/api/iot/device-status` | 浠ｇ爜瀹屾垚锛屾帴鍙ｆā鎷熼€氳繃 |
| ESP32 card-event | 鏀跺埌 STM32 `CARD_EVENT` 鍚庣珛鍗?POST `/api/iot/card-event` | 浠ｇ爜瀹屾垚锛屾帴鍙ｆā鎷熼€氳繃 |
| STM32 搴т綅鐘舵€?| `DEVICE_STATUS` 杩藉姞 seat1/seat2/seat3锛涘綋鍓嶄负涓绘帶妯℃嫙鐘舵€?`FREE/OCCUPIED/FREE` | 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| STM32 闂搁棬鐘舵€?| `DEVICE_STATUS` 杩藉姞 gate_state锛歚CLOSED/OPENING/OPEN/CLOSING` | 浠ｇ爜瀹屾垚锛屽疄鏈哄緟纭 |
| 鍚庣鐘舵€?API | `/api/iot/status` 杩斿洖 `esp_online`銆乣stm32_online`銆乣seats`銆乣gate`銆佹渶杩戝績璺虫椂闂?| 妯℃嫙閫氳繃 |
| 鍓嶇鐘舵€佸睍绀?| 涓枃鏄剧ず ESP32銆丼TM32銆佸悗绔湇鍔°€佸骇浣嶃€侀椄闂ㄣ€佸埛鍗¤褰?| 鏋勫缓閫氳繃 |

瀹炴満鍚姩鍛戒护锛?

```bash
python -m uvicorn backend.main:app --host 0.0.0.0 --port 18080
cd frontend
npm run dev -- --host 0.0.0.0 --port 15173
```

娉ㄦ剰锛欵SP32 鐨勫悗绔湴鍧€蹇呴』浣跨敤鍚庣鐢佃剳灞€鍩熺綉 IP 鎴栧叕缃戝湴鍧€锛屼笉鑳戒娇鐢?`localhost` / `127.0.0.1`銆?

## v1.3 Final Demo Stability Checklist

| Item | Expected result | Result |
|------|-----------------|--------|
| STM32 local access flow | Registered card opens gate; unregistered card is denied | Pending real demo |
| ESP32 active upload | Heartbeat/device-status/card-event POSTs return HTTP 200 | Pending real demo |
| Backend status API | `GET /api/iot/status` returns ESP32, STM32, seats, gate and server time | Simulated OK |
| Backend events API | `GET /api/iot/card-events` returns recent card events | Simulated OK |
| Web console | Shows backend service, ESP32, STM32, seats, gate, records and backend time | Build OK |
| ESP32 disconnect detection | Web shows connection lost within about 10 seconds | Pending real demo |
| ESP32 recovery | Web returns online after heartbeat resumes | Pending real demo |
| Backend restart recovery | Web recovers after backend restarts | Pending real demo |
| Clear Events | Clears recent event cache without blocking later uploads | Pending real demo |
| PlatformIO build | ESP32 project compiles with PlatformIO | Pending external build machine |

v1.3 conclusion: code-side integration is ready for final real-hardware
acceptance. Real hardware items above must be filled during the live demo run.
