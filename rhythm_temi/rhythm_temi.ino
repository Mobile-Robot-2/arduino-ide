#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// =====================================
// WiFi 설정
// =====================================
const char* ssid = "AndroidHotspot1119";
const char* password = "2022073290";

// =====================================
// 서버 주소 (노트북 IP 주소 확인 필요)
// =====================================
const char* serverUrl = "http://10.139.44.59:5000/pad";

// =====================================
// FSR 센서 설정
// =====================================
const int sensorPin = A0;  // FSR 연결 핀
const int ledPin = D4;     // 내장 LED 핀
const int threshold = 200; // 눌림 기준값
const int padId = 1;       // 패드 번호

// =====================================
// [수정] 시간 측정 및 상태 관리를 위한 변수
// =====================================
bool isPressed = false;           // 현재 눌려있는지 여부
unsigned long pressStartTime = 0; // 누르기 시작한 시간 (밀리초)
int maxSensorValue = 0;           // 누르는 동안의 최대 센서 원본값
int maxPressurePercent = 0;       // 누르는 동안의 최대 압력 퍼센트

// =====================================

void setup() {
  Serial.begin(115200);

  // LED 출력 설정
  pinMode(ledPin, OUTPUT);
  // 시작 시 LED 끄기 (ESP8266 내장 LED는 HIGH가 꺼짐입니다)
  digitalWrite(ledPin, HIGH);

  Serial.println();
  Serial.println("================================");
  Serial.println("FSR 스마트 패드 시스템 시작 (시간 측정 추가)");
  Serial.println("================================");

  connectWiFi();

  Serial.println("시스템 준비 완료");
}

// =====================================

void loop() {
  // FSR 값 읽기
  int sensorValue = analogRead(sensorPin);

  // 작은 노이즈 제거
  if (sensorValue < 10) {
    sensorValue = 0;
  }

  // 압력 퍼센트 계산 (0 ~ 1023 -> 0 ~ 100%)
  int pressurePercent = map(sensorValue, 0, 1023, 0, 100);

  // 1. 센서가 눌렸을 때 (기준값 초과)
  if (sensorValue > threshold) {
    
    // 처음 눌린 순간
    if (!isPressed) {
      isPressed = true;
      pressStartTime = millis(); // 누르기 시작한 절대 시간 기록
      maxSensorValue = sensorValue;
      maxPressurePercent = pressurePercent;

      // LED 켜기 (LOW가 켜짐)
      digitalWrite(ledPin, LOW);
      Serial.println("패드 터치 감지! (측정 시작)");
    } 
    // 이미 누르고 있는 상태라면 최대 압력값 지속 갱신
    else {
      if (sensorValue > maxSensorValue) {
        maxSensorValue = sensorValue;
        maxPressurePercent = pressurePercent;
      }
    }

  } 
  // 2. 센서에서 손을 뗐을 때 (기준값 미만)
  else {