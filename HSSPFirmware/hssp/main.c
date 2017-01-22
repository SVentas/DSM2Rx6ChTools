/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "issp.h"

uint8_t fRunMain = 0x01;

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palClearPad(GPIOA, GPIOA_LED_RED);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOA, GPIOA_LED_RED);
    chThdSleepMilliseconds(500);
  }
}

/*
 * Application entry point.
 */
int main(void) {
  uint8_t cmdID;
  uint8_t cmdTrm;
  uint8_t blockNum;
  uint16_t checksum;
  uint16_t targetChecksum;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD1, NULL);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (fRunMain) {
    chnRead(&SD1, &cmdID, 1);
    chnRead(&SD1, &cmdTrm, 1);

    if ((cmdID >= 'a') && (cmdID <= 'z') && (cmdTrm == '\n'))
    {
      switch (cmdID)
      {
        /* Initialize target and verify silicon ID */
        case 'a':
          if (targetInitialize())
          {
            chnWrite(&SD1, (const uint8_t *)"a\n1\n", 4);
          }
          else if (targetSiliconIDVerify())
          {
            chnWrite(&SD1, (const uint8_t *)"a\n2\n", 4);
          }
          else
          {
            chnWrite(&SD1, (const uint8_t *)"a\n0\n", 4);
          }
          break;

        /* Erase target */
        case 'b':
          if (targetErase())
          {
            chnWrite(&SD1, (const uint8_t *)"b\n1\n", 4);
          }
          else
          {
            chnWrite(&SD1, (const uint8_t *)"b\n0\n", 4);
          }
          break;

        /* Program flash memory of the target */
        case 'c':
          checksum = 0x0000;

          for (blockNum = 0; blockNum < BLOCKS_PER_BANK; blockNum++)
          {
            chnRead(&SD1, targetDataOUT, TARGET_DATABUFF_LEN);

            checksum += targetLoad();

            if (targetBlockProgram(blockNum))
            {
              chnWrite(&SD1, (const uint8_t *)"c\n1\n", 4);
              break;
            }
            else if (blockNum < (BLOCKS_PER_BANK - 1))
            {
              chnWrite(&SD1, (const uint8_t *)"c\n+\n", 4);
            }
            else
            {
              chnWrite(&SD1, (const uint8_t *)"c\n0\n", 4);
            }
          }
          break;

        /* Verify flash memory of the target */
        case 'd':
          for (blockNum = 0; blockNum < BLOCKS_PER_BANK; blockNum++)
          {
            chnRead(&SD1, targetDataOUT, TARGET_DATABUFF_LEN);

            if (targetBlockVerify(blockNum))
            {
              chnWrite(&SD1, (const uint8_t *)"d\n1\n", 4);
              break;
            }
            else if (blockNum < (BLOCKS_PER_BANK - 1))
            {
              chnWrite(&SD1, (const uint8_t *)"d\n+\n", 4);
            }
            else
            {
              chnWrite(&SD1, (const uint8_t *)"d\n0\n", 4);
            }
          }
          break;

        /* Program security bits */
        case 'e':
          chnRead(&SD1, targetDataOUT, TARGET_DATABUFF_LEN);

          if (targetSecurityFlash())
          {
            chnWrite(&SD1, (const uint8_t *)"e\n1\n", 4);
          }
          else
          {
            chnWrite(&SD1, (const uint8_t *)"e\n0\n", 4);
          }
          break;

        /* Verify security bits */
        case 'f':
          chnRead(&SD1, targetDataOUT, TARGET_DATABUFF_LEN);

          if (targetSecurityVerify())
          {
            chnWrite(&SD1, (const uint8_t *)"f\n1\n", 4);
          }
          else
          {
            chnWrite(&SD1, (const uint8_t *)"f\n0\n", 4);
          }
          break;

        /* Verify checksum */
        case 'g':
          targetChecksum = 0x0000;

          if (targetBankChecksumGet(&targetChecksum))
          {
            chnWrite(&SD1, (const uint8_t *)"g\n1\n", 4);
          }
          else if (targetChecksum != checksum)
          {
            chnWrite(&SD1, (const uint8_t *)"g\n2\n", 4);
          }
          else
          {
            chnWrite(&SD1, (const uint8_t *)"g\n0\n", 4);
          }
          break;

        /* ReStart target */
        case 'x':
          targetReStart();
          chnWrite(&SD1, (const uint8_t *)"x\n0\n", 4);
          break;

        /* ReStart hssp programmer */
        case 'z':
          fRunMain = 0x00;
          break;
        default:;
      }
    }
  }

  /* Stop the SerialPort 1 */
  sdStop(&SD1);

  chSysDisable();

  /* Reset all peripherals. */
  rccResetAPB1(0xFFFFFFFF);
  rccResetAPB2(0xFFFFFFFF);

  NVIC_SystemReset();

  /* This point should never be reached. */
  return 0;
}
