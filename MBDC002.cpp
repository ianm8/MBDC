/*
 * Multi-band Direct Conversion receiver control software
 * Version 1.0
 *
 * Copyright 2022, Ian Mitchell, VK7IAN
 */

//https://github.com/etherkit/Si5351Arduino
// 67848 bytes with new/delete
// 67808 bytes with stack

#include <ResponsiveAnalogRead.h>
#include "si5351.h"
#include "Wire.h"

#define DIT_PERIOD     100
#define SET_BANDS      11
#define BAND80_PIN     22
#define BAND40_PIN     21
#define BAND20_PIN     20
#define BAND15_PIN     19
#define BAND10_PIN     18
#define LED_SIGNAL_PIN 15
#define MAIN_TUNE_PIN  27
#define FINE_TUNE_PIN  26
#define BAND_PIN       28

// bands
/*
 *  0: 80 start (3500000 - 3600) and multiplier of 75
 *  1: 40 start (7000000 - 3600) and multiplier of 75
 *  2: 20 start (14000000 - 4660) and multiplier of 45
 *  3: 20 start (14175000 - 4660) and multiplier of 45
 *  4: 15 start (21000000 - 140) and multiplier of 55
 *  5: 15 start (21225000 - 140) and multiplier of 55
 *  6: 10 start (28000000 - 4080 ) and multiplier of 85
 *  7: 10 start (28340000 - 4080 ) and multiplier of 85
 *  8: 10 start (28680000 - 4080 ) and multiplier of 85
 *  9: 10 start (29020000 - 4080 ) and multiplier of 85
 * 10: 10 start (29360000 - 4080 ) and multiplier of 85
 */
typedef struct
{
  int32_t start;
  int32_t step;
} band_t;

static const band_t bands[SET_BANDS] =
{
  {3500000L - 3600L, 75L},
  {7000000L - 3600L, 75L},
  {14000000 - 4660L, 45L},
  {14175000 - 4660L, 45L},
  {21000000 - 140L,  55L},
  {21225000 - 140L,  55L},
  {28000000 - 4080L, 85L},
  {28340000 - 4080L, 85L},
  {28680000 - 4080L, 85L},
  {29020000 - 4080L, 85L},
  {29360000 - 4080L, 85L}
};


/*
 * band, measured centre values
 *
 *  0: 0
 *  1: 11
 *  2: 41
 *  3: 71
 *  4: 106
 *  5: 142
 *  6: 178
 *  7: 208
 *  8: 229
 *  9: 249,250,251,252
 * 10: 254
 */

static const uint8_t band_map[256] =
{
  (uint8_t)0U, //   0
  (uint8_t)0U, //   1
  (uint8_t)0U, //   2
  (uint8_t)0U, //   3
  (uint8_t)0U, //   4
  (uint8_t)0U, //   5
  (uint8_t)1U, //   6
  (uint8_t)1U, //   7
  (uint8_t)1U, //   8
  (uint8_t)1U, //   9
  (uint8_t)1U, //  10
  (uint8_t)1U, //  11: 1
  (uint8_t)1U, //  12
  (uint8_t)1U, //  13
  (uint8_t)1U, //  14
  (uint8_t)1U, //  15
  (uint8_t)1U, //  16
  (uint8_t)1U, //  17
  (uint8_t)1U, //  18
  (uint8_t)1U, //  19
  (uint8_t)1U, //  20
  (uint8_t)1U, //  21
  (uint8_t)1U, //  22
  (uint8_t)1U, //  23
  (uint8_t)1U, //  24
  (uint8_t)1U, //  25
  (uint8_t)1U, //  26
  (uint8_t)2U, //  27
  (uint8_t)2U, //  28
  (uint8_t)2U, //  29
  (uint8_t)2U, //  30
  (uint8_t)2U, //  31
  (uint8_t)2U, //  32
  (uint8_t)2U, //  33
  (uint8_t)2U, //  34
  (uint8_t)2U, //  35
  (uint8_t)2U, //  36
  (uint8_t)2U, //  37
  (uint8_t)2U, //  38
  (uint8_t)2U, //  39
  (uint8_t)2U, //  40
  (uint8_t)2U, //  41: 2
  (uint8_t)2U, //  42
  (uint8_t)2U, //  43
  (uint8_t)2U, //  44
  (uint8_t)2U, //  45
  (uint8_t)2U, //  46
  (uint8_t)2U, //  47
  (uint8_t)2U, //  48
  (uint8_t)2U, //  49
  (uint8_t)2U, //  50
  (uint8_t)2U, //  51
  (uint8_t)2U, //  52
  (uint8_t)2U, //  53
  (uint8_t)2U, //  54
  (uint8_t)2U, //  55
  (uint8_t)2U, //  56
  (uint8_t)3U, //  57
  (uint8_t)3U, //  58
  (uint8_t)3U, //  59
  (uint8_t)3U, //  60
  (uint8_t)3U, //  61
  (uint8_t)3U, //  62
  (uint8_t)3U, //  63
  (uint8_t)3U, //  64
  (uint8_t)3U, //  65
  (uint8_t)3U, //  66
  (uint8_t)3U, //  67
  (uint8_t)3U, //  68
  (uint8_t)3U, //  69
  (uint8_t)3U, //  70
  (uint8_t)3U, //  71: 3
  (uint8_t)3U, //  72
  (uint8_t)3U, //  73
  (uint8_t)3U, //  74
  (uint8_t)3U, //  75
  (uint8_t)3U, //  76
  (uint8_t)3U, //  77
  (uint8_t)3U, //  78
  (uint8_t)3U, //  79
  (uint8_t)3U, //  80
  (uint8_t)3U, //  81
  (uint8_t)3U, //  82
  (uint8_t)3U, //  83
  (uint8_t)3U, //  84
  (uint8_t)3U, //  85
  (uint8_t)3U, //  86
  (uint8_t)3U, //  87
  (uint8_t)3U, //  88
  (uint8_t)3U, //  89
  (uint8_t)4U, //  90
  (uint8_t)4U, //  91
  (uint8_t)4U, //  92
  (uint8_t)4U, //  93
  (uint8_t)4U, //  94
  (uint8_t)4U, //  95
  (uint8_t)4U, //  96
  (uint8_t)4U, //  97
  (uint8_t)4U, //  98
  (uint8_t)4U, //  99
  (uint8_t)4U, // 100
  (uint8_t)4U, // 101
  (uint8_t)4U, // 102
  (uint8_t)4U, // 103
  (uint8_t)4U, // 104
  (uint8_t)4U, // 105
  (uint8_t)4U, // 106: 4
  (uint8_t)4U, // 107
  (uint8_t)4U, // 108
  (uint8_t)4U, // 109
  (uint8_t)4U, // 110
  (uint8_t)4U, // 111
  (uint8_t)4U, // 112
  (uint8_t)4U, // 113
  (uint8_t)4U, // 114
  (uint8_t)4U, // 115
  (uint8_t)4U, // 116
  (uint8_t)4U, // 117
  (uint8_t)5U, // 118
  (uint8_t)5U, // 119
  (uint8_t)5U, // 120
  (uint8_t)5U, // 121
  (uint8_t)5U, // 122
  (uint8_t)5U, // 123
  (uint8_t)5U, // 124
  (uint8_t)5U, // 125
  (uint8_t)5U, // 126
  (uint8_t)5U, // 127
  (uint8_t)5U, // 128
  (uint8_t)5U, // 129
  (uint8_t)5U, // 130
  (uint8_t)5U, // 131
  (uint8_t)5U, // 132
  (uint8_t)5U, // 133
  (uint8_t)5U, // 134
  (uint8_t)5U, // 135
  (uint8_t)5U, // 136
  (uint8_t)5U, // 137
  (uint8_t)5U, // 138
  (uint8_t)5U, // 139
  (uint8_t)5U, // 140
  (uint8_t)5U, // 141
  (uint8_t)5U, // 142: 5
  (uint8_t)5U, // 143
  (uint8_t)5U, // 144
  (uint8_t)5U, // 145
  (uint8_t)5U, // 146
  (uint8_t)5U, // 147
  (uint8_t)5U, // 148
  (uint8_t)5U, // 149
  (uint8_t)5U, // 150
  (uint8_t)5U, // 151
  (uint8_t)5U, // 152
  (uint8_t)5U, // 153
  (uint8_t)5U, // 154
  (uint8_t)5U, // 155
  (uint8_t)5U, // 156
  (uint8_t)5U, // 157
  (uint8_t)5U, // 158
  (uint8_t)5U, // 159
  (uint8_t)5U, // 160
  (uint8_t)5U, // 161
  (uint8_t)5U, // 162
  (uint8_t)5U, // 163
  (uint8_t)6U, // 164
  (uint8_t)6U, // 165
  (uint8_t)6U, // 166
  (uint8_t)6U, // 167
  (uint8_t)6U, // 168
  (uint8_t)6U, // 169
  (uint8_t)6U, // 170
  (uint8_t)6U, // 171
  (uint8_t)6U, // 172
  (uint8_t)6U, // 173
  (uint8_t)6U, // 174
  (uint8_t)6U, // 175
  (uint8_t)6U, // 176
  (uint8_t)6U, // 177
  (uint8_t)6U, // 178: 6
  (uint8_t)6U, // 179
  (uint8_t)6U, // 180
  (uint8_t)6U, // 181
  (uint8_t)6U, // 182
  (uint8_t)6U, // 183
  (uint8_t)6U, // 184
  (uint8_t)6U, // 185
  (uint8_t)6U, // 186
  (uint8_t)6U, // 187
  (uint8_t)6U, // 188
  (uint8_t)6U, // 189
  (uint8_t)6U, // 190
  (uint8_t)6U, // 191
  (uint8_t)7U, // 192
  (uint8_t)7U, // 193
  (uint8_t)7U, // 194
  (uint8_t)7U, // 195
  (uint8_t)7U, // 196
  (uint8_t)7U, // 197
  (uint8_t)7U, // 198
  (uint8_t)7U, // 199
  (uint8_t)7U, // 200
  (uint8_t)7U, // 201
  (uint8_t)7U, // 202
  (uint8_t)7U, // 203
  (uint8_t)7U, // 204
  (uint8_t)7U, // 205
  (uint8_t)7U, // 206
  (uint8_t)7U, // 207
  (uint8_t)7U, // 208: 7
  (uint8_t)7U, // 209
  (uint8_t)7U, // 210
  (uint8_t)7U, // 211
  (uint8_t)7U, // 212
  (uint8_t)7U, // 213
  (uint8_t)7U, // 214
  (uint8_t)7U, // 215
  (uint8_t)7U, // 216
  (uint8_t)7U, // 217
  (uint8_t)8U, // 218
  (uint8_t)8U, // 219
  (uint8_t)8U, // 220
  (uint8_t)8U, // 221
  (uint8_t)8U, // 222
  (uint8_t)8U, // 223
  (uint8_t)8U, // 224
  (uint8_t)8U, // 225
  (uint8_t)8U, // 226
  (uint8_t)8U, // 227
  (uint8_t)8U, // 228
  (uint8_t)8U, // 229: 8
  (uint8_t)8U, // 230
  (uint8_t)8U, // 231
  (uint8_t)8U, // 232
  (uint8_t)8U, // 233
  (uint8_t)8U, // 234
  (uint8_t)8U, // 235
  (uint8_t)8U, // 236
  (uint8_t)8U, // 237
  (uint8_t)8U, // 238
  (uint8_t)9U, // 239
  (uint8_t)9U, // 240
  (uint8_t)9U, // 241
  (uint8_t)9U, // 242
  (uint8_t)9U, // 243
  (uint8_t)9U, // 244
  (uint8_t)9U, // 245
  (uint8_t)9U, // 246
  (uint8_t)9U, // 247
  (uint8_t)9U, // 248
  (uint8_t)9U, // 249: 9
  (uint8_t)9U, // 250
  (uint8_t)9U, // 251
  (uint8_t)9U, // 252
  (uint8_t)9U, // 253
  (uint8_t)10U, // 254: 10
  (uint8_t)10U  // 255
};

// each character is encoded into an 8-bit byte
static const uint8_t morse_tab[] =
{
  0b0,               // space                     // No Morse
  0b01001010,        // ! exclamation
  0b01101101,        // " quotation
  0b01010111,        // # pound                   // No Morse, mapped to SK
  0b10110111,        // $ dollar or ~SX
  0b00000000,        // % percent
  0b00111101,        // & ampersand or ~AS
  0b01100001,        // ' apostrophe
  0b00110010,        // ( open paren
  0b01010010,        // ) close paren
  0b0,               // * asterisk                // No Morse
  0b00110101,        // + plus or ~AR
  0b01001100,        // , comma
  0b01011110,        // - hypen
  0b01010101,        // . period
  0b00110110,        // / slant
  0b00100000,        // 0                         // Read the bits from RIGHT to left,
  0b00100001,        // 1                         // with a "1"=dit and "0"=dah
  0b00100011,        // 2                         // example: 2 = 11000 or dit-dit-dah-dah-dah
  0b00100111,        // 3                         // the final bit is always 1 = stop bit.
  0b00101111,        // 4                         // see "sendElements" routine for more info.
  0b00111111,        // 5
  0b00111110,        // 6
  0b00111100,        // 7
  0b00111000,        // 8
  0b00110000,        // 9
  0b01111000,        // : colon
  0b01101010,        // ; semicolon
  0b0,               // <                         // No Morse
  0b00101110,        // = equals or ~BT
  0b0,               // >                         // No Morse
  0b01110011,        // ? question
  0b01101001,        // @ at or ~AC
  0b00000101,        // A
  0b00011110,        // B
  0b00011010,        // C
  0b00001110,        // D
  0b00000011,        // E
  0b00011011,        // F
  0b00001100,        // G
  0b00011111,        // H
  0b00000111,        // I
  0b00010001,        // J
  0b00001010,        // K
  0b00011101,        // L
  0b00000100,        // M
  0b00000110,        // N
  0b00001000,        // O
  0b00011001,        // P
  0b00010100,        // Q
  0b00001101,        // R
  0b00001111,        // S
  0b00000010,        // T
  0b00001011,        // U
  0b00010111,        // V
  0b00001001,        // W
  0b00010110,        // X
  0b00010010,        // Y
  0b00011100         // Z
};

static const int NUM_READINGS = 256;

static int32_t readings[NUM_READINGS];     // the readings from the analog input
static int32_t readIndex = 0;              // the index of the current reading
static int32_t total = 0;                  // the running total
static int32_t average = 0;                // the average

ResponsiveAnalogRead mainTune(MAIN_TUNE_PIN,true);
ResponsiveAnalogRead fineTune(FINE_TUNE_PIN,true);
ResponsiveAnalogRead bandSet(BAND_PIN,false);
Si5351 si5351;

volatile static uint32_t frequency = 0;

void setup(void)
{
  pinMode(BAND80_PIN,OUTPUT);
  pinMode(BAND40_PIN,OUTPUT);
  pinMode(BAND20_PIN,OUTPUT);
  pinMode(BAND15_PIN,OUTPUT);
  pinMode(BAND10_PIN,OUTPUT);
  pinMode(LED_SIGNAL_PIN,OUTPUT);
  digitalWrite(BAND80_PIN,LOW);
  digitalWrite(BAND40_PIN,LOW);
  digitalWrite(BAND20_PIN,LOW);
  digitalWrite(BAND15_PIN,LOW);
  digitalWrite(BAND10_PIN,LOW);
  digitalWrite(LED_SIGNAL_PIN,LOW);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN,LOW);
  delay(500);

  const bool si5351_found = si5351.init(SI5351_CRYSTAL_LOAD_0PF, 26000000UL, 0UL);
  if (!si5351_found)
  {
    for (;;)
    {
      digitalWrite(LED_BUILTIN,HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN,LOW);
      delay(500);
    }
  }
  si5351.set_freq(7041400UL*100ULL, SI5351_CLK0);
  mainTune.setAnalogResolution(4096);
  fineTune.setAnalogResolution(4096);
  bandSet.setAnalogResolution(4096);
  memset(readings,0,sizeof(readings));

  //Serial.begin(115200);
  //delay(5000);
}

static void set_band(const uint8_t band)
{
  static const uint8_t band_pin_map[SET_BANDS] =
  {
    BAND80_PIN,
    BAND40_PIN,
    BAND20_PIN,
    BAND20_PIN,
    BAND15_PIN,
    BAND15_PIN,
    BAND10_PIN,
    BAND10_PIN,
    BAND10_PIN,
    BAND10_PIN,
    BAND10_PIN
  };
  static uint8_t current_band = SET_BANDS;
  if (current_band==band || band>=SET_BANDS)
  {
    return;
  }
  current_band = band;
  digitalWrite(BAND80_PIN,LOW);
  digitalWrite(BAND40_PIN,LOW);
  digitalWrite(BAND20_PIN,LOW);
  digitalWrite(BAND15_PIN,LOW);
  digitalWrite(BAND10_PIN,LOW);
  digitalWrite(band_pin_map[current_band],HIGH);
  delay(100);
}

void loop1(void)
{
  static uint32_t morse_frequency;
  const uint32_t f = frequency;
  if (morse_frequency!=f)
  {
    morse_frequency = f;
    char cw[16] = "";
    memset(cw,0,sizeof(cw));
    sprintf(cw,"%u",morse_frequency/1000UL);
    char *p = cw;
    while (*p)
    {
      const char c = *p++;
      uint8_t morse = morse_tab[c-32];
      while (morse>1)
      {
        digitalWrite(LED_SIGNAL_PIN,HIGH);
        if (morse&1)
        {
          delay(DIT_PERIOD);
        }
        else
        {
          delay(DIT_PERIOD*3);
        }
        digitalWrite(LED_SIGNAL_PIN,LOW);
        delay(DIT_PERIOD);
        morse >>= 1;
      }
      delay(DIT_PERIOD*5);
    }
    delay(1000);
  }
}

void loop(void)
{
  static uint8_t band = 0;
  static int32_t fine_tune = 0;
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(MAIN_TUNE_PIN);
  total = total + readings[readIndex];
  readIndex++;
  if (readIndex >= NUM_READINGS)
  {
    readIndex = 0;
    fineTune.update(analogRead(FINE_TUNE_PIN)>>4);
    bandSet.update(analogRead(BAND_PIN)>>4);
    //Serial.printf("band: %u\n",bandSet.getValue()&0xff);
    band = band_map[bandSet.getValue()&0xff];
    fine_tune = ((int32_t)fineTune.getValue()-128L);
    fine_tune = fine_tune<-10?fine_tune+10:(fine_tune>10?fine_tune-10:0);
    fine_tune = constrain(fine_tune,-100,100)*5;
  }
  average = total / NUM_READINGS;
  mainTune.update(average);
  uint32_t f = bands[band].start + bands[band].step * mainTune.getValue();
  f -= f%1000;
  f += fine_tune;
  if (frequency!=f)
  {
    frequency = f;
    si5351.set_freq(frequency*SI5351_FREQ_MULT, SI5351_CLK0);
    //Serial.printf("ADC2, Core temperature: %i %i %u %2.1fC %i\n",analog.getRawValue(),analog.getValue(),f,analogReadTemp(),band);
    //delay(1000);
  }
  set_band(band);
}
