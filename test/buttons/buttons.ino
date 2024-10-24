// 必要なライブラリをインクルード
#include <SPI.h>
#include <Adafruit_MCP23X17.h>  // MCP23S17用ライブラリ

// MCP23S17のSPI接続ピンの定義
#define MCP_CS_PIN D7  // チップセレクトピン（XIAO ESP32C6のD7）

// MCP23S17のインスタンス作成
Adafruit_MCP23X17 mcp;

void setup(void) {
  // シリアルモニタの初期化（デバッグ用）
  Serial.begin(115200);

  // SPIの初期化
  SPI.begin();

  // MCP23S17の初期化
  if (!mcp.begin_SPI(MCP_CS_PIN)) {
    Serial.println("MCP23S17の初期化に失敗しました");
    abort();
  }

  // モーメンタリスイッチのピンを入力プルアップモードに設定
  for (uint8_t i = 0; i < 5; i++) {
    mcp.pinMode(i + 8, INPUT_PULLUP);  // GPB0～GPB4に対応
  }
}

void loop(void) {
  static uint8_t prevButtonState = 0xFF;  // 初期状態はすべてHIGH（未押下）

  // MCP23S17からGPBポートの状態を読み取る
  uint16_t gpioState = mcp.readGPIOAB();
  uint8_t buttonState = (gpioState >> 8) & 0xFF;  // GPBポートの状態

  // 各ボタンの状態を出力
  Serial.print("ボタンの状態: ");
  for (uint8_t i = 0; i < 5; i++) {
    bool state = buttonState & (1 << i);
    Serial.print("GPB");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(state ? "HIGH" : "LOW");
    Serial.print(" ");
  }
  Serial.println();

  // ボタンの状態変化を検出
  uint8_t changedButtons = buttonState ^ prevButtonState;

  // 各ボタンの状態をチェック（プルアップのため押下時は0）
  for (uint8_t i = 0; i < 5; i++) {
    if (changedButtons & (1 << i)) {
      // 状態が変化したボタン
      if (!(buttonState & (1 << i))) {
        // ボタンが押された
        uint8_t keycode = 0x04 + i;  // 'a'から'e'のキーコード
        Serial.print("キー ");
        Serial.print((char)('a' + i));
        Serial.println(" が押されました");
      }
    }
  }

  prevButtonState = buttonState;

  delay(50);  // デバウンス処理
}
