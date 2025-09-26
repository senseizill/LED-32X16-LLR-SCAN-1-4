#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <PxMatrix.h>

#ifdef ESP32
#define P_LAT 22
#define P_A   19
#define P_B   23
#define P_C   18
#define P_OE  15
#endif

// Panel size
#define matrix_width 32
#define matrix_height 16
uint8_t display_draw_time = 10;
PxMATRIX display(matrix_width, matrix_height, P_LAT, P_OE, P_A, P_B, P_C);

// Web server
WebServer server(80);

// Timer for PxMatrix refresh
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// ====== Font 10x14 (doubled from 5x7 for larger and bolder text) ======
// Each pixel in 5x7 is scaled to a 2x2 block to make it larger and thicker
const byte font5x7[][40] PROGMEM = {
  // 32 ' '
  {0x00,0x00,0x00,0x00,0x00},
  // 33 '!'
  {0x00,0x00,0x5F,0x00,0x00},
  // 34 '"'
  {0x00,0x07,0x00,0x07,0x00},
  // 35 '#'
  {0x14,0x7F,0x14,0x7F,0x14},
  // 36 '$'
  {0x24,0x2A,0x7F,0x2A,0x12},
  // 37 '%'
  {0x23,0x13,0x08,0x64,0x62},
  // 38 '&'
  {0x36,0x49,0x55,0x22,0x50},
  // 39 '''
  {0x00,0x05,0x03,0x00,0x00},
  // 40 '('
  {0x00,0x1C,0x22,0x41,0x00},
  // 41 ')'
  {0x00,0x41,0x22,0x1C,0x00},
  // 42 '*'
  {0x14,0x08,0x3E,0x08,0x14},
  // 43 '+'
  {0x08,0x08,0x3E,0x08,0x08},
  // 44 ','
  {0x00,0x50,0x30,0x00,0x00},
  // 45 '-'
  {0x08,0x08,0x08,0x08,0x08},
  // 46 '.'
  {0x00,0x60,0x60,0x00,0x00},
  // 47 '/'
  {0x20,0x10,0x08,0x04,0x02},
  // 48 '0'
  {0x3E,0x45,0x49,0x51,0x3E},
  // 49 '1'
  {0x00,0x41,0x7F,0x40,0x00},
  // 50 '2'
  {0x42,0x61,0x51,0x49,0x46},
  // 51 '3'
  {0x21,0x41,0x45,0x4B,0x31},
  // 52 '4'
  {0x18,0x14,0x12,0x7F,0x10},
  // 53 '5'
  {0x27,0x45,0x45,0x45,0x39},
  // 54 '6'
  {0x3C,0x4A,0x49,0x49,0x30},
  // 55 '7'
  {0x01,0x71,0x09,0x05,0x03},
  // 56 '8'
  {0x36,0x49,0x49,0x49,0x36},
  // 57 '9'
  {0x06,0x49,0x49,0x29,0x1E},
  // 58 ':'
  {0x00,0x36,0x36,0x00,0x00},
  // 59 ';'
  {0x00,0x56,0x36,0x00,0x00},
  // 60 '<'
  {0x08,0x14,0x22,0x41,0x00},
  // 61 '='
  {0x14,0x14,0x14,0x14,0x14},
  // 62 '>'
  {0x00,0x41,0x22,0x14,0x08},
  // 63 '?'
  {0x02,0x01,0x51,0x09,0x06},
  // 64 '@'
  {0x32,0x49,0x79,0x41,0x3E},
  // 65 'A'
  {0x7E,0x11,0x11,0x7E,0x00},
  // 66 'B'
  {0x7F,0x49,0x49,0x36,0x00},
  // 67 'C'
  {0x3E,0x41,0x41,0x22,0x00},
  // 68 'D'
  {0x7F,0x41,0x41,0x3E,0x00},
  // 69 'E'
  {0x7F,0x49,0x49,0x41,0x00},
  // 70 'F'
  {0x7F,0x09,0x09,0x01,0x00},
  // 71 'G'
  {0x3E,0x41,0x51,0x32,0x00},
  // 72 'H'
  {0x7F,0x08,0x08,0x7F,0x00},
  // 73 'I'
  {0x41,0x7F,0x41,0x00,0x00},
  // 74 'J'
  {0x20,0x40,0x41,0x3F,0x00},
  // 75 'K'
  {0x7F,0x08,0x14,0x63,0x00},
  // 76 'L'
  {0x7F,0x40,0x40,0x40,0x00},
  // 77 'M'
  {0x7F,0x06,0x06,0x7F,0x00},
  // 78 'N'
  {0x7F,0x06,0x18,0x7F,0x00},
  // 79 'O'
  {0x3E,0x41,0x41,0x3E,0x00},
  // 80 'P'
  {0x7F,0x09,0x09,0x06,0x00},
  // 81 'Q'
  {0x3E,0x41,0x61,0x7E,0x00},
  // 82 'R'
  {0x7F,0x09,0x19,0x66,0x00},
  // 83 'S'
  {0x00,0x01,0x7F,0x01,0x00},
  // 84 'T'
 {0x01,0x01,0x7F,0x01,0x01},
  // 85 'U'
  {0x3F,0x40,0x40,0x3F,0x00},
  // 86 'V'
  {0x1F,0x20,0x40,0x3F,0x00},
  // 87 'W'
  {0x7F,0x30,0x30,0x7F,0x00},
  // 88 'X'
  {0x63,0x14,0x08,0x14,0x63},
  // 89 'Y'
  {0x07,0x08,0x70,0x08,0x07},
  // 90 'Z'
  {0x61,0x51,0x49,0x45,0x43},
  // 91 '['
  {0x00,0x7F,0x41,0x41,0x00},
  // 92 '\'
  {0x02,0x04,0x08,0x10,0x20},
  // 93 ']'
  {0x00,0x41,0x41,0x7F,0x00},
  // 94 '^'
  {0x04,0x02,0x01,0x02,0x04},
  // 95 '_'
  {0x40,0x40,0x40,0x40,0x40}
};

// ====== Remap hàng cho panel LLR nếu cần (giữ nguyên) ======
int remapY(int y) {
  int mapTable[16] = {
    4,5,6,7,  0,1,2,3,  12,13,14,15,  8,9,10,11
  };
  return mapTable[y % matrix_height];
}

void drawPixelRemap(int x, int y, uint16_t color) {
  display.drawPixel(x, remapY(y), color);
}

// ====== Hàm vẽ ký tự và chuỗi theo font10x14, dùng remap hàng ======
void drawCharRemap(int x, int y, char c, uint16_t color, int scale=2) {
  if (c < 32 || c > 95) return;   // hỗ trợ từ ' ' (32) tới '_' (95)
  const byte* bitmap = font5x7[c - 32];  // Lấy theo bảng font
  for (int col = 0; col < 5; col++) {
    byte line = pgm_read_byte(&bitmap[col]);
    for (int row = 0; row < 7; row++) {
      if (line & (1 << row)) {
        // vẽ khối scale x scale thay vì 1 điểm
        for (int dx = 0; dx < scale; dx++) {
          for (int dy = 0; dy < scale; dy++) {
            drawPixelRemap(x + col*scale + dx, y + row*scale + dy, color);
          }
        }
      }
    }
  }
}
void drawTextRemap(int x, int y, const char* text, uint16_t color, int scale=2) {
  while (*text) {
    if (*text == ' ') {
      x += 3 * scale;   // space ngắn hơn
    } else {
      drawCharRemap(x, y, *text, color, scale);
      x += (5 * scale + 1); // 5 cột font + khoảng cách
    }
    text++;
  }
}

// ====== PxMatrix ISR refresh (dùng timer hardware) ======
void IRAM_ATTR display_updater(){
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable){
  if (is_enable){
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  } else {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
}

// ====== Scrolling non-blocking state ======
String currentMessage = "HELLO";
int16_t scrollPos = matrix_width;
unsigned long lastScrollMillis = 0;
int scrollSpeed = 120; // ms per pixel step
uint16_t scrollColor = display.color565(255,0,255); // default

// ====== Web UI (HTML + JS) ======
const char* html_page = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 LED Matrix</title>
<style>
body{font-family:Arial;text-align:center;padding:18px}
input[type=text]{width:80%;padding:10px;font-size:16px}
label{display:inline-block;margin:8px}
button{padding:10px 16px;font-size:16px}
.range{width:80%}
.colorBox{display:inline-block;width:20px;height:20px;border:1px solid #ccc;vertical-align:middle;margin-left:8px}
</style>
</head>
<body>
<h2>Gửi chữ lên LED Matrix</h2>
<input type="text" id="msg" placeholder="Nhập tin nhắn..." /><br><br>

<label>R <input id="r" type="range" min="0" max="255" value="255" class="range"></label><br>
<label>G <input id="g" type="range" min="0" max="255" value="0" class="range"></label><br>
<label>B <input id="b" type="range" min="0" max="255" value="255" class="range"></label><br><br>

<label>Tốc độ (ms per step) <input id="speed" type="number" min="20" max="1000" value="120" style="width:100px"></label><br><br>

<button onclick="send()">Gửi lên LED</button>
<p id="status"></p>

<script>
function send(){
  let msg = encodeURIComponent(document.getElementById('msg').value);
  let r = document.getElementById('r').value;
  let g = document.getElementById('g').value;
  let b = document.getElementById('b').value;
  let speed = document.getElementById('speed').value;
  if(msg === ''){ document.getElementById('status').innerText='Nhập tin nhắn!'; return; }
  fetch('/send?msg='+msg+'&r='+r+'&g='+g+'&b='+b+'&speed='+speed)
    .then(resp=>resp.text()).then(txt=>{
      document.getElementById('status').innerText = txt;
      document.getElementById('msg').value = '';
    }).catch(err=>{ document.getElementById('status').innerText='Lỗi gửi'; });
}
</script>
</body>
</html>
)rawliteral";

// ====== Web handlers ======
void handleRoot(){
  server.send(200, "text/html; charset=UTF-8", html_page);
}

void handleSend(){
  if (server.hasArg("msg")) {
    String msg = server.arg("msg");
    msg.trim();
    if (msg.length() == 0) {
      server.send(200, "text/plain", "Tin nhắn rỗng");
      return;
    }

    currentMessage = msg;

    // đọc màu
    int r = server.hasArg("r") ? server.arg("r").toInt() : 255;
    int g = server.hasArg("g") ? server.arg("g").toInt() : 0;
    int b = server.hasArg("b") ? server.arg("b").toInt() : 255;
    scrollColor = display.color565(r, g, b);

    // đọc tốc độ
    scrollSpeed = server.hasArg("speed") ? constrain(server.arg("speed").toInt(), 20, 1000) : scrollSpeed;

    // reset lại vị trí chạy
    scrollPos = matrix_width;

    // ============ Monitor Serial ============
    Serial.println("=== Nhận lệnh mới từ Web ===");
    Serial.print("Tin nhắn: "); Serial.println(currentMessage);
    Serial.printf("Màu (R,G,B): (%d, %d, %d)\n", r, g, b);
    Serial.print("Tốc độ (ms/step): "); Serial.println(scrollSpeed);
    Serial.println("==============================");

    server.send(200, "text/plain", "Đã nhận: " + currentMessage);
  } else {
    server.send(400, "text/plain", "Thiếu tham số msg");
  }
}
void handleNotFound(){
  server.send(404, "text/plain", "Not found");
}

// ====== Setup ======
void setup(){
  Serial.begin(115200);
  delay(100);

  // WiFiManager: tạo AP nếu không có WiFi đã lưu
  WiFiManager wm;
  wm.autoConnect("ESP32-LED", "12345678");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // start webserver
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.onNotFound(handleNotFound);
  server.begin();

  // PxMatrix init
  display.begin(4); // scan 1/4
  display.setTextWrap(false);
  display.setFastUpdate(true);
  display_update_enable(true);
  display.clearDisplay();
}

// ====== Non-blocking scroll update called from loop() ======
void updateScroll() {
  if (currentMessage.length() == 0) return;

  unsigned long now = millis();
  if (now - lastScrollMillis < (unsigned long)scrollSpeed) return;
  lastScrollMillis = now;

  int textWidth = (int)currentMessage.length() * 12; // 10px + 2px gap per character
  display.clearDisplay();
  drawTextRemap(scrollPos, 1, currentMessage.c_str(), scrollColor); // Adjusted y to 1 for vertical centering
  display.showBuffer();

  scrollPos--;
  if (scrollPos < -textWidth) {
    // loop — start again from right edge
    scrollPos = matrix_width;
    Serial.println("HELLO");
  }
}

// ====== Loop ======
void loop(){
  server.handleClient();   // must be called frequently
  updateScroll();
}