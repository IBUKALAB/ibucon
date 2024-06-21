#include <SPI.h>

static constexpr int IOEXP_CS_PIN = 17;
static constexpr int IOEXP_SCK = SCK;
static constexpr int IOEXP_MOSI = MOSI;
static constexpr int IOEXP_MISO = MISO;

void setup() {
  Serial.begin(115200);

  pinMode(IOEXP_CS_PIN, OUTPUT);
  digitalWrite(IOEXP_CS_PIN, HIGH);
  SPI.begin(IOEXP_SCK, IOEXP_MISO, IOEXP_MOSI);
  SPI.beginTransaction({ 10000000, MSBFIRST, SPI_MODE0 });
}

void loop() {

  uint8_t data[] = {
    0b01000001,  // read
    0x00,        // register address
    0x00,
    0x00,
    0x00,
    0x00,
  };

  digitalWrite(IOEXP_CS_PIN, LOW);
  SPI.transfer(data, sizeof(data));
  digitalWrite(IOEXP_CS_PIN, HIGH);

  Serial.printf("%02x %02x %02x %02x\n", data[2], data[3], data[4], data[5]);

  delay(1000);
}
