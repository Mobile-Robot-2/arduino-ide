#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// =====================================
// WiFi 설정
// =====================================

const char* ssid = "AndroidHotspot1119";
const char* password = "2022073290";

// =====================================
// 서버 주소
// =====================================

const char* serverUrl =
  "http://10.139.44.59:5000/pad";

// =====================================
// FSR 센서 설정
// =====================================

// FSR 연결 핀
const int sensorPin = A0;

// LED 핀 (내장 LED)
const int ledPin = D4;

// 눌림 기준값
const int threshold = 200;

// 중복 입력 방지
bool isPressed = false;

// 패드 번호
const int padId = 1;

// =====================================

void setup() {

  Serial.begin(115200);

  // LED 출력 설정
  pinMode(ledPin, OUTPUT);

  // 시작 시 LED 끄기
  digitalWrite(ledPin, HIGH);

  Serial.println();
  Serial.println("================================");
  Serial.println("FSR 스마트 패드 시스템 시작");
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

  // 압력 퍼센트 계산
  int pressurePercent =
    map(sensorValue, 0, 1023, 0, 100);

  // threshold 이상일 때만 동작
  if (sensorValue > threshold) {

    Serial.println("--------------------------------");

    Serial.print("FSR 값: ");
    Serial.println(sensorValue);

    Serial.print("압력 강도(%): ");
    Serial.println(pressurePercent);

    // 처음 눌린 순간만 실행
    if (!isPressed) {

      Serial.println("패드 터치 감지!");

      // LED 켜기
      digitalWrite(ledPin, LOW);

      // 서버 전송
      sendPadSignal(
        sensorValue,
        pressurePercent
      );

      // 눌림 상태 저장
      isPressed = true;

      // 노이즈 방지
      delay(300);
    }

  } else {

    // LED 끄기
    digitalWrite(ledPin, HIGH);

    // 다시 입력 가능
    isPressed = false;
  }

  delay(100);
}

// =====================================
// WiFi 연결
// =====================================

void connectWiFi() {

  Serial.print("WiFi 연결 중");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi 연결 성공!");
  Serial.println("================================");
}

// =====================================
// 서버 전송
// =====================================

void sendPadSignal(
  int sensorValue,
  int pressurePercent
) {

  // WiFi 연결 확인
  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi 연결 끊김");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  // 서버 연결 시작
  http.begin(client, serverUrl);

  // JSON 형식 지정
  http.addHeader(
    "Content-Type",
    "application/json"
  );

  // 서버로 보낼 데이터
  String jsonPayload =
    "{"
    "\"pad_id\":" + String(padId) + ","
    "\"value\":" + String(sensorValue) + ","
    "\"pressure\":" + String(pressurePercent) + ","
    "\"event\":\"touch\""
    "}";

  Serial.println("전송 데이터:");
  Serial.println(jsonPayload);

  // POST 요청 전송
  int httpResponseCode =
    http.POST(jsonPayload);

  // 결과 출력
  if (httpResponseCode > 0) {

    Serial.print("전송 성공! 응답 코드: ");
    Serial.println(httpResponseCode);

    String response =
      http.getString();

    Serial.print("서버 응답: ");
    Serial.println(response);

  } else {

    Serial.print("전송 실패. 에러 코드: ");
    Serial.println(httpResponseCode);

    Serial.println("서버 주소 또는 포트 확인");
  }

  // 연결 종료
  http.end();

  Serial.println("--------------------------------");
}