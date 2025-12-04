#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

#define MODE_BTN 2
#define SET_BTN 3
#define UP_BTN 4
#define DOWN_BTN 5
#define BUZZER 6

// ====== BIẾN TOÀN CỤC ======
volatile bool modePressed = false;
volatile bool setPressed = false;
unsigned long modePressStartTime = 0; // Biến theo dõi thời gian nhấn giữ MODE

int mode = 0;               // 0: xem giờ, 1: cài báo thức, 2: cài thời gian
int subMode = 0;            // con trỏ trong quá trình cài
int alarmHour = 0, alarmMin = 0;
bool alarmOn = false;

unsigned long lastUpdate = 0;
bool isRinging = false;

// ====== HÀM NGẮT ======
void modeISR() {
  static unsigned long last = 0;
  if (millis() - last > 200) modePressed = true;
  last = millis();
}

void setISR() {
  static unsigned long last = 0;
  if (millis() - last > 200) setPressed = true;
  last = millis();
}

void setup() 
{
  pinMode(BUZZER, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(MODE_BTN), modeISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(SET_BTN), setISR, FALLING);

  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Line 1 test OK");
  lcd.setCursor(0, 1);
  lcd.print("Line 2 test OK");

  // Kiểm tra RTC
  if (!rtc.begin()) {
    lcd.print("RTC not found!");
  }

  // Nếu RTC chưa cài giờ, cài giờ theo thời gian máy tính
  if (!rtc.isrunning()) {
    lcd.clear();
    lcd.print("RTC adjust...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(2000);
  }

  lcd.clear();

}

// ====== HÀM HIỂN THỊ ======
void displayTime(DateTime now) {
  lcd.setCursor(0, 0);
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());
  
  lcd.setCursor(0, 1);
  lcd.print("AL:");
  if (alarmHour < 10) lcd.print("0");
  lcd.print(alarmHour);
  lcd.print(":");
  if (alarmMin < 10) lcd.print("0");
  lcd.print(alarmMin);
  lcd.print(alarmOn ? " ON " : " OFF");
}

// ====== HÀM CÀI BÁO THỨC ======
void setAlarm() {
  lcd.clear();
  lcd.print("Set Alarm Time");
  delay(1000);
  lcd.clear();
  subMode = 0;

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Hour:");
    if (alarmHour < 10) lcd.print("0");
    lcd.print(alarmHour);

    lcd.setCursor(0, 1);
    lcd.print("Min :");
    if (alarmMin < 10) lcd.print("0");
    lcd.print(alarmMin);

    if (digitalRead(UP_BTN) == LOW) {
      if (subMode == 0) alarmHour = (alarmHour + 1) % 24;
      else alarmMin = (alarmMin + 1) % 60;
      delay(200);
    }

    if (digitalRead(DOWN_BTN) == LOW) {
      if (subMode == 0) alarmHour = (alarmHour + 23) % 24;
      else alarmMin = (alarmMin + 59) % 60;
      delay(200);
    }

    if (setPressed) {
      setPressed = false;
      subMode++;
      if (subMode > 1) {
        lcd.clear();
        lcd.print("Alarm Saved!");
        delay(1000);
        lcd.clear();
        return;
      }
    }
  }
}

// ====== HÀM CÀI THỜI GIAN ======
void setTime() {
  lcd.clear();
  lcd.print("Set RTC Time");
  delay(1000);
  lcd.clear();

  DateTime now = rtc.now();
  int h = now.hour(), m = now.minute();

  subMode = 0;
  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Hour:");
    if (h < 10) lcd.print("0");
    lcd.print(h);

    lcd.setCursor(0, 1);
    lcd.print("Min :");
    if (m < 10) lcd.print("0");
    lcd.print(m);

    if (digitalRead(UP_BTN) == LOW) {
      if (subMode == 0) h = (h + 1) % 24;
      else m = (m + 1) % 60;
      delay(200);
    }

    if (digitalRead(DOWN_BTN) == LOW) {
      if (subMode == 0) h = (h + 23) % 24;
      else m = (m + 59) % 60;
      delay(200);
    }

    if (setPressed) {
      setPressed = false;
      subMode++;
      if (subMode > 1) {
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), h, m, 0));
        lcd.clear();
        lcd.print("Time Updated!");
        delay(1000);
        lcd.clear();
        return;
      }
    }
  }
}

// ====== HÀM XỬ LÝ BÁO THỨC ======
void checkAlarm(DateTime now) {
  if (alarmOn && now.hour() == alarmHour && now.minute() == alarmMin && now.second() == 0) {
    isRinging = true;
  }

  if (isRinging) {
    tone(BUZZER, 1000);

    // Nếu nhấn nút SET hoặc MODE thì tắt báo thức
    if (setPressed || modePressed) {
      setPressed = false;
      modePressed = false;
      isRinging = false;
      noTone(BUZZER);
    }
  }
}

void loop() {
  DateTime now = rtc.now();

  // Cập nhật hiển thị mỗi 500ms
  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    displayTime(now);
  }

  // Kiểm tra báo thức
  checkAlarm(now);

  // Xử lý nút MODE (chuyển chế độ)
  if (modePressed) 
  {
    modePressed = false;
    modePressStartTime = millis();

    // Đợi người dùng thả nút
    while (digitalRead(MODE_BTN) == LOW);

    if (millis() - modePressStartTime > 2000) 
    {
      alarmOn = !alarmOn;
      // EEPROM.write(2, alarmOn);
      lcd.clear();
      lcd.print(alarmOn ? "Alarm ON " : "Alarm OFF");
      delay(1000);
      lcd.clear();
      // while (digitalRead(MODE_BTN) == LOW);
    }
    else if (millis() - modePressStartTime < 2000)
    {
      mode++;
      if (mode > 2) mode = 0;

      switch (mode) 
      {
        case 1: setAlarm(); break;
        case 2: setTime(); break;
        // default: lcd.clear(); break;
      }
    } 
  }
}
