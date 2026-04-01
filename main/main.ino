#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// 固定配置
const char* WIFI_SSID = "ChinaNet-zMkE";
const char* WIFI_PASS = "ewxh9eed";
const char* VERSION_URL = "http://127.0.0.1:8888/version.txt";
const char* FIRMWARE_URL = "http://127.0.0.1:8888/app.bin";

#define STM32 Serial
WiFiClient client;

// CRC16 (Modbus)
uint16_t crc16_modbus(const uint8_t* data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }
  return crc;
}

void connectWifi() {
  STM32.printf("[WiFi] Connecting to %s...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    STM32.print(".");
    retry++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    STM32.println("\n[WiFi] Connected!");
    STM32.print("[WiFi] IP: ");
    STM32.println(WiFi.localIP());
  } else {
    STM32.println("\n[WiFi] Connect Failed");
  }
}

void setup() {
  STM32.begin(115200);
  delay(500);
  STM32.println("[ESP8266] Booting...");
  connectWifi();
  STM32.println("READY");
}

void loop() {
  handleCommand();
}

// 命令解析
void handleCommand() {
  if (!STM32.available()) return;

  String cmd = STM32.readStringUntil('\n');
  cmd.trim(); // 删除字符串首尾的空格

  if (cmd == "AT") {
    STM32.println("OK");
  }
  else if (cmd == "AT+GetConfig") {
    STM32.printf("SSID: %s\n", WIFI_SSID);
    STM32.printf("VerURL: %s\n", VERSION_URL);
    STM32.printf("FwURL: %s\n", FIRMWARE_URL);
    STM32.println("OK");
  }
  else if (cmd == "AT+GetNewVersion") {
    sendVersion();
  }
  else if (cmd == "AT+GetBinSize") {
    sendBinSize();
  }
  else if (cmd == "AT+GetBin") {
    sendBinBlocks();
  }
  else {
    STM32.println("ERR_CMD");
  }
}

// 发送版本号
void sendVersion() {
  if (WiFi.status() != WL_CONNECTED) {
    STM32.println("ERR_NO_WIFI");
    return;
  }
  
  HTTPClient http;
  http.begin(client, VERSION_URL);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String version = http.getString();
    version.trim();
    STM32.println(version);
  } else {
    STM32.printf("ERR %d\n", code);
  }
  http.end();
}

// 发送固件大小
void sendBinSize() {
  if (WiFi.status() != WL_CONNECTED) {
    STM32.println("ERR_NO_WIFI");
    return;
  }

  HTTPClient http;
  http.begin(client, FIRMWARE_URL);
  int code = http.sendRequest("HEAD");

  if (code > 0) {
    int size = http.getSize();
    STM32.println(size);
  } else {
    STM32.printf("ERR %d\n", code);
  }
  http.end();
}

// 分块发送固件
void sendBinBlocks() {
  if (WiFi.status() != WL_CONNECTED) {
    STM32.println("ERR_NO_WIFI");
    return;
  }

  const int BLOCK_SIZE = 1024;
  uint32_t offset = 0;
  uint32_t total = 0;

  // 获取固件大小
  HTTPClient headHttp;
  headHttp.begin(client, FIRMWARE_URL);
  int headCode = headHttp.sendRequest("HEAD");
  if (headCode > 0)
    total = headHttp.getSize();
  else {
    STM32.println("ERR_SIZE");
    headHttp.end();
    return;
  }
  headHttp.end();

  while (offset < total) {
    uint32_t endByte = offset + BLOCK_SIZE - 1;
    if (endByte >= total) endByte = total - 1;

    HTTPClient http;
    WiFiClient blockClient;
    char rangeHeader[64];
    sprintf(rangeHeader, "bytes=%lu-%lu", offset, endByte);

    http.begin(blockClient, FIRMWARE_URL);
    http.addHeader("Range", rangeHeader); // 添加头是为了告诉服务器只返回文件的第0到1023字节
    int code = http.GET();

    if (code == HTTP_CODE_PARTIAL_CONTENT || code == HTTP_CODE_OK) {
      WiFiClient* stream = http.getStreamPtr();
      uint8_t buffer[BLOCK_SIZE];
      int len = http.getSize();
      int n = stream->readBytes(buffer, len);

      // 构造协议包：AA lenL lenH data... crcL crcH 55
      uint8_t packet[BLOCK_SIZE + 6];
      packet[0] = 0xAA;
      packet[1] = n & 0xFF;
      packet[2] = (n >> 8) & 0xFF;
      memcpy(&packet[3], buffer, n);
      uint16_t crc = crc16_modbus(buffer, n);
      packet[3 + n] = crc & 0xFF;
      packet[4 + n] = (crc >> 8) & 0xFF;
      packet[5 + n] = 0x55;

      STM32.write(packet, n + 6);
      STM32.flush();

      // 等 STM32 回复 OK
      unsigned long t0 = millis();
      bool ok = false;
      String resp = "";
      while (millis() - t0 < 3000) {
        while (STM32.available()) {
          char c = STM32.read();
          resp += c;
          if (resp.endsWith("OK") || resp.indexOf("OK") != -1) {
            ok = true;
            break;
          }
        }
        if (ok) {
          delay(10);
          while(STM32.available()) { STM32.read(); }
          break;
        }
        delay(1);
      }

      if (!ok) {
        STM32.println("ERR_TIMEOUT");
        http.end();
        return;
      }

      offset += n;
    } else {
      STM32.printf("ERR_HTTP %d\n", code);
      http.end();
      return;
    }

    http.end();
  }

  STM32.println("END");
}
