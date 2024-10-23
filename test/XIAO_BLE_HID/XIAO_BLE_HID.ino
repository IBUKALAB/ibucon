// 必要なライブラリをインクルード
#include <SPI.h>
#include <Adafruit_MCP23X17.h> // MCP23S17用ライブラリ
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>         // HID関連の型定義
#include <HIDKeyboardTypes.h> // キーボード用HIDタイプ

// MCP23S17のSPI接続ピンの定義
#define MCP_CS_PIN 17     // チップセレクトピン（XIAO ESP32C6のD7）
#define MCP_MOSI_PIN MOSI // MOSIピン（XIAO ESP32C6のD10）
#define MCP_MISO_PIN MISO // MISOピン（XIAO ESP32C6のD9）
#define MCP_SCK_PIN SCK   // SCKピン（XIAO ESP32C6のD8）

// MCP23S17のインスタンス作成
Adafruit_MCP23X17 mcp;

// BLE HID関連の変数
BLEHIDDevice *hidDevice;
BLECharacteristic *inputKeyboard;
bool deviceConnected = false;

// BLEサーバーのコールバッククラス
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    Serial.println("デバイスが接続されました");
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    Serial.println("デバイスが切断されました");
    BLEDevice::startAdvertising();
  }
};

// HIDレポートマップ用のマクロを定義（Arduinoのマクロと競合しないように）
#define HID_USAGE_PAGE(n) 0x05, n
#define HID_USAGE(n) 0x09, n
#define HID_COLLECTION(n) 0xA1, n
#define HID_REPORT_ID(n) 0x85, n
#define HID_USAGE_MINIMUM(n) 0x19, n
#define HID_USAGE_MAXIMUM(n) 0x29, n
#define HID_LOGICAL_MINIMUM(n) 0x15, n
#define HID_LOGICAL_MAXIMUM(n) 0x25, n
#define HID_REPORT_SIZE(n) 0x75, n
#define HID_REPORT_COUNT(n) 0x95, n
#define HID_INPUT(n) 0x81, n
#define HID_END_COLLECTION 0xC0

void setup()
{
  // シリアルモニタの初期化（デバッグ用）
  Serial.begin(115200);
  Serial.println("BLE HIDキーボードを起動します");

  // SPIの初期化
  SPI.begin(MCP_SCK_PIN, MCP_MISO_PIN, MCP_MOSI_PIN);

  // MCP23S17の初期化
  if (!mcp.begin_SPI(MCP_CS_PIN))
  {
    Serial.println("MCP23S17の初期化に失敗しました");
    while (1)
      ;
  }

  // モーメンタリスイッチのピンを入力プルアップモードに設定
  for (uint8_t i = 0; i < 5; i++)
  {
    mcp.pinMode(i + 8, INPUT_PULLUP); // GPB0～GPB4に対応
  }

  // BLEの初期化
  BLEDevice::init("XIAO BLE HID");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  hidDevice = new BLEHIDDevice(pServer);
  inputKeyboard = hidDevice->inputReport(1); // レポートID 1

  // キーボードのHIDレポートマップを設定
  const uint8_t reportMap[] = {
      HID_USAGE_PAGE(0x01), // Generic Desktop
      HID_USAGE(0x06),      // Keyboard
      HID_COLLECTION(0x01), // Application
      HID_REPORT_ID(0x01),  // レポートID
      HID_USAGE_PAGE(0x07), // Keyboard/Keypad
      HID_USAGE_MINIMUM(0xE0),
      HID_USAGE_MAXIMUM(0xE7),
      HID_LOGICAL_MINIMUM(0x00),
      HID_LOGICAL_MAXIMUM(0x01),
      HID_REPORT_SIZE(0x01),
      HID_REPORT_COUNT(0x08),
      HID_INPUT(0x02), // Data, Variable, Absolute
      HID_REPORT_COUNT(0x01),
      HID_REPORT_SIZE(0x08),
      HID_INPUT(0x01), // Constant
      HID_REPORT_COUNT(0x06),
      HID_REPORT_SIZE(0x08),
      HID_LOGICAL_MINIMUM(0x00),
      HID_LOGICAL_MAXIMUM(0x65),
      HID_USAGE_MINIMUM(0x00),
      HID_USAGE_MAXIMUM(0x65),
      HID_INPUT(0x00), // Data, Array
      HID_END_COLLECTION};
  hidDevice->reportMap((uint8_t *)reportMap, sizeof(reportMap));
  hidDevice->startServices();

  // BLE広告の設定
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hidDevice->hidService()->getUUID());
  pAdvertising->start();
  hidDevice->setBatteryLevel(100);

  Serial.println("クライアントの接続待ち...");
}

void loop()
{
  static uint8_t prevButtonState = 0xFF; // 初期状態はすべてHIGH（未押下）

  // MCP23S17からGPBポートの状態を読み取る
  uint16_t gpioState = mcp.readGPIOAB();
  uint8_t buttonState = (gpioState >> 8) & 0xFF; // GPBポートの状態

  // 各ボタンの状態を出力
  Serial.print("ボタンの状態: ");
  for (uint8_t i = 0; i < 5; i++)
  {
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
  for (uint8_t i = 0; i < 5; i++)
  {
    if (changedButtons & (1 << i))
    {
      // 状態が変化したボタン
      if (!(buttonState & (1 << i)))
      {
        // ボタンが押された
        uint8_t keycode = 0x04 + i; // 'a'から'e'のキーコード
        sendKey(keycode);
        Serial.print("キー ");
        Serial.print((char)('a' + i));
        Serial.println(" が押されました");
      }
    }
  }

  prevButtonState = buttonState;

  delay(50); // デバウンス処理
}

// キー入力を送信する関数
void sendKey(uint8_t keycode)
{
}
