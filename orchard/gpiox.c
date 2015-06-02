
#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "gpiox.h"
#include "orchard.h"
#include "orchard-app.h"

#define REG_ID            0x01
#define REG_DIR           0x03
#define REG_OUT           0x05
#define REG_HIZ           0x07
#define REG_IRQ_LEVEL     0x09
#define REG_PULL_ENABLE   0x0b
#define REG_PULL_DIR      0x0d
#define REG_IN            0x0f
#define REG_IRQ_MASK      0x11
#define REG_IRQ_STATUS    0x13

event_source_t gpiox_rising[GPIOX_NUM_PADS];
event_source_t gpiox_falling[GPIOX_NUM_PADS];
static I2CDriver *driver;
static uint8_t gpiox_pal_mode[GPIOX_NUM_PADS];
static uint8_t regcache[10];

static void gpiox_poll_int(int ign);

static void gpiox_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, gpioxAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static void gpiox_sync(uint8_t reg) {

  gpiox_set(reg, regcache[reg / 2]);
}

static uint8_t gpiox_get(uint8_t reg) {

  const uint8_t tx[1] = {reg};
  uint8_t rx[1];

  i2cMasterTransmitTimeout(driver, gpioxAddr,
                           tx, sizeof(tx),
                           rx, sizeof(rx),
                           TIME_INFINITE);
  return rx[0];
}

static uint8_t gpiox_read_pad(void *port, int pad) {

  (void)port;
  return !!(gpiox_get(REG_IN) & (1 << pad));
}

#if ORCHARD_BOARD_REV == ORCHARD_REV_EVT1
/* Fake a GPIO by polling the GPIO pin */
static THD_WORKING_AREA(waGpioxPollThread, 192);
static THD_FUNCTION(gpiox_poll_thread, arg) {

  (void)arg;

  chRegSetThreadName("GPIOX poll thread");
  while (1) {
    chThdSleepMilliseconds(30);
    gpiox_poll_int(0);
  }

  return;
}
#endif

void gpioxStart(I2CDriver *i2cp) {

  unsigned int i;

  driver = i2cp;

  for (i = 0; i < ARRAY_SIZE(regcache); i++)
    regcache[i] = 0;

  for (i = 0; i < GPIOX_NUM_PADS; i++) {
    chEvtObjectInit(&gpiox_rising[i]);
    chEvtObjectInit(&gpiox_falling[i]);
  }

  i2cAcquireBus(driver);
  gpiox_set(REG_ID, 1);
  /* Wait for the reset to complete */
  while (!(gpiox_get(REG_ID) & (1 << 1)));
  i2cReleaseBus(driver);

#if ORCHARD_BOARD_REV == ORCHARD_REV_EVT1
  chThdCreateStatic(waGpioxPollThread, sizeof(waGpioxPollThread),
                    ORCHARD_APP_PRIO, gpiox_poll_thread, NULL);
#else
  evtTableHook(orchard_events, gpiox_rdy, gpiox_poll_int);
#endif
}

uint8_t gpioxGetDebug(uint8_t reg) {
  uint8_t ret;
  i2cAcquireBus(driver);
  ret = gpiox_get(reg);
  i2cReleaseBus(driver);

  return ret;
}

void gpioxSetPad(void *port, int pad) {

  (void)port;
  
  regcache[REG_OUT / 2] |= (1 << pad);
  i2cAcquireBus(driver);
  gpiox_sync(REG_OUT);
  i2cReleaseBus(driver);
}

void gpioxClearPad(void *port, int pad) {

  (void)port;
  
  regcache[REG_OUT / 2] &= ~(1 << pad);
  i2cAcquireBus(driver);
  gpiox_sync(REG_OUT);
  i2cReleaseBus(driver);
}

void gpioxTogglePad(void *port, int pad) {

  (void)port;
  
  regcache[REG_OUT / 2] ^= (1 << pad);
  i2cAcquireBus(driver);
  gpiox_sync(REG_OUT);
  i2cReleaseBus(driver);
}

void gpioxSetPadMode(void *port, int pad, int mode) {

  int bit = (1 << pad);

  (void)port;

  if (pad > GPIOX_NUM_PADS)
    return;

  gpiox_pal_mode[pad] = mode;

  /* Start out by masking the IRQ, to prevent spurrious interrupts */
  regcache[REG_IRQ_MASK / 2] |= bit;
  i2cAcquireBus(driver);
  gpiox_sync(REG_IRQ_MASK);

  switch (mode & GPIOX_IRQMASK) {

    case GPIOX_IRQ_RISING:
      regcache[REG_IRQ_MASK  / 2] &= ~bit;
      regcache[REG_IRQ_LEVEL / 2] &= ~bit;
      break;

    case GPIOX_IRQ_FALLING:
      regcache[REG_IRQ_MASK  / 2] &= ~bit;
      regcache[REG_IRQ_LEVEL / 2] |=  bit;
      break;

    case GPIOX_IRQ_BOTH:
      regcache[REG_IRQ_MASK  / 2] &= ~bit;
      if (gpiox_read_pad(port, pad))
        regcache[REG_IRQ_LEVEL / 2] |=  bit;
      else
        regcache[REG_IRQ_LEVEL / 2] &= ~bit;
      break;

    case GPIOX_IRQ_NONE:
    default:
      regcache[REG_IRQ_MASK / 2] |= bit;
      break;
  }

  switch (mode & GPIOX_DIRMASK) {

    case GPIOX_OUT_OPENDRAIN:
      regcache[REG_DIR / 2] |= bit;
      regcache[REG_HIZ / 2] |= bit;
      break;

    case GPIOX_OUT_PUSHPULL:
      regcache[REG_DIR / 2] |= bit;
      regcache[REG_HIZ / 2] &= ~bit;
      break;

    case GPIOX_IN:
    default:
      regcache[REG_DIR / 2] &= ~bit;
      regcache[REG_HIZ / 2] &= ~bit;
      break;
  }

  if (mode & GPIOX_VAL_HIGH)
    regcache[REG_OUT / 2] |= bit;
  else
    regcache[REG_OUT / 2] &= ~bit;

  switch (mode & GPIOX_PULLMASK) {

    case GPIOX_PULL_UP:
      regcache[REG_PULL_ENABLE / 2] |= bit;
      regcache[REG_PULL_DIR    / 2] |= bit;
      break;

    case GPIOX_PULL_DOWN:
      regcache[REG_PULL_ENABLE / 2] |= bit;
      regcache[REG_PULL_DIR    / 2] &= ~bit;
      break;

    case GPIOX_PULL_NONE:
    default:
      regcache[REG_PULL_ENABLE / 2] &= ~bit;
      regcache[REG_PULL_DIR    / 2] &= ~bit;
      break;
  }

  gpiox_sync(REG_OUT);
  gpiox_sync(REG_HIZ);
  gpiox_sync(REG_PULL_DIR);
  gpiox_sync(REG_PULL_ENABLE);
  gpiox_sync(REG_DIR);
  gpiox_sync(REG_IRQ_LEVEL);
  gpiox_sync(REG_IRQ_MASK);

  i2cReleaseBus(driver);
}

uint8_t gpioxReadPad(void *port, int pad) {

  (void)port;
  uint8_t val;
  
  i2cAcquireBus(driver);
  val = gpiox_read_pad(port, pad);
  i2cReleaseBus(driver);

  return val;
}


static void irq_check_and_broadcast(uint8_t state, int gpio) {

  int bit = 1 << gpio;

  if (!(state & bit))
    return;

  /* If the "level" bit is set and the IRQ fired, then that means that
   * the pin fell. */
  if (regcache[REG_IRQ_LEVEL / 2] & bit) {

    /* "Mask" the GPIO */
    regcache[REG_IRQ_LEVEL / 2] &= ~bit;

    if (gpiox_pal_mode[gpio] & GPIOX_IRQ_FALLING)
      chEvtBroadcast(&gpiox_falling[gpio]);
  }
  else {
    /* "Mask" the GPIO */
    regcache[REG_IRQ_LEVEL / 2] |= bit;

    if (gpiox_pal_mode[gpio] & GPIOX_IRQ_RISING)
      chEvtBroadcast(&gpiox_rising[gpio]);
  }
}

static uint32_t false_interrupts;
static uint32_t real_interrupts;
static void gpiox_poll_int(int ign) {

  (void)ign;
  int pad;
  uint8_t irq_state;
  int pal_level;

#if ORCHARD_BOARD_REV == ORCHARD_REV_EVT1
  /* Original board rev had the GPIO connected to a non-IRQ line */
  pal_level = palReadPad(GPIOB, 0);
#else
  pal_level = palReadPad(GPIOD, 4);
#endif

  if (pal_level)
    goto out;

  i2cAcquireBus(driver);
  irq_state = gpiox_get(REG_IRQ_STATUS);
  i2cReleaseBus(driver);

  if (!irq_state && !pal_level)
    false_interrupts++;
  else
    real_interrupts++;

  for (pad = 0; pad < GPIOX_NUM_PADS; pad++)
    irq_check_and_broadcast(irq_state, pad);

  /* Acknowledge the IRQ by writing new values to watch */
  i2cAcquireBus(driver);
  /* Mask off the pins that caused this interrupt, while we re-set the level */
  gpiox_set(REG_IRQ_MASK, regcache[REG_IRQ_MASK / 2] | irq_state);
  gpiox_sync(REG_IRQ_LEVEL);
  /* Unmask the new pins */
  gpiox_sync(REG_IRQ_MASK);
  i2cReleaseBus(driver);

out:
  return;
}
