#include "ch.h"
#include "hal.h"
#include "issp.h"

/**
 *       (M6) - PC8;
 *       (M5) - PC9;
 *       (M4) - PB6;
 * XRES  (M3) - PB7;
 * SDATA (M2) - PB8;
 * SCLK  (M1) - PB9;
 */

#define XRES_CLK_DELAY          (10)
#define TRANSITION_TIMEOUT      (0x00020000)

// ID for CY8C21434
const uint8_t target_id_v[] = {0x00, 0x38};

const uint16_t num_bits_checksum = 286;
const uint8_t checksum_v[] =
{
  0xDE, 0xE0, 0x1F, 0x7B, 0x00, 0x79, 0xF0, 0x75,
  0xE7, 0xC8, 0x1F, 0xDE, 0xA0, 0x1F, 0x7A, 0x01,
  0xF9, 0xF7, 0x01, 0xF7, 0xC9, 0x87, 0xDF, 0x48,
  0x1E, 0x7D, 0x00, 0x7D, 0xE0, 0x0F, 0xF7, 0xC0,
  0x07, 0xDF, 0xE2, 0x5C

//0b11011110,0b11100000,0b00011111,0b01111011,0b00000000,0b01111001,0b11110000,0b01110101,
//0b11100111,0b11001000,0b00011111,0b11011110,0b10100000,0b00011111,0b01111010,0b00000001,
//0b11111001,0b11110111,0b00000001,0b11110111,0b11001001,0b10000111,0b11011111,0b01001000,
//0b00011110,0b01111101,0b00000000,0b01111101,0b11100000,0b00001111,0b11110111,0b11000000,
//0b00000111,0b11011111,0b11100010,0b01011100
};

const uint16_t num_bits_program_block = 308;
const uint8_t program_block[] =
{
  0x9F, 0x8A, 0x9E, 0x7F, 0x2B, 0x7D, 0xEE, 0x01,
  0xF7, 0xB0, 0x07, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
  0xFD, 0xEA, 0x01, 0xF7, 0xA0, 0x1F, 0x9F, 0x70,
  0x1F, 0x7C, 0x98, 0x7D, 0xF4, 0x81, 0xF7, 0x80,
  0x17, 0xDF, 0x00, 0x1F, 0x7F, 0x89, 0x70

//0b10011111,0b10001010,0b10011110,0b01111111,0b00101011,0b01111101,0b11101110,0b00000001,
//0b11110111,0b10110000,0b00000111,0b10011111,0b00000111,0b01011110,0b01111100,0b10000001,
//0b11111101,0b11101010,0b00000001,0b11110111,0b10100000,0b00011111,0b10011111,0b01110000,
//0b00011111,0b01111100,0b10011000,0b01111101,0b11110100,0b10000001,0b11110111,0b10000000,
//0b00010111,0b11011111,0b00000000,0b00011111,0b01111111,0b10001001,0b01110000
};

const uint16_t num_bits_init1 = 396;
const uint8_t init1_v[] =
{
  0xCA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0D, 0xEE, 0x01, 0xF7, 0xB0, 0x07, 0x9F, 0x07,
  0x5E, 0x7C, 0x81, 0xFD, 0xEA, 0x01, 0xF7, 0xA0,
  0x1F, 0x9F, 0x70, 0x1F, 0x7C, 0x98, 0x7D, 0xF4,
  0x81, 0xF7, 0x80, 0x4F, 0xDF, 0x00, 0x1F, 0x7F,
  0x89, 0x70

//0b11001010,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
//0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
//0b00001101,0b11101110,0b00000001,0b11110111,0b10110000,0b00000111,0b10011111,0b00000111,
//0b01011110,0b01111100,0b10000001,0b11111101,0b11101010,0b00000001,0b11110111,0b10100000,
//0b00011111,0b10011111,0b01110000,0b00011111,0b01111100,0b10011000,0b01111101,0b11110100,
//0b10000001,0b11110111,0b10000000,0b01001111,0b11011111,0b00000000,0b00011111,0b01111111,
//0b10001001,0b01110000
};

const uint16_t num_bits_init2 = 286;
const uint8_t init2_v[] =
{
  0xDE, 0xE0, 0x1F, 0x7B, 0x00, 0x79, 0xF0, 0x75,
  0xE7, 0xC8, 0x1F, 0xDE, 0xA0, 0x1F, 0x7A, 0x01,
  0xF9, 0xF7, 0x01, 0xF7, 0xC9, 0x87, 0xDF, 0x48,
  0x1E, 0x7D, 0x00, 0xFD, 0xE0, 0x0D, 0xF7, 0xC0,
  0x07, 0xDF, 0xE2, 0x5C

//0b11011110,0b11100000,0b00011111,0b01111011,0b00000000,0b01111001,0b11110000,0b01110101,
//0b11100111,0b11001000,0b00011111,0b11011110,0b10100000,0b00011111,0b01111010,0b00000001,
//0b11111001,0b11110111,0b00000001,0b11110111,0b11001001,0b10000111,0b11011111,0b01001000,
//0b00011110,0b01111101,0b00000000,0b11111101,0b11100000,0b00001101,0b11110111,0b11000000,
//0b00000111,0b11011111,0b11100010,0b01011100
};

const uint16_t num_bits_init3_3v = 836;
const uint8_t init3_3v[] =
{
  0xDE, 0xE0, 0x1F, 0x7A, 0x01, 0xFD, 0xEA, 0x01,
  0xF7, 0xB0, 0x47, 0xDF, 0x0A, 0x3F, 0x7C, 0xFC,
  0x7D, 0xF4, 0x61, 0xF7, 0xF8, 0x97, 0x00, 0x00,
  0x03, 0x7B, 0x80, 0x7D, 0xE8, 0x07, 0xF7, 0xA8,
  0x07, 0xDE, 0xC1, 0x1F, 0x7C, 0x30, 0x7D, 0xF3,
  0xD5, 0xF7, 0xD1, 0x87, 0xDE, 0xE2, 0x1F, 0x7F,
  0x89, 0x70, 0x00, 0x00, 0x37, 0xB8, 0x07, 0xDE,
  0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC, 0x11, 0xF7,
  0xC2, 0x8F, 0xDF, 0x3F, 0x3F, 0x7D, 0x18, 0x7D,
  0xFE, 0x25, 0xC0, 0x00, 0x00, 0xDE, 0xE0, 0x1F,
  0x7A, 0x01, 0xFD, 0xEA, 0x01, 0xF7, 0xB0, 0x47,
  0xDF, 0x0C, 0x1F, 0x7C, 0xF4, 0x7D, 0xF4, 0x61,
  0xF7, 0xB8, 0x87, 0xDF, 0xE2, 0x5C, 0x00, 0x00,
  0x00

//0b11011110,0b11100000,0b00011111,0b01111010,0b00000001,0b11111101,0b11101010,0b00000001,
//0b11110111,0b10110000,0b01000111,0b11011111,0b00001010,0b00111111,0b01111100,0b11111100,
//0b01111101,0b11110100,0b01100001,0b11110111,0b11111000,0b10010111,0b00000000,0b00000000,
//0b00000011,0b01111011,0b10000000,0b01111101,0b11101000,0b00000111,0b11110111,0b10101000,
//0b00000111,0b11011110,0b11000001,0b00011111,0b01111100,0b00110000,0b01111101,0b11110011,
//0b11010101,0b11110111,0b11010001,0b10000111,0b11011110,0b11100010,0b00011111,0b01111111,
//0b10001001,0b01110000,0b00000000,0b00000000,0b00110111,0b10111000,0b00000111,0b11011110,
//0b10000000,0b01111111,0b01111010,0b10000000,0b01111101,0b11101100,0b00010001,0b11110111,
//0b11000010,0b10001111,0b11011111,0b00111111,0b00111111,0b01111101,0b00011000,0b01111101,
//0b11111110,0b00100101,0b11000000,0b00000000,0b00000000,0b11011110,0b11100000,0b00011111,
//0b01111010,0b00000001,0b11111101,0b11101010,0b00000001,0b11110111,0b10110000,0b01000111,
//0b11011111,0b00001100,0b00011111,0b01111100,0b11110100,0b01111101,0b11110100,0b01100001,
//0b11110111,0b10111000,0b10000111,0b11011111,0b11100010,0b01011100,0b00000000,0b00000000,
//0b00000000
};

const uint16_t num_bits_id_setup = 330;
const uint8_t id_setup_v[] =
{
  0xDE, 0xE2, 0x1F, 0x70, 0x01, 0x7D, 0xEE, 0x01,
  0xF7, 0xB0, 0x07, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
  0xFD, 0xEA, 0x01, 0xF7, 0xA0, 0x1F, 0x9F, 0x70,
  0x1F, 0x7C, 0x98, 0x7D, 0xF4, 0x81, 0xE7, 0xD0,
  0x07, 0xDE, 0x00, 0xDF, 0x7C, 0x00, 0x7D, 0xFE,
  0x25, 0xC0

//0b11011110,0b11100010,0b00011111,0b01110000,0b00000001,0b01111101,0b11101110,0b00000001,
//0b11110111,0b10110000,0b00000111,0b10011111,0b00000111,0b01011110,0b01111100,0b10000001,
//0b11111101,0b11101010,0b00000001,0b11110111,0b10100000,0b00011111,0b10011111,0b01110000,
//0b00011111,0b01111100,0b10011000,0b01111101,0b11110100,0b10000001,0b11100111,0b11010000,
//0b00000111,0b11011110,0b00000000,0b11011111,0b01111100,0b00000000,0b01111101,0b11111110,
//0b00100101,0b11000000
};

const uint16_t num_bits_erase_all = 308;
const uint8_t erase_all_v[] =
{
  0x9F, 0x82, 0xBE, 0x7F, 0x2B, 0x7D, 0xEE, 0x01,
  0xF7, 0xB0, 0x07, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
  0xFD, 0xEA, 0x01, 0xF7, 0xA0, 0x1F, 0x9F, 0x70,
  0x1F, 0x7C, 0x98, 0x7D, 0xF4, 0x81, 0xF7, 0x80,
  0x2F, 0xDF, 0x00, 0x1F, 0x7F, 0x89, 0x70

//0b10011111,0b10000010,0b10111110,0b01111111,0b00101011,0b01111101,0b11101110,0b00000001,
//0b11110111,0b10110000,0b00000111,0b10011111,0b00000111,0b01011110,0b01111100,0b10000001,
//0b11111101,0b11101010,0b00000001,0b11110111,0b10100000,0b00011111,0b10011111,0b01110000,
//0b00011111,0b01111100,0b10011000,0b01111101,0b11110100,0b10000001,0b11110111,0b10000000,
//0b00101111,0b11011111,0b00000000,0b00011111,0b01111111,0b10001001,0b01110000
};

const uint8_t read_id_v[] =
{
  0xBF, 0x00, 0xDF, 0x90, 0x00

//0b10111111,0b00000000,0b11011111,0b10010000,0b00000000
};

const uint8_t num_bits_address_length = 6;
const uint8_t num_bits_data_length = 8;

const uint8_t num_bits_write_byte_start = 5;
const uint8_t write_byte_start = 0x90;

const uint8_t num_bits_write_byte_end = 3;
const uint8_t write_byte_end = 0xE0;

const uint8_t num_bits_block_number_length = 8;
const uint8_t num_bits_set_block_number = 11;
const uint8_t set_block_number[] = {0x9F, 0x40, 0xE0};

const uint8_t num_bits_set_block_number_end = 3;
const uint8_t set_block_number_end = 0xE0;

const uint8_t num_bits_wait_and_poll_end = 40;
const uint8_t wait_and_poll_end[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00

//0b00000000,0b00000000,0b00000000,0b00000000,0b00000000
};  // forty '0's per the spec

const uint8_t read_checksum_v[] =
{
  0xBF, 0x20, 0xDF, 0x80, 0x80

//0b10111111, 0b00100000,0b11011111,0b10000000,0b10000000
};

const uint8_t num_bits_read_byte_start = 5;
const uint8_t num_bits_read_byte_end = 1;

const uint8_t read_byte_v[] =
{
  0xB0, 0x80

//0b10110000, 0b10000000
};

const uint16_t num_bits_verify_setup = 264;
const uint8_t verify_setup_v[] =
{
  0xDE, 0xE0, 0x1F, 0x7B, 0x00, 0x79, 0xF0, 0x75,
  0xE7, 0xC8, 0x1F, 0xDE, 0xA0, 0x1F, 0x7A, 0x01,
  0xF9, 0xF7, 0x01, 0xF7, 0xC9, 0x87, 0xDF, 0x48,
  0x1F, 0x78, 0x00, 0xFD, 0xF0, 0x01, 0xF7, 0xF8,
  0x97

//0b11011110,0b11100000,0b00011111,0b01111011,0b00000000,0b01111001,0b11110000,0b01110101,
//0b11100111,0b11001000,0b00011111,0b11011110,0b10100000,0b00011111,0b01111010,0b00000001,
//0b11111001,0b11110111,0b00000001,0b11110111,0b11001001,0b10000111,0b11011111,0b01001000,
//0b00011111,0b01111000,0b00000000,0b11111101,0b11110000,0b00000001,0b11110111,0b11111000,
//0b10010111
};

const uint16_t num_bits_security = 308;
const uint8_t security_v[] =
{
  0x9F, 0x8A, 0x9E, 0x7F, 0x2B, 0x7D, 0xEE, 0x01,
  0xF7, 0xB0, 0x07, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
  0xFD, 0xEA, 0x01, 0xF7, 0xA0, 0x1F, 0x9F, 0x70,
  0x1F, 0x7C, 0x98, 0x7D, 0xF4, 0x81, 0xF7, 0x80,
  0x27, 0xDF, 0x00, 0x1F, 0x7F, 0x89, 0x70

//0b10011111,0b10001010,0b10011110,0b01111111,0b00101011,0b01111101,0b11101110,0b00000001,
//0b11110111,0b10110000,0b00000111,0b10011111,0b00000111,0b01011110,0b01111100,0b10000001,
//0b11111101,0b11101010,0b00000001,0b11110111,0b10100000,0b00011111,0b10011111,0b01110000,
//0b00011111,0b01111100,0b10011000,0b01111101,0b11110100,0b10000001,0b11110111,0b10000000,
//0b00100111,0b11011111,0b00000000,0b00011111,0b01111111,0b10001001,0b01110000
};

const uint16_t num_bits_securityVerification = 308;
const uint8_t securityVerification_v[] =
{
  0xDE,0xE0,0x1F,0x7B,0x00,0x79,0xF0,0x75,0xE7,0xC8,0x1F,
  0x9F,0xA0,0x1E,0x7F,0x80,0x7D,0xEA,0x01,0xF7,0xA0,0x1F,
  0x9F,0x70,0x1F,0x7C,0x98,0x7D,0xF4,0x81,0xF7,0x80,0x87,
  0xDF,0x0,0x1F,0x7F,0x89,0x70

//0b11011110, 0b11100000, 0b00011111, 0b01111011, 0b00000000, 0b01111001, 0b11110000, 0b01110101,
//0b11100111, 0b11001000, 0b00011111, 0b10011111, 0b10100000, 0b00011110, 0b01111111, 0b10000000,
//0b01111101, 0b11101010, 0b00000001, 0b11110111, 0b10100000, 0b00011111, 0b10011111, 0b01110000,
//0b00011111, 0b01111100, 0b10011000, 0b01111101, 0b11110100, 0b10000001, 0b11110111, 0b10000000,
//0b10000111, 0b11011111, 0b00000000, 0b00011111, 0b01111111, 0b10001001, 0b01110000
};

uint8_t targetDataOUT[TARGET_DATABUFF_LEN];

/**
 *
 */
static void sdataHiZ(void)
{
  palSetPadMode(GPIOB, GPIOB_SDATA_PIN, PAL_MODE_INPUT);
}

/**
 *
 */
static void sdataPP(void)
{
  palSetPadMode(GPIOB, GPIOB_SDATA_PIN, PAL_MODE_OUTPUT_PUSHPULL);
}

/**
 *
 */
static void sdataLow(void)
{
  palClearPad(GPIOB, GPIOB_SDATA_PIN);
}

/**
 *
 */
static void sdataHigh(void)
{
  palSetPad(GPIOB, GPIOB_SDATA_PIN);
}

/**
 *
 */
static void sclkLow(void)
{
  palClearPad(GPIOB, GPIOB_SCLK_PIN);
}

/**
 *
 */
static void sclkHigh(void)
{
  palSetPad(GPIOB, GPIOB_SCLK_PIN);
}

/**
 *
 */
static void xresAssert(void)
{
  palSetPad(GPIOB, GPIOB_XRES_PIN);
}

/**
 *
 */
static void xresDeassert(void)
{
  palClearPad(GPIOB, GPIOB_XRES_PIN);
}

/**
 *
 */
static uint8_t transitionHiLoDetect(void)
{
  // Timer breaks out of the while loops if the wait in the two loops totals
  // more than 100 msec.
  volatile uint32_t timer;

  //Load the time out count for 100ms
  timer = TRANSITION_TIMEOUT;

  //Wait till SDATA is driven high by the target device
  while (1)
  {
    if (palReadPad(GPIOB, GPIOB_SDATA_PIN))
      break;

    // If the wait is too long then timeout
    if (timer-- == 0)
    {
      return(0x01);
    }
  }

  //Load the time out count for 100ms
  timer = TRANSITION_TIMEOUT;

  //Wait till SDATA is driven low by the target device
  while (1)
  {
    if (!palReadPad(GPIOB, GPIOB_SDATA_PIN))
      break;

    // If the wait is too long then timeout
    if (timer-- == 0)
    {
      return (0x01);
    }
  }

  return (0x00);
}

/**
 *
 */
static uint8_t bitReceive(void)
{
  sclkLow();
  sclkHigh();
  if (palReadPad(GPIOB, GPIOB_SDATA_PIN)) {
    return(0x01);
  }
  else
  {
    return(0x00);
  }
}

/**
 *
 */
static uint8_t byteReceive(void)
{
  uint8_t b;
  uint8_t currByte = 0x00;

  for (b = 0; b < 8; b++) {
    currByte = (currByte << 1) + bitReceive();
  }

  return(currByte);
}

/**
 *
 */
static void byteSend(uint8_t byte, uint8_t numBits)
{
  uint8_t b = 0;

  for (b = 0; b < numBits; b++)
  {

    if (byte & 0x80)
    {
      // Send a '1'
      sdataHigh();
    }
    else
    {
      // Send a '0'
      sdataLow();
    }

    //Send one clock
    sclkHigh();
    sclkLow();

    byte = byte << 1;
  }
}

/**
 *
 */
static void vectorSend(const uint8_t* vect, uint16_t numBits)
{
  sdataPP();

  while (numBits >= 8)
  {
    byteSend(*(vect), 8);
    numBits -= 8;
    vect++;
  }

  if (numBits > 0)
  {
    byteSend(*(vect), (uint8_t)numBits);
  }

  sdataHiZ();
}

/**
 *
 */
void targetReStart(void)
{
  // Assert XRES, then release, then disable XRES-Enable
  xresAssert();
  chThdSleepMilliseconds(XRES_CLK_DELAY);
  xresDeassert();
}

/**
 *
 */
uint8_t targetInitialize(void)
{
  // Configure the pins for initialization
  sdataHiZ();
  sclkLow();

  // Cycle reset and put the device in programming mode when it exits reset
  targetReStart();

  // Send Initialization Vectors and detect Hi-Lo transition on SDATA
  vectorSend(init1_v, num_bits_init1);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect())
  {
    return(0x01);
  }
  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  // Send Initialize 2 Vector
  vectorSend(init2_v, num_bits_init2);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect()) {
    return(0x01);
  }
  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  // Target Vdd = 3.3v
  vectorSend(init3_3v, num_bits_init3_3v);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  // NOTE: DO NOT not wait for HiLo on SDATA after vector Init-3
  //       it does not occur (per spec).

  return 0;
}

/**
 *
 */
uint8_t targetSiliconIDVerify(void)
{
  uint8_t targetID[2];

  // Send ID-Setup vector set
  vectorSend(id_setup_v, num_bits_id_setup);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect()) {
    return(0x01);
  }
  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  //Send Read ID vector and get Target ID
  vectorSend(read_id_v, 11);      // Read-MSB Vector is the first 11-Bits

  // Two SCLK cycles between write & read
  sclkHigh();
  sclkLow();
  sclkHigh();
  sclkLow();

  targetID[0] = byteReceive();

  sclkHigh();
  sclkLow();

  // 12 bits starting from the 3rd character
  vectorSend(read_id_v + 2, 12);

  // Two SCLK cycles between write & read
  sclkHigh();
  sclkLow();
  sclkHigh();
  sclkLow();

  // Read-LSB Command
  targetID[1] = byteReceive();

  sclkHigh();
  sclkLow();

  // 1 bit starting from the 5th character
  vectorSend(read_id_v + 4, 1);

  if ((targetID[0] != target_id_v[0]) || (targetID[1] != target_id_v[1]))
  {
    return(0x01);
  }

  return(0x00);
}

/**
 *
 */
uint8_t targetErase(void)
{
  vectorSend(erase_all_v, num_bits_erase_all);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect())
  {
    return(0x01);
  }

  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  return(0x00);
}

/**
 *
 */
uint16_t targetLoad(void)
{
  uint8_t temp;
  uint8_t targetAddress = 0x00;
  uint8_t targetDataPtr = 0x00;
  uint16_t checksum = 0x0000;

  // Set SDATA to Push-Pull.
  sdataPP();

  // Transfer the temporary RAM array into the target.
  // In this section, a 64-Byte array was specified by #define, so the entire
  // 64-Bytes are written in this loop.
  while(targetDataPtr < TARGET_DATABUFF_LEN)
  {
    temp = targetDataOUT[targetDataPtr++];
    checksum += temp;

    byteSend(write_byte_start, num_bits_write_byte_start);
    byteSend(targetAddress, num_bits_address_length);
    byteSend(temp, num_bits_data_length);
    byteSend(write_byte_end, num_bits_write_byte_end);

    // !!!NOTE:
    // byteSend() uses MSbits, so inc by '4' to put the 0..63 address into
    // the six MSBit locations.
    //
    // This can be confusing, but check the logic:
    //   The address is only 6-Bits long. The byteSend() subroutine will
    // send however-many bits, BUT...always reads them from left-to-
    // right. So in order to pass a value of 0..63 as the address using
    // byteSend(), we have to left justify the address by 2-Bits.
    //   This can be done easily by incrementing the address each time by
    // '4' rather than by '1'.
    targetAddress += 4;
  }

  return(checksum);
}

/**
 *
 */
uint8_t targetBlockProgram(uint8_t blockNumber)
{
  // Send the block-select vector.
  vectorSend(set_block_number, num_bits_set_block_number);

  // Set SDATA to Push-Pull.
  sdataPP();

  byteSend(blockNumber, num_bits_block_number_length);
  byteSend(set_block_number_end, num_bits_set_block_number_end);

  // Send the program-block vector.
  vectorSend(program_block, num_bits_program_block);

  // Generate one clock to make M8C core to start executing the instructions
  // from the test queue
  sclkHigh();
  sclkLow();

  // Wait for acknowledge from target.
  if (transitionHiLoDetect())
  {
    return(0x01);
  }

  // Send the Wait-For-Poll-End vector
  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  return(0x00);
}

/**
 *
 */
uint8_t targetBlockVerify(uint8_t blockNumber)
{
  uint8_t targetAddress = 0x00;
  uint8_t targetDataPtr = 0x00;
  uint8_t targetDataIN;

  vectorSend(set_block_number, num_bits_set_block_number);

  // Set the drive here because byteSend() does not
  sdataPP();

  byteSend(blockNumber, num_bits_block_number_length);
  byteSend(set_block_number_end, num_bits_set_block_number_end);

  vectorSend(verify_setup_v, num_bits_verify_setup);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect())
  {
    return(0x01);
  }

  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  while (targetDataPtr < TARGET_DATABUFF_LEN)
  {
    //Send Read Byte vector and then get a byte from Target
    vectorSend(read_byte_v, num_bits_read_byte_start);

    // Set the drive here because byteSend() does not
    sdataPP();

    byteSend(targetAddress, num_bits_address_length);

    // Run two SCLK cycles between writing and reading
    sclkHigh();
    sclkLow();
    sclkHigh();
    sclkLow();

    // Set to HiZ so Target can drive SDATA
    sdataHiZ();

    targetDataIN = byteReceive();

    sclkHigh();
    sclkLow();

    // Send the ReadByte Vector End
    vectorSend(read_byte_v + 1, num_bits_read_byte_end);

    // Test the Byte that was read from the Target against the original
    // value (already in the 64-Byte array "targetDataOUT[]"). If it
    // matches, then bump the address & pointer, loop-back and continue.
    // If it does NOT match abort the loop and return and error.
    if (targetDataIN != targetDataOUT[targetDataPtr++])
    {
      return(0x01);
    }

    // Increment the address by four to accomodate 6-Bit addressing
    // (puts the 6-bit address into MSBit locations for "byteSend()").
    targetAddress += 4;
  }

  return(0x00);
}

/**
 *
 */
uint8_t targetBankChecksumGet(uint16_t* pAcc)
{
  vectorSend(checksum_v, num_bits_checksum);

  // Generate one clock to make M8C core to start executing the instructions
  // from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect())
  {
    return(0x01);
  }

  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  // Send Read Checksum vector and get Target Checksum
  vectorSend(read_checksum_v, 11);     // first 11-bits is ReadCKSum-MSB

  // Two SCLKs between write & read
  sclkHigh();
  sclkLow();
  sclkHigh();
  sclkLow();

  // Read-MSB Command
  *pAcc += (uint16_t)(byteReceive() << 8);

  sclkHigh();
  sclkLow();

  // 12 bits starting from 3rd character
  vectorSend(read_checksum_v + 2, 12);

  // Two SCLKs between write & read
  sclkHigh();
  sclkLow();
  sclkHigh();
  sclkLow();

  // Read-LSB Command
  *pAcc += byteReceive();

  sclkHigh();
  sclkLow();

  // Send the final bit of the command
  vectorSend(read_checksum_v + 3, 1);

  return(0x00);
}

/**
 *
 */
uint8_t targetSecurityFlash(void)
{
  uint8_t temp;
  uint8_t targetAddress = 0x00;
  uint8_t targetDataPtr = 0x00;

  sdataPP();

  // Transfer the temporary RAM array into the target
  while(targetDataPtr < SECURITY_BYTES_PER_BANK)
  {
    temp = targetDataOUT[targetDataPtr++];
    byteSend(write_byte_start, num_bits_write_byte_start);
    byteSend(targetAddress, num_bits_address_length);
    byteSend(temp, num_bits_data_length);
    byteSend(write_byte_end, num_bits_write_byte_end);

    // SendBytes() uses MSBits, so increment the address by '4' to put
    // the 0..n address into the six MSBit locations
    targetAddress += 4;
  }

  vectorSend(security_v, num_bits_security);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect())
  {
    return(0x01);
  }

  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  return(0x00);
}

/**
 *
 */
uint8_t targetSecurityVerify(void)
{
  uint8_t targetAddress = 0x00;
  uint8_t targetDataPtr = 0x00;
  uint8_t targetDataIN;

  //Send the vector to read the security bits
  vectorSend(securityVerification_v, num_bits_securityVerification);

  //Generate one clock to make M8C core to start executing the instructions
  //from the test queue
  sclkHigh();
  sclkLow();

  if (transitionHiLoDetect())
  {
    return(0x01);
  }

  vectorSend(wait_and_poll_end, num_bits_wait_and_poll_end);

  /* Read the security bytes */
  while(targetDataPtr < SECURITY_BYTES_PER_BANK)
  {
    //Send Read Byte vector and then get a byte from Target
    vectorSend(read_byte_v, num_bits_read_byte_start);

    // Set the drive here because byteSend() does not
    sdataPP();

    byteSend(targetAddress, num_bits_address_length);

    // Run two SCLK cycles between writing and reading
    sclkHigh();
    sclkLow();
    sclkHigh();
    sclkLow();

    // Set to HiZ so Target can drive SDATA
    sdataHiZ();

    targetDataIN = byteReceive();

    sclkHigh();
    sclkLow();

    // Send the ReadByte Vector End
    vectorSend(read_byte_v + 1, num_bits_read_byte_end);

    // *********************************************************************
    // ****                      USER ATTENTION REQUIRED                ****
    // *********************************************************************
    // Add your code here to verify the security value. Currently,
    // known value of 0x00 (U - Unprotected) is written in the function
    // fSecureTargetFlash()

    if (targetDataIN != targetDataOUT[targetDataPtr++])
    {
      return(0x01);
    }

    // SendBytes() uses MSBits, so increment the address by '4' to put
    // the 0..n address into the six MSBit locations
    targetAddress += 4;
  }

  return(0x00);
}
