#ifndef ISSP_H
#define ISSP_H

// Block-Verify Uses 64-Bytes of RAM
#define TARGET_DATABUFF_LEN     (64)

// CY8C21x34
#define NUM_BANKS               (1)
#define BLOCKS_PER_BANK         (128)
#define SECURITY_BYTES_PER_BANK (32)

#ifdef __cplusplus
extern "C" {
#endif
  extern uint8_t targetDataOUT[TARGET_DATABUFF_LEN];

  void targetReStart(void);
  uint8_t targetInitialize(void);
  uint8_t targetSiliconIDVerify(void);
  uint8_t targetErase(void);
  uint16_t targetLoad(void);
  uint8_t targetBlockProgram(uint8_t blockNumber);
  uint8_t targetBlockVerify(uint8_t blockNumber);
  uint8_t targetBankChecksumGet(uint16_t* pAcc);
  uint8_t targetSecurityFlash(void);
  uint8_t targetSecurityVerify(void);
#ifdef __cplusplus
}
#endif

#endif /* ISSP_H */
