#include "driver/i2s.h"
#include <M5Cardputer.h>

#define display M5Cardputer.Display
#define kb M5Cardputer.Keyboard

#define I2S_SAMPLE_RATE     (8000)
#define I2S_SAMPLE_BITS     (I2S_BITS_PER_SAMPLE_16BIT)
#define I2S_CHANNEL_NUM     (1)
#define I2S_BUFFER_SIZE     (16)
#define PIN_LRC             43
#define PIN_BCLK            41
#define PIN_DIN             I2S_PIN_NO_CHANGE
#define PIN_DOUT            42

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  Serial.begin(9600);
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER| I2S_MODE_TX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_SAMPLE_BITS,
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 16,
    .dma_buf_len = I2S_BUFFER_SIZE,
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = PIN_BCLK,
    .ws_io_num = PIN_LRC,
    .data_out_num = PIN_DOUT,
    .data_in_num = PIN_DIN,
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk((i2s_port_t)0, I2S_SAMPLE_RATE, I2S_SAMPLE_BITS, (i2s_channel_t)1);
  playBeat();
}

static float VolumeToAmplification(int volume) 
//Volume in the Range 0..100
{
 /*
    https://www.dr-lex.be/info-stuff/volumecontrols.html
  
    Dynamic range   a           b       Approximation
    50 dB         3.1623e-3     5.757       x^3
    60 dB         1e-3          6.908       x^4
    70 dB         3.1623e-4     8.059       x^5
    80 dB         1e-4          9.210       x^6
    90 dB         3.1623e-5     10.36       x^6
    100 dB        1e-5          11.51       x^7
 */


float x = volume / 100.0f; //"volume" Range 0..100


#if 0
  float a = 1e-3;
  float b = 6.908f;
  float ampl = a * expf( b * x );
  if (x < 0.1f) ampl *= x*10.0f;
#else  
  //Approximation:
  float ampl = x * x * x * x; //60dB
#endif  

  return ampl;
}


void playBeat() {
  int t = 0;
  int volume = 25;
  int lineX = 0;
  int prevRes = 0;
  int prevLineX = lineX;

  while(true) {
    uint8_t out = t*((t&4096?t%65536<59392?7:t&7:16)+(1&t>>14))>>(3&-t>>(t&2048?2:10));
    int16_t res = map(out,0,255,-32768,32767); // Convert to 8 bit
    
    M5Cardputer.update();
    if (kb.isChange()) {
      if (kb.isPressed()) {
        if (kb.isKeyPressed(',') && volume > 5) { // Less than 10 is inaudible (at least for me) 
          volume -= 5;
        }
        if (kb.isKeyPressed('/') && volume < 70) { // More than 70 distorts the sound
          volume += 5;
        }
        Serial.println(volume);
      }
    }
    
    if (out <= display.height()) {
      display.drawLine(lineX, 0, lineX, display.height(), BLACK);
      display.drawPixel(lineX, display.height()-out, WHITE);
    }

    prevRes = out;
    
    if (lineX >= display.width()) {
      lineX = 0;
    }
    
    lineX += 2;
    
    res = res*VolumeToAmplification(volume);
    size_t size = 1;
    i2s_write(I2S_NUM_0, &res, sizeof(res), &size, portMAX_DELAY);
    
    t++;
  }
}

void loop() {
}
