#include <I2S.h>
#define I2S_SAMPLE_RATE     8000
#define I2S_SAMPLE_BITS     16

#define PIN_LRC             43
#define PIN_BCLK            41
#define PIN_DOUT            42

int t = 0;

void setup() {
  I2S.setSckPin(PIN_BCLK);
  I2S.setFsPin(PIN_LRC);
  I2S.setDataPin(PIN_DOUT);

  if (!I2S.begin(I2S_PHILIPS_MODE, I2S_SAMPLE_RATE, I2S_SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");

    while (1); // do nothing
  }
}

void loop() {
  // bytebeat
  uint8_t out = t*((t&4096?t%65536<59392?7:t&7:16)+(1&t>>14))>>(3&-t>>(t&2048?2:10));

  int16_t res = map(out,0,255,-32768,32767);

  I2S.write(res);
  I2S.write(res);

  t++;
}
