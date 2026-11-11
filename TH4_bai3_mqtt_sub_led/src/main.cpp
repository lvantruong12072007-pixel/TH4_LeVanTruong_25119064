#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- THAY ĐỔI WIFI Ở ĐÂY ---
const char* ssid = "Tro Goc HA NOI";
const char* password = "88888888";

// Cấu hình MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Định nghĩa Topic RIÊNG của Trường (Mã SV: 25119064)
const char* TOPIC_TELE = "aet2025/25119064/telemetry"; // Chiều đi: Gửi số đo
const char* TOPIC_CMD = "aet2025/25119064/cmd/led";     // Chiều về: Nhận lệnh bật/tắt LED

const int LED_PIN = 14; // Chân cắm đèn LED

WiFiClient espClient;
PubSubClient mqtt(espClient);
unsigned long thoi_gian_truoc = 0;

float docNhiet() { return 28.5; }
float docAm() { return 65.0; }

// --- HÀM MỚI: Lắng nghe và xử lý lệnh bật tắt LED ---
void nhanDuLieu(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Nhan duoc lenh tu topic: ");
  Serial.print(topic);
  Serial.print(" - Noi dung: ");
  Serial.println(message);

  // Nếu nhận được chữ "1" thì bật LED, "0" thì tắt LED
  if (String(topic) == String(TOPIC_CMD)) {
    if (message == "1") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("-> DA BAT LED");
    } else if (message == "0") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("-> DA TAT LED");
    }
  }
}

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
  while (!mqtt.connected()) {
    Serial.print("Dang ket noi MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" Thanh cong!");
      // QUAN TRỌNG: Khi kết nối lại thành công, phải Đăng ký lắng nghe Topic Lệnh
      mqtt.subscribe(TOPIC_CMD); 
    } else {
      Serial.print(" That bai. Thu lai sau 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Tắt LED ban đầu
  
  setup_wifi();
  mqtt.setServer(mqtt_server, mqtt_port);
  
  // Khai báo hàm xử lý khi nhận được dữ liệu (Chiều về)
  mqtt.setCallback(nhanDuLieu); 
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  // Chiều đi: Gửi số liệu mỗi 2 giây
  unsigned long thoi_gian_nay = millis();
  if (thoi_gian_nay - thoi_gian_truoc >= 2000) {
    thoi_gian_truoc = thoi_gian_nay;
    JsonDocument doc;
    doc["temp"] = docNhiet();
    doc["hum"] = docAm();
    doc["uptime"] = millis() / 1000;
    doc["rssi"] = WiFi.RSSI();

    char buffer[128];
    serializeJson(doc, buffer);
    mqtt.publish(TOPIC_TELE, buffer);
  }
}