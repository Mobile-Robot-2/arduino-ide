#include <ESP8266WiFi.h>

// =========================
// 와이파이(핫스팟) 정보
// =========================
const char* ssid = "U+Net8D80";
const char* password = "498D21G@09";

// =========================
// Python 서버(노트북) 정보
// =========================
const char* host = "192.168.219.101"; // 노트북의 IP 주소
const int port = 5000;

WiFiClient client;

// =========================
// 센서 연결 핀 정의 (D1 ~ D5)
// =========================
#define PAD_A D1
#define PAD_B D2
#define PAD_C D3
#define PAD_D D4
#define PAD_E D5

// 이전 발판 상태 저장 변수 (기본값 HIGH)
bool lastA = HIGH;
bool lastB = HIGH;
bool lastC = HIGH;
bool lastD = HIGH;
bool lastE = HIGH;

void setup() {
  Serial.begin(115200);

  // 센서 핀을 입력 및 풀업 모드로 설정 (풀업: 평소 HIGH, 밟으면 LOW)
  pinMode(PAD_A, INPUT_PULLUP);
  pinMode(PAD_B, INPUT_PULLUP);
  pinMode(PAD_C, INPUT_PULLUP);
  pinMode(PAD_D, INPUT_PULLUP);
  pinMode(PAD_E, INPUT_PULLUP);

  // WiFi 연결 시작
  WiFi.begin(ssid, password);
  Serial.print("WiFi 연결중");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi 연결 완료");
  Serial.print("아두이노 IP: ");
  Serial.println(WiFi.localIP());

  // 서버에 최초 연결 시도
  connectServer();
}

void loop() {
  // 서버와 연결이 끊어지면 자동으로 재연결 시도
  if (!client.connected()) {
    connectServer();
  }

  // 5개 패드 센서 값 실시간 읽기
  bool currentA = digitalRead(PAD_A);
  bool currentB = digitalRead(PAD_B);
  bool currentC = digitalRead(PAD_C);
  bool currentD = digitalRead(PAD_D);
  bool currentE = digitalRead(PAD_E);

  // 1번 패드 (PAD_A): 평소 HIGH(1)였다가 발로 밟아 LOW(0)가 되는 순간 감지
  if (lastA == HIGH && currentA == LOW) {
    sendMessage("PAD_A");
    delay(200); // 밟을 때 진동으로 여러 번 찍히는 현상(바운싱) 방지
  }
  
  // 2번 패드 (PAD_B)
  if (lastB == HIGH && currentB == LOW) {
    sendMessage("PAD_B");
    delay(200);
  }

  // 3번 패드 (PAD_C)
  if (lastC == HIGH && currentC == LOW) {
    sendMessage("PAD_C");
    delay(200);
  }

  // 4번 패드 (PAD_D)
  if (lastD == HIGH && currentD == LOW) {
    sendMessage("PAD_D");
    delay(200);
  }

  // 5번 패드 (PAD_E)
  if (lastE == HIGH && currentE == LOW) {
    sendMessage("PAD_E");
    delay(200);
  }

  // 다음 루프 비교를 위해 현재 상태를 이전 상태 변수에 저장
  lastA = currentA;
  lastB = currentB;
  lastC = currentC;
  lastD = currentD;
  lastE = currentE;

  delay(10); // 시스템 과부하 방지를 위한 미세한 대기
}

// =========================
// 서버 연결 함수
// =========================
void connectServer() {
  Serial.println("파이썬 서버 연결 시도...");

  while (!client.connect(host, port)) {
    Serial.println("연결 실패! 1초 후 재시도...");
    delay(1000);
  }

  Serial.println("파이썬 서버 연결 성공!");
}

// =========================
// 메시지 전송 함수
// =========================
void sendMessage(String msg) {
  client.println(msg); // 문자열 전송 후 끝에 \n 자동으로 붙음
  Serial.print("서버로 신호 전송: ");
  Serial.println(msg);
}