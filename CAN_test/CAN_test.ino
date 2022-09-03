#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <mcp_can.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define Ratio 6.0
#define WHL_CRC 1.5
#define BUTTON 3
#define INT 2

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

uint16_t RPM = 0;
char Velocity[6] = "";
uint8_t controller_temp = 0;
uint8_t laps = 0;
uint8_t BattVolt = 0;
bool lapCountLock = 0;
unsigned long exTime;

MCP_CAN CAN(10);

void setup() {
  Serial.begin(9600);
  CAN.begin(CAN_250KBPS);
  lcd.begin();
  lcd.backlight();
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), lapCounter, RISING);
  pinMode (INT, INPUT);
  exTime = millis();
}

void loop() {
  if (millis() - exTime > 30000) {
    lapCountLock = 0;
  }
  if (!digitalRead(2))     // 데이터가 수신되면 2번핀이 LOW상태가 됩니다.
  {
    CAN.readMsgBuf(&len, rxBuf);  // 데이터의 길이와 데이터를 읽습니다.
    rxId = CAN.getCanId();        // 데이터의 ID(식별자)를 읽습니다.
    Serial.print("ID: ");
    Serial.println(rxId, HEX);       // ID를 출력합니다.
    if (rxId == 0x0CF11E05) {
      Serial.print("RPM: ");
      RPM = (rxBuf[1] << 8) + rxBuf[0];
      Serial.println(RPM);
      Serial.print("Battery Voltage: ");
      BattVolt = ((rxBuf[3] << 8) + rxBuf[2]) / 10.0;
      Serial.println(BattVolt);
    }
    else if (rxId == 0x0CF11F05) {
      Serial.print("Throttle: ");
      Serial.println(rxBuf[0]);
      Serial.print("Controller temp: ");
      controller_temp = rxBuf[1];
      Serial.println(controller_temp);
    }
  }
  cluster();
}

void lapCounter() {
  delay(200);
  if (digitalRead(BUTTON) && !lapCountLock) {
    laps++;
    lapCountLock = 1;
    exTime = millis();
  }
  else {
    //nothing
  }
}

void cluster() {
  char info_1[20];
  char info_2[20] = " ";
  dtostrf(RPM / Ratio * WHL_CRC * 60 / 1000, 5, 1, Velocity);
  sprintf(info_1, "%4dRPM%skm/h", RPM, Velocity);
  sprintf(info_2, "%4d\'C%5dLaps", controller_temp, laps);
  lcd.setCursor(0, 0);
  Serial.println(info_1);
  lcd.print(info_1);
  lcd.setCursor(0, 1);
  lcd.print(info_2);
}
