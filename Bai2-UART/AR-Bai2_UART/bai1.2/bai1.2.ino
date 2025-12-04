#define BUTTON_PIN 2
#define LED_PIN 13

bool configMode = false;  // false: vận hành, true: cấu hình
int baudRate = 9600;      // mặc định
unsigned long pressStart = 0;     // lưu thời điểm bắt đầu nhấn nút
bool buttonState = false;         // phát hiện sườn nhấn
bool lastButtonState = false;     // phát hiện sườn nhả
unsigned long lastPressTime = 0;  // nhớ lần nhấn cuối cùng
int pressCount = 0;       // đếm số lần nhấn nhanh trong chế độ cấu hình (1/2)

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN, INPUT_PULLUP); // nút nhấn kéo lên
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(baudRate);
  Serial.println("Che do van hanh");
}

void loop() {
  // put your main code here, to run repeatedly:
  bool reading = digitalRead(BUTTON_PIN);

  // Phát hiện nhấn giữ
  if (reading == LOW && lastButtonState == HIGH) 
  {
    pressStart = millis();
    buttonState = true;
  }

  if (reading == HIGH && lastButtonState == LOW) 
  {
    buttonState = false;
    unsigned long pressDuration = millis() - pressStart;

    // Nếu nhấn giữ >3s -> đổi chế độ
    if (pressDuration > 3000) 
    {
      configMode = !configMode;
      pressCount = 0;
      if (configMode) 
      {
        Serial.println("Vao che do cau hinh");
        blink(LED_PIN, 6, 100); // nháy nhanh để báo hiệu
      } 
      else 
      {
        Serial.print("Baud rate duoc ap dung la:"); Serial.println(baudRate);
        Serial.println("Tro lai che do van hanh");
        blink(LED_PIN, 6, 100); // nháy nhanh để báo hiệu
      }
    } 
    else if (configMode) 
    {
      // Đếm số lần nhấn nhanh trong cấu hình
      if (millis() - lastPressTime < 1000) 
      {
        pressCount++;
      } 
      else 
      {
        pressCount = 1;
      }
      lastPressTime = millis();
    }
  }

  // Kiểm tra chọn baud rate
  if (configMode) 
  {
    if (pressCount > 0 && (millis() - lastPressTime > 1000)) 
    {
      if (pressCount == 1) 
      {
        if (baudRate != 9600) 
        {
          baudRate = 9600;
          blink(LED_PIN, 1, 300);
          Serial.println("Baud rate: 115200 --> 9600");
        } 
        else 
        {
          blink(LED_PIN, 1, 300);
          Serial.println("Baud rate la 9600");
        }
      } 
      else if (pressCount == 2) 
      {
        if (baudRate != 115200) 
        {
          baudRate = 115200;
          blink(LED_PIN, 2, 300);
          Serial.println("Baud rate: 9600 --> 115200");
        } 
        else 
        {
          blink(LED_PIN, 2, 300);
          Serial.println("Baud rate la 115200");
        }
      } 
      pressCount = 0;
    }
  }

  lastButtonState = reading;
}

// Hàm nháy LED n lần + nhanh chậm
void blink(int pin, int times, int ms) 
{
  for (int i = 0; i < times; i++) 
  {
    digitalWrite(pin, HIGH);
    delay(ms);
    digitalWrite(pin, LOW);
    delay(ms);
  }
}

