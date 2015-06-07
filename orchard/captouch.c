#include "ch.h"
#include "hal.h"
#include "i2c.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-events.h"
#include "orchard-app.h"

#include "captouch.h"
#include "gpiox.h"
#include "orchard-test.h"
#include "test-audit.h"

#include <stdlib.h>

static I2CDriver *driver;
event_source_t captouch_changed;
static uint16_t captouch_state;

static void captouch_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, touchAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static uint8_t captouch_get(uint8_t reg) {

  uint8_t val;

  i2cMasterTransmitTimeout(driver, touchAddr,
                           &reg, 1,
                           (void *)&val, 1,
                           TIME_INFINITE);
  return val;
}

static uint16_t captouch_read(void) {

  uint16_t val;
  uint8_t reg;

  reg = ELE_TCHL;
  i2cMasterTransmitTimeout(driver, touchAddr,
                           &reg, 1,
                           (void *)&val, 2,
                           TIME_INFINITE);
  return val;
}

// Configure all registers as described in AN3944
static void captouch_config(void) {

  captouch_set(ELE_CFG, 0x00);  // register updates only allowed when in stop mode

  // Section A
  // This group controls filtering when data is > baseline.
  captouch_set(MHD_R, 0x01);
  captouch_set(NHD_R, 0x01);
  captouch_set(NCL_R, 0x03);
  captouch_set(FDL_R, 0x02);

  // Section B
  // This group controls filtering when data is < baseline.
  captouch_set(MHD_F, 0x01);
  captouch_set(NHD_F, 0x01);
  captouch_set(NCL_F, 0x04);
  captouch_set(FDL_F, 0x02);

  // Section C
  // This group sets touch and release thresholds for each electrode
#if KEY_LAYOUT == LAYOUT_BC1
  captouch_set(ELE0_T, 18); // outer electrodes have an offset
  captouch_set(ELE0_R, 25);
#else
  captouch_set(ELE0_T, TOU_THRESH);
  captouch_set(ELE0_R, REL_THRESH);
#endif
  captouch_set(ELE1_T, TOU_THRESH);
  captouch_set(ELE1_R, REL_THRESH);
  captouch_set(ELE2_T, TOU_THRESH);
  captouch_set(ELE2_R, REL_THRESH);
  captouch_set(ELE3_T, TOU_THRESH);
  captouch_set(ELE3_R, REL_THRESH);
  captouch_set(ELE4_T, TOU_THRESH);
  captouch_set(ELE4_R, REL_THRESH);
  captouch_set(ELE5_T, TOU_THRESH);
  captouch_set(ELE5_R, REL_THRESH);
  captouch_set(ELE6_T, TOU_THRESH);
  captouch_set(ELE6_R, REL_THRESH);
  captouch_set(ELE7_T, TOU_THRESH);
  captouch_set(ELE7_R, REL_THRESH);
  captouch_set(ELE8_T, TOU_THRESH);
  captouch_set(ELE8_R, REL_THRESH);
  captouch_set(ELE9_T, TOU_THRESH);
  captouch_set(ELE9_R, REL_THRESH);
  captouch_set(ELE10_T, TOU_THRESH);
  captouch_set(ELE10_R, REL_THRESH);

#if KEY_LAYOUT == LAYOUT_BC1
  captouch_set(ELE11_T, 18);
  captouch_set(ELE11_R, 25);
#else
  captouch_set(ELE11_T, TOU_THRESH);
  captouch_set(ELE11_R, REL_THRESH);
#endif
  // set debounce requirements
  captouch_set(TCH_DBNC, 0x11); 

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  captouch_set(FIL_CFG, 0x24);

  // Section F
  // Enable Auto Config and auto Reconfig
  captouch_set(ATO_CFG_CTL0, 0x0B);

  captouch_set(ATO_CFG_USL, 0xC4);  // USL 196
  captouch_set(ATO_CFG_LSL, 0x7F);  // LSL 127
  captouch_set(ATO_CFG_TGT, 0xB0);  // target level 176

  // Section E
  // Electrode Configuration
  // Enable 6 Electrodes and set to run mode
  // Set ELE_CFG to 0x00 to return to standby mode
  captouch_set(ELE_CFG, 0x0C);  // Enables all 12 Electrodes

}

void captouchFastBaseline(void) {
  uint8_t ret;
  uint16_t electrode;
  uint8_t i, j;

  captouch_set(ELE_CFG, 0x00);  // disable all electrodes

  for( i = 0x4, j = 0x1e; i < 0x1b; i+= 2, j++ ) {
    electrode = 0;
    ret = captouchGet(i);
    electrode = (uint16_t) ret;
    ret = captouchGet(i+1);
    electrode |= (ret << 8);

    captouchSet(j, (uint8_t) (electrode >> 2));
  }
  
  captouch_set(ELE_CFG, 0x0C);  // enable all electrodes
  chThdSleepMilliseconds(50);

  captouch_state = 0x0; // clear captouch state, it'll be all wrong now
}

static void captouch_keychange(void *port, int irq, int type) {

  uint16_t mask;
  bool changed = false;

  (void)port;
  (void)irq;
  (void)type;

  i2cAcquireBus(driver);
  mask = captouch_read();
  i2cReleaseBus(driver);

  if (captouch_state != mask)
    changed = true;
  captouch_state = mask;

  if (changed)
    chEvtBroadcast(&captouch_changed);
}

uint16_t captouchRead(void) {
  return captouch_state;
}

void captouchStop() {
  i2cAcquireBus(driver);
  captouch_set(ELE_CFG, 0x00);  // disabling all electrodes puts us in stop mode
  i2cReleaseBus(driver);
}

void captouchStart(I2CDriver *i2cp) {

  driver = i2cp;

  i2cAcquireBus(driver);
  captouch_config();
  i2cReleaseBus(driver);

  chEvtObjectInit(&captouch_changed);

  gpioxSetPadMode(GPIOX, 7, GPIOX_IN | GPIOX_IRQ_BOTH | GPIOX_PULL_UP);
  gpioxRegisterHandler(GPIOX, 7, captouch_keychange);
}

static void dump(uint8_t *byte, uint32_t count) {
  uint32_t i;

  for( i = 0; i < count; i++ ) {
    if( (i % 16) == 0 ) {
      chprintf(stream, "\n\r%08x: ", i );
    }
    chprintf(stream, "%02x ", byte[i] );
  }
  chprintf(stream, "\n\r" );
}

void captouchPrint(uint8_t reg) {
  uint8_t val;

  i2cAcquireBus(driver);
  val = captouch_get(reg);
  i2cReleaseBus(driver);
  
  chprintf( stream, "Value at %02x: %02x\n\r", reg, val );
}

uint8_t captouchGet(uint8_t reg) {
  uint8_t val;

  i2cAcquireBus(driver);
  val = captouch_get(reg);
  i2cReleaseBus(driver);
  
  return val;
}

void captouchSet(uint8_t adr, uint8_t dat) {
  //  chprintf( stream, "Writing %02x into %02x\n\r", dat, adr );

  i2cAcquireBus(driver);
  captouch_set(adr, dat);
  i2cReleaseBus(driver);
}

void captouchRecal(void) {
  i2cAcquireBus(driver);
  captouch_set(0x80, 0x63); // resets the chip
  chThdSleepMilliseconds(50);
  captouch_set(0x80, 0x00); // is this necessary??
  chThdSleepMilliseconds(50);

  captouch_config();
  i2cReleaseBus(driver);
}

void captouchDebug(void) {
  uint8_t i;
  uint8_t val[128];

  for( i = 0; i < 128; i++ ) {
    i2cAcquireBus(driver);
    val[i] = captouch_get(i);
    i2cReleaseBus(driver);
  }
  dump( val, 128 );

}

#define CAL_RETRY_LIMIT 5
void captouchCalibrate(void) {
  coord_t width;
  coord_t height;
  font_t font;
  uint8_t i;
  char attemptstr[16];

  orchardGfxStart();
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  
  gdispDrawStringBox(0, height * 2, width, height,
                     "Calibrating", font, White, justifyCenter);
  gdispDrawStringBox(0, height * 3, width, height,
                     "touch surfaces:", font, White, justifyCenter);
  gdispDrawStringBox(0, height * 4, width, height,
                     "Please do not touch", font, White, justifyCenter);
  gdispDrawStringBox(0, height * 5, width, height,
                     "the touch surfaces!", font, White, justifyCenter);
  
  gdispFlush();
  i = 0;
  while( i < CAL_RETRY_LIMIT ) {
    chsnprintf(attemptstr, 16, "Attempt %d of %d", i+1, CAL_RETRY_LIMIT);
    gdispFillArea(0, height*6, width, height, Black);
    gdispDrawStringBox(0, height * 6, width, height,
		       attemptstr, font, White, justifyCenter);
    gdispFlush();
    captouchFastBaseline();
    chThdSleepMilliseconds(1000);

    i2cAcquireBus(driver);
    captouch_state = captouch_read();
    i2cReleaseBus(driver);

    if( captouch_state == 0x0 )
      break;

    i++;
  }

  gdispCloseFont(font);
  gdispFlush();
  orchardGfxEnd();
}

OrchardTestResult test_captouch(const char *my_name, OrchardTestType test_type) {

  switch(test_type) {
  default:
    return orchardResultNoTest;
  }
  
  return orchardResultNoTest;
}
orchard_test("captouch", test_captouch);

/*
This cal set works through the touch surface:

00000000: 00 00 00 00 D9 02 C8 02 C4 02 C5 02 C7 02 CB 02 
00000010: CA 02 C7 02 C4 02 CF 02 C5 02 C4 02 00 00 B6 B1 
00000020: B0 B1 B1 B2 B2 B1 B0 B3 B1 B0 00 01 01 00 00 01 
00000030: 01 FF 02 00 00 00 00 00 00 00 00 00 00 00 00 00 
00000040: 00 0F 2A 0F 2A 0F 2A 0F 2A 0F 2A 0F 2A 0F 2A 0F 
00000050: 2A 0F 2A 0F 2A 0F 2A 0F 2A 00 00 00 10 04 0C 27 
00000060: 2D 2A 28 28 2F 2F 28 29 2E 2E 27 00 11 11 11 11 
00000070: 11 11 00 00 00 00 00 00 00 00 00 0B 00 C4 7F B0 

This cal set is even better:
00000000: 00 00 00 00 B4 02 C1 02 BE 02 BF 02 C3 02 BD 02 
00000010: BB 02 B4 02 BE 02 C2 02 BF 02 BD 02 00 00 AC B0 
00000020: AF AF B0 AF AE AD AF B0 AF AF 00 01 01 00 00 01 
00000030: 01 FF 02 00 00 00 00 00 00 00 00 00 00 00 00 00 
00000040: 00 06 0B 06 0B 06 0B 06 0B 06 0B 06 0B 06 0B 06 
00000050: 0B 06 0B 06 0B 06 0B 06 0B 00 00 11 10 24 0C 25 
00000060: 2D 2A 28 28 2E 2E 27 29 2E 2E 27 00 11 11 11 11 
00000070: 11 11 00 00 00 00 00 00 00 00 00 0B 00 C4 7F B0 

00000000: 00 00 00 00 B4 02 C1 02 BE 02 BF 02 C3 02 BD 02 
00000010: BC 02 B5 02 BE 02 C2 02 BF 02 BD 02 00 00 AD B0 
00000020: AF AF B0 AF AE AD AF B0 AF AF 00 01 01 00 00 01 
00000030: 01 FF 02 00 00 00 00 00 00 00 00 00 00 00 00 00 
00000040: 00 06 0B 06 0B 06 0B 06 0B 06 0B 06 0B 06 0B 06 
00000050: 0B 06 0B 06 0B 06 0B 06 0B 00 00 11 10 24 0C 25 
00000060: 2D 2A 28 28 2E 2E 27 29 2E 2E 27 00 11 11 11 11 
00000070: 11 11 00 00 00 00 00 00 00 00 00 0B 00 C4 7F B0 
 */
