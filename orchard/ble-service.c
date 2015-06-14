#include "ble-service.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

static const struct ble_service_message broadcast_messages[] = {
{0x00,
        {
            0x07,0x06,0x00,0x00,0x03,0x02,0x41,0xd7,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x01,0x01,0x00,0x00,0x06,0x00,0x01,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x10,0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x90,0x00,0x64,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x10,0x38,0x02,0xff,0x02,0x58,0x00,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        },
    },
    {0x00,
        {
            0x05,0x06,0x10,0x54,0x00,0x00,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x20,0x00,0x04,0x04,0x02,0x02,0x00,0x01,0x28,0x00,0x01,0x00,0x18,0x04,0x04,0x05,0x05,0x00,
            0x02,0x28,0x03,0x01,0x0e,0x03,0x00,0x00,0x2a,0x04,0x14,0x0b,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x20,0x1c,0x05,0x00,0x03,0x2a,0x00,0x01,0x62,0x63,0x61,0x73,0x74,0x63,0x73,0x65,0x6d,0x69,
            0x2e,0x04,0x04,0x05,0x05,0x00,0x04,0x28,0x03,0x01,0x02,0x05,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x20,0x38,0x00,0x01,0x2a,0x06,0x04,0x03,0x02,0x00,0x05,0x2a,0x01,0x01,0x00,0x00,0x04,0x04,
            0x05,0x05,0x00,0x06,0x28,0x03,0x01,0x02,0x07,0x00,0x04,0x2a,
        },
    },
    {0x00,
        {
            0x1f,0x06,0x20,0x54,0x06,0x04,0x09,0x08,0x00,0x07,0x2a,0x04,0x01,0xff,0xff,0xff,0xff,0x00,0x00,0xff,
            0xff,0x04,0x04,0x02,0x02,0x00,0x08,0x28,0x00,0x01,0x01,0x18,
        },
    },
    {0x00,
        {
            0x04,0x06,0x20,0x70,0x00,
        },
    },
    {0x00,
        {
            0x0d,0x06,0x40,0x00,0x2a,0x00,0x01,0x00,0x80,0x04,0x00,0x03,0x00,0x00,
        },
    },
    {0x00,
        {
            0x06,0x06,0x60,0x00,0x00,0x00,0x00,
        },
    },
    {0x00,
        {
            0x06,0x06,0xf0,0x00,0x03,0x4c,0xf2,
        },
    },
};

static const struct ble_service_message hid_keyboard_messages[] = {
  {0x00,
    {
      0x07, 0x06, 0x00, 0x00, 0x03, 0x02, 0x42, 0x07,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x10, 0x00, 0x3a, 0x68, 0x00, 0x00,
      0x02, 0x00, 0x0c, 0x00, 0x10, 0x01, 0x01, 0x00,
      0x00, 0x02, 0x00, 0x04, 0x81, 0x12, 0x18, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x10, 0x1c, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x12,
      0x00, 0x00, 0x00, 0x00, 0x03, 0x99, 0x00, 0x64,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x10, 0x38, 0x02, 0xff, 0x02, 0x58,
      0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x05, 0x06, 0x10, 0x54, 0x01, 0x01,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0x00, 0x04, 0x04, 0x02, 0x02,
      0x00, 0x01, 0x28, 0x00, 0x01, 0x00, 0x18, 0x04,
      0x04, 0x05, 0x05, 0x00, 0x02, 0x28, 0x03, 0x01,
      0x0e, 0x03, 0x00, 0x00, 0x2a, 0x04, 0x34, 0x11,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0x1c, 0x0f, 0x00, 0x03, 0x2a,
      0x00, 0x01, 0x4b, 0x65, 0x79, 0x62, 0x6f, 0x61,
      0x72, 0x64, 0x20, 0x4e, 0x6f, 0x72, 0x64, 0x69,
      0x63, 0x00, 0x00, 0x04, 0x04, 0x05, 0x05, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0x38, 0x04, 0x28, 0x03, 0x01,
      0x02, 0x05, 0x00, 0x01, 0x2a, 0x06, 0x04, 0x03,
      0x02, 0x00, 0x05, 0x2a, 0x01, 0x01, 0xc1, 0x03,
      0x04, 0x04, 0x05, 0x05, 0x00, 0x06, 0x28, 0x03,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0x54, 0x01, 0x02, 0x07, 0x00,
      0x04, 0x2a, 0x06, 0x04, 0x09, 0x08, 0x00, 0x07,
      0x2a, 0x04, 0x01, 0x06, 0x00, 0x10, 0x00, 0x19,
      0x00, 0x2c, 0x01, 0x04, 0x04, 0x02, 0x02, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0x70, 0x08, 0x28, 0x00, 0x01,
      0x01, 0x18, 0x04, 0x04, 0x02, 0x02, 0x00, 0x09,
      0x28, 0x00, 0x01, 0x12, 0x18, 0x04, 0x04, 0x05,
      0x05, 0x00, 0x0a, 0x28, 0x03, 0x01, 0x12, 0x0b,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0x8c, 0x00, 0x4d, 0x2a, 0x14,
      0x0c, 0x08, 0x00, 0x00, 0x0b, 0x2a, 0x4d, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x46, 0x34, 0x03, 0x02, 0x00, 0x0c, 0x29, 0x02,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0xa8, 0x01, 0x00, 0x00, 0x06,
      0x0c, 0x03, 0x02, 0x00, 0x0d, 0x29, 0x08, 0x01,
      0x00, 0x01, 0x04, 0x04, 0x05, 0x05, 0x00, 0x0e,
      0x28, 0x03, 0x01, 0x02, 0x0f, 0x00, 0x4a, 0x2a,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0xc4, 0x06, 0x0c, 0x05, 0x04,
      0x00, 0x0f, 0x2a, 0x4a, 0x01, 0x11, 0x01, 0x00,
      0x03, 0x04, 0x04, 0x05, 0x05, 0x00, 0x10, 0x28,
      0x03, 0x01, 0x06, 0x11, 0x00, 0x4e, 0x2a, 0x46,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0xe0, 0x3c, 0x02, 0x01, 0x00,
      0x11, 0x2a, 0x4e, 0x01, 0x01, 0x04, 0x04, 0x05,
      0x05, 0x00, 0x12, 0x28, 0x03, 0x01, 0x04, 0x13,
      0x00, 0x4c, 0x2a, 0x46, 0x30, 0x02, 0x01, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x20, 0xfc, 0x13, 0x2a, 0x4c, 0x01,
      0x00, 0x04, 0x04, 0x05, 0x05, 0x00, 0x14, 0x28,
      0x03, 0x01, 0x02, 0x15, 0x00, 0x4b, 0x2a, 0x06,
      0x0c, 0x4d, 0x4c, 0x00, 0x15, 0x2a, 0x4b, 0x01,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0x18, 0x05, 0x01, 0x09, 0x06,
      0xa1, 0x01, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7,
      0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08,
      0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0x34, 0x95, 0x05, 0x75, 0x01,
      0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02,
      0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06,
      0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0x50, 0x19, 0x00, 0x29, 0x65,
      0x81, 0x00, 0x09, 0x05, 0x15, 0x00, 0x26, 0xff,
      0x00, 0x75, 0x08, 0x95, 0x02, 0xb1, 0x02, 0xc0,
      0x04, 0x04, 0x05, 0x05, 0x00, 0x16, 0x28, 0x03,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0x6c, 0x01, 0x0e, 0x17, 0x00,
      0x4d, 0x2a, 0x44, 0x3c, 0x01, 0x01, 0x00, 0x17,
      0x2a, 0x4d, 0x01, 0x00, 0x06, 0x0c, 0x03, 0x02,
      0x00, 0x18, 0x29, 0x08, 0x01, 0x00, 0x02, 0x04,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0x88, 0x04, 0x05, 0x05, 0x00,
      0x19, 0x28, 0x03, 0x01, 0x12, 0x1a, 0x00, 0x22,
      0x2a, 0x16, 0x0c, 0x09, 0x08, 0x00, 0x1a, 0x2a,
      0x22, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0xa4, 0x00, 0x00, 0x46, 0x34,
      0x03, 0x02, 0x00, 0x1b, 0x29, 0x02, 0x01, 0x00,
      0x00, 0x04, 0x04, 0x05, 0x05, 0x00, 0x1c, 0x28,
      0x03, 0x01, 0x0e, 0x1d, 0x00, 0x32, 0x2a, 0x46,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0xc0, 0x3c, 0x02, 0x01, 0x00,
      0x1d, 0x2a, 0x32, 0x01, 0x00, 0x04, 0x04, 0x02,
      0x02, 0x00, 0x1e, 0x28, 0x00, 0x01, 0x0f, 0x18,
      0x04, 0x04, 0x05, 0x05, 0x00, 0x1f, 0x28, 0x03,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0xdc, 0x01, 0x02, 0x20, 0x00,
      0x19, 0x2a, 0x06, 0x0c, 0x02, 0x01, 0x00, 0x20,
      0x2a, 0x19, 0x01, 0x32, 0x04, 0x04, 0x05, 0x05,
      0x00, 0x21, 0x28, 0x03, 0x01, 0x02, 0x22, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x21, 0xf8, 0x1a, 0x2a, 0x06, 0x0c,
      0x02, 0x01, 0x00, 0x22, 0x2a, 0x1a, 0x01, 0x2f,
      0x04, 0x04, 0x02, 0x02, 0x00, 0x23, 0x28, 0x00,
      0x01, 0x13, 0x18, 0x04, 0x04, 0x05, 0x05, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x22, 0x14, 0x24, 0x28, 0x03, 0x01,
      0x04, 0x25, 0x00, 0x4f, 0x2a, 0x46, 0x30, 0x05,
      0x04, 0x00, 0x25, 0x2a, 0x4f, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x04, 0x04, 0x05, 0x05, 0x00, 0x26,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x22, 0x30, 0x28, 0x03, 0x01, 0x10,
      0x27, 0x00, 0x31, 0x2a, 0x14, 0x00, 0x01, 0x00,
      0x00, 0x27, 0x2a, 0x31, 0x01, 0x00, 0x46, 0x34,
      0x03, 0x02, 0x00, 0x28, 0x29, 0x02, 0x01, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x22, 0x4c, 0x00, 0x04, 0x04, 0x02,
      0x02, 0x00, 0x29, 0x28, 0x00, 0x01, 0x0a, 0x18,
      0x04, 0x04, 0x05, 0x05, 0x00, 0x2a, 0x28, 0x03,
      0x01, 0x02, 0x2b, 0x00, 0x50, 0x2a, 0x06, 0x0c,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x22, 0x68, 0x08, 0x07, 0x00, 0x2b,
      0x2a, 0x50, 0x01, 0x02, 0x15, 0x19, 0xee, 0xee,
      0x00, 0x01, 0x04, 0x04, 0x05, 0x05, 0x00, 0x2c,
      0x28, 0x03, 0x01, 0x02, 0x2d, 0x00, 0x27, 0x2a,
    },
  },
  {0x00,
    {
      0x16, 0x06, 0x22, 0x84, 0x04, 0x0c, 0x09, 0x00,
      0x00, 0x2d, 0x2a, 0x27, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x40, 0x00, 0x2a, 0x00, 0x01, 0x00,
      0x80, 0x04, 0x00, 0x03, 0x00, 0x00, 0x2a, 0x4d,
      0x01, 0x00, 0x82, 0x04, 0x00, 0x0b, 0x00, 0x0c,
      0x2a, 0x4e, 0x01, 0x00, 0x88, 0x04, 0x00, 0x11,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x40, 0x1c, 0x00, 0x00, 0x2a, 0x4c,
      0x01, 0x00, 0x08, 0x04, 0x00, 0x13, 0x00, 0x00,
      0x2a, 0x4d, 0x01, 0x04, 0x08, 0x04, 0x00, 0x17,
      0x00, 0x00, 0x2a, 0x22, 0x01, 0x00, 0x02, 0x04,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x40, 0x38, 0x00, 0x1a, 0x00, 0x1b,
      0x2a, 0x32, 0x01, 0x04, 0x08, 0x04, 0x00, 0x1d,
      0x00, 0x00, 0x2a, 0x19, 0x01, 0x00, 0x80, 0x04,
      0x00, 0x20, 0x00, 0x00, 0x2a, 0x1a, 0x01, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x40, 0x54, 0x80, 0x04, 0x00, 0x22,
      0x00, 0x00, 0x2a, 0x4f, 0x01, 0x00, 0x08, 0x04,
      0x00, 0x25, 0x00, 0x00, 0x2a, 0x31, 0x01, 0x00,
      0x02, 0x04, 0x00, 0x27, 0x00, 0x28, 0x2a, 0x27,
    },
  },
  {0x00,
    {
      0x0b, 0x06, 0x40, 0x70, 0x01, 0x00, 0x80, 0x04,
      0x00, 0x2d, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x1f, 0x06, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x0b, 0x06, 0x60, 0x1c, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
    },
  },
  {0x00,
    {
      0x19, 0x06, 0x70, 0x00, 0x19, 0x02, 0xc1, 0x03,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00,
    },
  },
  {0x00,
    {
      0x06, 0x06, 0xf0, 0x00, 0x03, 0xc4, 0xee,
    },
  },
};

const struct ble_service hid_keyboard = {
 .count = ARRAY_SIZE(hid_keyboard_messages),
 .data  = hid_keyboard_messages,
};

const struct ble_service ble_broadcast = {
 .count = ARRAY_SIZE(broadcast_messages),
 .data  = broadcast_messages,
};