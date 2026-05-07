#include "http_server.h"
#include "device_state.h"
#include <Arduino.h>
#include <WebServer.h>

static WebServer server(80);

static String json_escape(const String &value)
{
    String out;
    out.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); ++i) {
        char c = value[i];
        if (c == '\\' || c == '"') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else {
            out += c;
        }
    }
    return out;
}

static void send_cors()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

static void send_json(const String &body)
{
    send_cors();
    server.send(200, "application/json", body);
}

static void handle_options()
{
    send_cors();
    server.send(204, "text/plain", "");
}

static void handle_status()
{
    DeviceStatus s = device_state_get_status();
    String body = "{";
    body += "\"device\":\"kento-main-controller\",";
    body += "\"stm32_online\":";
    body += (s.stm32_online ? "true" : "false");
    body += ",\"wifi_connected\":";
    body += (s.wifi_connected ? "true" : "false");
    body += ",\"ssid\":\"";
    body += json_escape(s.ssid);
    body += "\",\"ip\":\"";
    body += json_escape(s.ip);
    body += "\",\"rssi\":";
    body += String(s.rssi);
    body += ",\"card_count\":";
    body += String(s.card_count);
    body += ",\"log_count\":";
    body += String(s.log_count);
    body += ",\"ds1302_valid\":";
    body += (s.ds1302_valid ? "true" : "false");
    body += ",\"current_time\":\"";
    body += json_escape(s.current_time);
    body += "\",\"heartbeat_count\":";
    body += String(s.heartbeat_count);
    body += ",\"last_heartbeat_ms\":";
    body += String(s.last_heartbeat_ms);
    body += ",\"last_card_event_ms\":";
    body += String(s.last_card_event_ms);
    body += ",\"device_status\":{\"seats\":[";
    for (uint8_t i = 0; i < 3; ++i) {
        if (i > 0) body += ",";
        body += "{\"id\":";
        body += String(i + 1);
        body += ",\"state\":\"";
        body += json_escape(s.seats[i]);
        body += "\"}";
    }
    body += "],\"gate\":{\"state\":\"";
    body += json_escape(s.gate_state);
    body += "\"}}";
    body += "}";
    send_json(body);
}

static void handle_card_events()
{
    size_t count = device_state_card_event_count();
    String body = "{";
    body += "\"count\":";
    body += String(count);
    body += ",\"events\":[";
    for (size_t i = 0; i < count; ++i) {
        CardEvent event = device_state_get_card_event(i);
        if (i > 0) {
            body += ",";
        }
        body += "{";
        body += "\"uid\":\"";
        body += json_escape(event.uid);
        body += "\",\"type\":\"";
        body += json_escape(event.type);
        body += "\",\"allowed\":";
        body += (event.allowed ? "true" : "false");
        body += ",\"time\":\"";
        body += json_escape(event.time);
        body += "\",\"received_ms\":";
        body += String(event.received_ms);
        body += "}";
    }
    body += "]}";
    send_json(body);
}

static void handle_health()
{
    DeviceStatus s = device_state_get_status();
    String body = "{";
    body += "\"ok\":true,";
    body += "\"uptime_ms\":";
    body += String(millis());
    body += ",\"stm32_online\":";
    body += (s.stm32_online ? "true" : "false");
    body += ",\"wifi_connected\":";
    body += (s.wifi_connected ? "true" : "false");
    body += "}";
    send_json(body);
}

static void handle_clear_events()
{
    device_state_clear_card_events();
    send_json("{\"ok\":true}");
}

static void handle_not_found()
{
    send_cors();
    server.send(404, "application/json", "{\"ok\":false,\"error\":\"not_found\"}");
}

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Kento Library Seat System</title>
<style>
:root{font-family:Arial,Helvetica,sans-serif;color:#17202a;background:#f5f7fa}
body{margin:0}
header{padding:18px 20px;background:#1f2937;color:#fff}
h1{margin:0;font-size:22px;font-weight:700}
main{max-width:980px;margin:0 auto;padding:18px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}
.card{background:#fff;border:1px solid #d9e0e8;border-radius:8px;padding:14px}
.label{font-size:12px;color:#667085;text-transform:uppercase}
.value{margin-top:8px;font-size:20px;font-weight:700}
.online{color:#16794c}.offline{color:#b42318}
.toolbar{display:flex;gap:10px;margin:16px 0;flex-wrap:wrap}
button{border:1px solid #b8c2cc;background:#fff;border-radius:6px;padding:9px 12px;font-size:14px;cursor:pointer}
button.primary{background:#2563eb;color:#fff;border-color:#2563eb}
table{width:100%;border-collapse:collapse;background:#fff;border:1px solid #d9e0e8;border-radius:8px;overflow:hidden}
th,td{text-align:left;padding:10px;border-bottom:1px solid #edf1f5;font-size:14px}
th{background:#eef2f7;color:#374151}
tr:last-child td{border-bottom:0}
.ok{color:#16794c;font-weight:700}.deny{color:#b42318;font-weight:700}
.muted{color:#667085}
</style>
</head>
<body>
<header><h1>Kento Library Seat System</h1></header>
<main>
<div class="grid">
<section class="card"><div class="label">STM32</div><div id="stm32" class="value muted">Loading</div></section>
<section class="card"><div class="label">WiFi</div><div id="wifi" class="value muted">Loading</div><div id="wifiDetail" class="muted"></div></section>
<section class="card"><div class="label">Cards</div><div id="cards" class="value">0</div></section>
<section class="card"><div class="label">Logs</div><div id="logs" class="value">0</div></section>
<section class="card"><div class="label">DS1302 Time</div><div id="time" class="value muted">--</div></section>
</div>
<div class="toolbar">
<button class="primary" onclick="refreshAll()">Refresh</button>
<button onclick="clearEvents()">Clear Events</button>
</div>
<table>
<thead><tr><th>Time</th><th>UID</th><th>Type</th><th>Result</th></tr></thead>
<tbody id="events"><tr><td colspan="4" class="muted">Loading</td></tr></tbody>
</table>
</main>
<script>
async function getJson(url){const r=await fetch(url,{cache:'no-store'});if(!r.ok)throw new Error(url);return r.json();}
function setText(id,text){document.getElementById(id).textContent=text;}
async function loadStatus(){
  try{
    const s=await getJson('/api/status');
    const stm32=document.getElementById('stm32');
    stm32.textContent=s.stm32_online?'Online':'Offline';
    stm32.className='value '+(s.stm32_online?'online':'offline');
    const wifi=document.getElementById('wifi');
    wifi.textContent=s.wifi_connected?s.ssid:'Offline';
    wifi.className='value '+(s.wifi_connected?'online':'offline');
    setText('wifiDetail',(s.ip||'0.0.0.0')+' / RSSI '+s.rssi);
    setText('cards',s.card_count);
    setText('logs',s.log_count);
    setText('time',s.ds1302_valid?s.current_time:'Invalid');
  }catch(e){
    setText('stm32','Offline');
    document.getElementById('stm32').className='value offline';
  }
}
async function loadEvents(){
  const body=document.getElementById('events');
  try{
    const data=await getJson('/api/card-events');
    if(!data.events.length){body.innerHTML='<tr><td colspan="4" class="muted">No card events</td></tr>';return;}
    body.innerHTML=data.events.slice().reverse().map(e=>'<tr><td>'+e.time+'</td><td>'+e.uid+'</td><td>'+e.type+'</td><td class="'+(e.allowed?'ok':'deny')+'">'+(e.allowed?'OK':'DENIED')+'</td></tr>').join('');
  }catch(e){body.innerHTML='<tr><td colspan="4" class="muted">Load failed</td></tr>';}
}
async function refreshAll(){await Promise.all([loadStatus(),loadEvents()]);}
async function clearEvents(){await fetch('/api/clear-events',{method:'POST'});await loadEvents();}
refreshAll();
setInterval(refreshAll,2000);
</script>
</body>
</html>
)HTML";

static void handle_index()
{
    server.send_P(200, "text/html", INDEX_HTML);
}

void http_server_begin()
{
    server.on("/", HTTP_GET, handle_index);
    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/status", HTTP_OPTIONS, handle_options);
    server.on("/api/card-events", HTTP_GET, handle_card_events);
    server.on("/api/card-events", HTTP_OPTIONS, handle_options);
    server.on("/api/health", HTTP_GET, handle_health);
    server.on("/api/health", HTTP_OPTIONS, handle_options);
    server.on("/api/clear-events", HTTP_POST, handle_clear_events);
    server.on("/api/clear-events", HTTP_OPTIONS, handle_options);
    server.onNotFound(handle_not_found);
    server.begin();
    Serial.println("[HTTP] server started on port 80");
}

void http_server_task()
{
    server.handleClient();
}
