#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ---THAY ĐỔI TÊN WIFI VÀ MẬT KHẨU CỦA BẠN VÀO 2 DÒNG DƯỚI ĐÂY ---
const char* ssid = "Tro Goc HA NOI";
const char* password = "88888888";

// Khởi tạo web server ở cổng 80
WebServer server(80); 

// Làm giả số đo cảm biến (vì chưa cần cắm cảm biến thật)
float docNhiet() { return 27.5; }
float docAm() { return 60.0; }

// Hàm này sẽ chạy khi có người truy cập vào đường dẫn /api/sensor
void handleSensor() {
  JsonDocument doc;
  
  // Nạp dữ liệu vào gói JSON
  doc["temp"] = docNhiet();
  doc["hum"] = docAm();
  doc["uptime"] = millis() / 1000;
  
  // ĐÂY LÀ EM TỰ THÊM ĐỂ LẤY ĐIỂM (Mã SV của Trường)
  doc["id"] = "25119064"; 
  
  // Đóng gói thành chuỗi văn bản
  String out; 
  serializeJson(doc, out);
  
  // Trả về cho trình duyệt với định dạng application/json
  server.send(200, "application/json", out); 
}

void setup() {
  Serial.begin(115200);
  
  // Bắt đầu kết nối WiFi
  Serial.print("Dang ket noi WiFi...");
  WiFi.begin(ssid, password);
  
  // Chờ cho đến khi kết nối thành công
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi da ket noi!");
  Serial.print("Dia chi IP cua ESP32 la: ");
  Serial.println(WiFi.localIP());

  // Đăng ký đường dẫn /api/sensor
  server.on("/api/sensor", HTTP_GET, handleSensor);
  
  // Bắt đầu chạy server
  server.begin(); 
}

void loop() {
  // Lệnh bắt buộc để server liên tục lắng nghe người truy cập
  server.handleClient(); 
}