#include <DHT.h>

#define DHTPIN 2     // Chân data của DHT11 nối với chân D2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Baudrate phải khớp với Node.js
  dht.begin();
}

void loop() {
  // Đọc độ ẩm và nhiệt độ
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Kiểm tra lỗi
  if (isnan(h) || isnan(t)) {
    // Serial.println("Lỗi đọc DHT11!"); // Không gửi dòng lỗi để tránh sai format JSON
    return;
  }

  // Gửi dữ liệu dạng JSON String
  // Format: {"temp": <giá trị>, "hum": <giá trị>}
  Serial.print("{\"temp\":");
  Serial.print(t);
  Serial.print(",\"hum\":");
  Serial.print(h);
  Serial.println("}"); // Xuống dòng để báo hiệu kết thúc gói tin

  delay(2000); // DHT11 cần khoảng 2s để ổn định giữa các lần đọc
}