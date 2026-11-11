#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- THAY ĐỔI WIFI Ở ĐÂY ---
const char* ssid = "Tro Goc HA NOI";
const char* password = "88888888";

// Cấu hình MQTT Broker (Dùng broker công cộng miễn phí)
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Định nghĩa Topic riêng của Trường (Mã SV: 25119064)
const char* TOPIC_TELE = "aet2025/25119064/telemetry";

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long thoi_gian_truoc = 0;

// Giả lập cảm biến
float docNhiet() { return 28.5; }
float docAm() { return 65.0; }

void setup_wifi() {
  Serial.print("Dang ket noi WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi da ket noi!");
}

void reconnect() {
  // Vòng lặp chờ kết nối lại MQTT
  while (!mqtt.connected()) {
    Serial.print("Dang ket noi MQTT Broker...");
    // Tạo client ID ngẫu nhiên để không bị trùng lặp với các kit khác
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" Da ket noi MQTT!");
    } else {
      Serial.print(" That bai, ma loi=");
      Serial.print(mqtt.state());
      Serial.println(" Thu lai sau 5 giay");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  
  // Cài đặt server MQTT
  mqtt.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Nếu rớt mạng MQTT thì gọi hàm nối lại
  if (!mqtt.connected()) {
    reconnect();
  }
  
  // Hàm BẮT BUỘC phải gọi thường xuyên để duy trì kết nối
  mqtt.loop();

  // Cứ mỗi 2000 mili-giây (2 giây) thì gửi dữ liệu 1 lần
  unsigned long thoi_gian_nay = millis();
  if (thoi_gian_nay - thoi_gian_truoc >= 2000) {
    thoi_gian_truoc = thoi_gian_nay;

    JsonDocument doc;
    doc["temp"] = docNhiet();
    doc["hum"] = docAm();
    doc["uptime"] = millis() / 1000;
    
    // Yêu cầu bài 2: Thêm một trường tùy biến (Mình lấy RSSI - độ mạnh yếu của sóng WiFi)
    doc["rssi"] = WiFi.RSSI();

    char buffer[128];
    serializeJson(doc, buffer);

    // Bắn dữ liệu (Publish) lên Topic
    mqtt.publish(TOPIC_TELE, buffer);
    
    Serial.print("Da gui du lieu len topic ");
    Serial.print(TOPIC_TELE);
    Serial.print(": ");
    Serial.println(buffer);
  }
}