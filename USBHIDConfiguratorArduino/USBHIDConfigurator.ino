#include "Keyboard.h"
#include "Mouse.h"

const int buttonPins[] = {0, 1, 2}; // モーメンタリースイッチを接続するピン番号
char keysToSend[] = {'A', 'B', 'C'}; // 各ボタンに対応するデフォルトキー

const int vrxPin = A3; // アナログジョイスティックX軸
const int vryPin = A4; // アナログジョイスティックY軸
const int swPin = 5; // ジョイスティックスイッチ

const int enc1Apin = 6; // エンコーダ1のAピン
const int enc1Bpin = 7; // エンコーダ1のBピン
const int enc2Apin = 8; // エンコーダ2のAピン
const int enc2Bpin = 9; // エンコーダ2のBピン

volatile int enc1Pos = 0;
volatile bool enc1Updated = false;
volatile int enc2Pos = 0;
volatile bool enc2Updated = false;

String buttonConfigs[3] = {"A", "B", "C"}; // ボタンの設定を保存する配列
String wheelConfigs[2] = {"スクロール1", "スクロール2"}; // スクロールホイールの設定を保存する配列

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(swPin, INPUT_PULLUP);
  pinMode(enc1Apin, INPUT_PULLUP);
  pin### 修正されたArduinoコード（最新のWindowsアプリに対応）

```cpp
#include "Keyboard.h"
#include "Mouse.h"

const int buttonPins[] = {0, 1, 2}; // モーメンタリースイッチを接続するピン番号
char keysToSend[] = {'A', 'B', 'C'}; // 各ボタンに対応するデフォルトキー

const int vrxPin = A3; // アナログジョイスティックX軸
const int vryPin = A4; // アナログジョイスティックY軸
const int swPin = 5; // ジョイスティックスイッチ

const int enc1Apin = 6; // エンコーダ1のAピン
const int enc1Bpin = 7; // エンコーダ1のBピン
const int enc2Apin = 8; // エンコーダ2のAピン
const int enc2Bpin = 9; // エンコーダ2のBピン

volatile int enc1Pos = 0;
volatile bool enc1Updated = false;
volatile int enc2Pos = 0;
volatile bool enc2Updated = false;

String buttonConfigs[3] = {"A", "B", "C"}; // ボタンの設定を保存する配列
String wheelConfigs[2] = {"Scroll1", "Scroll2"}; // スクロールホイールの設定を保存する配列

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(swPin, INPUT_PULLUP);
  pinMode(enc1Apin, INPUT_PULLUP);
  pinMode(enc1Bpin, INPUT_PULLUP);
  pinMode(enc2Apin, INPUT_PULLUP);
  pinMode(enc2Bpin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(enc1Apin), updateEnc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc1Bpin), updateEnc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc2Apin), updateEnc2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(enc2Bpin), updateEnc2, CHANGE);

  Keyboard.begin();
  Mouse.begin();
  Serial.begin(115200);
  Serial.println("Setup complete");
}

void loop() {
  // 各ボタンの状態をチェックして対応するキーを送信
  for (int i = 0; i < 3; i++) {
    int buttonState = digitalRead(buttonPins[i]);
    if (buttonState == LOW) {
      sendKey(buttonConfigs[i]);
      Serial.print("Key sent for button ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(buttonConfigs[i]);
      delay(50); // デバウンス用の遅延
      while (digitalRead(buttonPins[i]) == LOW) {
        delay(10); // ボタンが放されるまで待機
      }
      Keyboard.release(buttonConfigs[i].charAt(0));
    }
  }

  // ジョイスティックのアナログ入力を読み取る
  int vrxValue = analogRead(vrxPin);
  int vryValue = analogRead(vryPin);

  // デッドゾーンを設定
  const int deadZone = 34; // ニュートラル位置の5%範囲（0-1023のうち）
  if (abs(vrxValue - 512) < deadZone) {
    vrxValue = 512;
  }
  if (abs(vryValue - 512) < deadZone) {
    vryValue = 512;
  }

  // アナログ値をマウスの移動量に変換
  int moveX = map(vrxValue, 0, 1023, -10, 10);
  int moveY = map(vryValue, 0, 1023, -10, 10);

  // マウスを移動
  if (moveX != 0 || moveY != 0) {
    Mouse.move(moveX, -moveY); // y軸は反転させる
  }

  // ジョイスティックの押下をチェック
  int swState = digitalRead(swPin);
  if (swState == LOW) {
    Mouse.press(MOUSE_LEFT); // スイッチ押下で左クリック
  } else {
    Mouse.release(MOUSE_LEFT);
  }

  // ロータリーエンコーダの値をシリアルモニタに表示
  if (enc1Updated) {
    Serial.print("Enc1 Position: ");
    Serial.println(enc1Pos);
    enc1Updated = false;

    // 縦スクロール
    sendScroll(enc1Pos, wheelConfigs[0]); // 縦スクロール
    enc1Pos = 0; // リセット
  }

  if (enc2Updated) {
    Serial.print("Enc2 Position: ");
    Serial.println(enc2Pos);
    enc2Updated = false;

    // 横スクロール
    sendScroll(enc2Pos, wheelConfigs[1]); // 横スクロール
    enc2Pos = 0; // リセット
  }

  delay(10); // 感度を調整
}

void updateEnc1() {
  static bool lastStateA = LOW;
  static bool lastStateB = LOW;
  bool currentStateA = digitalRead(enc1Apin);
  bool currentStateB = digitalRead(enc1Bpin);

  if (currentStateA != lastStateA || currentStateB != lastStateB) {
    if (currentStateA == lastStateB) {
      enc1Pos++;
    } else {
      enc1Pos--;
    }
    lastStateA = currentStateA;
    lastStateB = currentStateB;
    enc1Updated = true;
  }
}

void updateEnc2() {
  static bool lastStateA = LOW;
  static bool lastStateB = LOW;
  bool currentStateA = digitalRead(enc2Apin);
  bool currentStateB = digitalRead(enc2Bpin);

  if (currentStateA != lastStateA || currentStateB != lastStateB) {
    if (currentStateA == lastStateB) {
      enc2Pos++;
    } else {
      enc2Pos--;
    }
    lastStateA = currentStateA;
    lastStateB = currentStateB;
    enc2Updated = true;
  }
}

void sendKey(String key) {
  if (key.length() > 0) {
    Keyboard.press(key.charAt(0));
    delay(100);
    Keyboard.release(key.charAt(0));
  }
}

void sendScroll(int delta, String config) {
  if (config.startsWith("Shift+")) {
    Keyboard.press(KEY_LEFT_SHIFT);
    if (config.endsWith("スクロール1")) {
      Mouse.scroll(delta);
    } else if (config.endsWith("スクロール2")) {
      Mouse.move(delta, 0);
    }
    Keyboard.release(KEY_LEFT_SHIFT);
  } else {
    if (config == "スクロール1") {
      Mouse.scroll(delta);
    } else if (config == "スクロール2") {
      Mouse.move(delta, 0);
    }
  }
}

void parseConfig(String configString) {
  int index = configString.indexOf("B1:");
  if (index != -1) {
    int endIndex = configString.indexOf(";", index);
    buttonConfigs[0] = configString.substring(index + 3, endIndex);
  }

  index = configString.indexOf("B2:");
  if (index != -1) {
    int endIndex = configString.indexOf(";", index);
    buttonConfigs[1] = configString.substring(index + 3, endIndex);
  }

  index = configString.indexOf("B3:");
  if (index != -1) {
    int endIndex = configString.indexOf(";", index);
    buttonConfigs[2] = configString.substring(index + 3, endIndex);
  }

  index = configString.indexOf("W1:");
  if (index != -1) {
    int endIndex = configString.indexOf(";", index);
    wheelConfigs[0] = configString.substring(index + 3, endIndex);
  }

  index = configString.indexOf("W2:");
  if (index != -1) {
    int endIndex = configString.indexOf(";", index);
    wheelConfigs[1] = configString.substring(index + 3, endIndex);
  }
}
