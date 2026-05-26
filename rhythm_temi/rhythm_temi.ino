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
    
    // 방금 전까지 눌려있다가 손을 뗀 순간
    if (isPressed) {
      isPressed = false;
      
      // 누른 시간 계산 (현재 시간 - 시작 시간)
      unsigned long duration = millis() - pressStartTime;

      // LED 끄기
      digitalWrite(ledPin, HIGH);

      Serial.println("--------------------------------");
      Serial.println("패드 터치 종료! 데이터 전송 중...");
      Serial.print("누적 시간: "); Serial.print(duration); Serial.println(" ms");
      Serial.print("최대 압력: "); Serial.print(maxPressurePercent); Serial.println("%");

      // [수정] 수집된 데이터와 누른 시간을 함께 서버로 전송
      sendPadSignal(maxSensorValue, maxPressurePercent, duration);

      // 바운싱(노이즈) 방지를 위한 약간의 대기
      delay(300);
    }
  }

  // 루프 주기 조절 (반응 속도를 위해 50ms로 단축)
  delay(50);
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
// 서버 전송 (duration_ms 파라미터 추가)
// =====================================
void sendPadSignal(int sensorValue, int pressurePercent, unsigned long duration) {
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
  http.addHeader("Content-Type", "application/json");

  // [수정] Flask 서버 규격에 맞게 "duration_ms" 항목을 추가한 JSON 문자열 생성
  String jsonPayload =
    "{"
    "\"pad_id\":" + String(padId) + ","
    "\"value\":" + String(sensorValue) + ","
    "\"pressure\":" + String(pressurePercent) + ","
    "\"duration_ms\":" + String(duration) + ","
    "\"event\":\"touch_complete\""
    "}";

  Serial.println("전송 데이터:");
  Serial.println(jsonPayload);

  // POST 요청 전송
  int httpResponseCode = http.POST(jsonPayload);

  // 결과 출력
  if (httpResponseCode > 0) {
    Serial.print("전송 성공! 응답 코드: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.print("서버 응답: ");
    Serial.println(response);
  } else {
    Serial.print("전송 실패. 에러 코드: ");
    Serial.println(httpResponseCode);
  }

  // 연결 종료
  http.end();
  Serial.println("--------------------------------");
}