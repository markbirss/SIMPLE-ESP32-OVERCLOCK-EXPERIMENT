//********************************************************************************
//*                                                                              *
//*   SIMPLE ESP32 OVERCLOCK EXPERIMENT                                          *
//*                                                                              *
//********************************************************************************
//*                                                                              *
//*   Do everything at your own risk!!!                                          *
//*                                                                              *
//*   TESTED on WROVER B with 40MHz crystal oscillator                           *
//*                                                                              *
//*   increasing speed of ESP32 cause timing changes of all system...            *
//*                                                                              *
//********************************************************************************

//********************************************************************************
// >>>> HERE:

uint8_t OVERCLOCK=1;

//0 = 1.000x of speed = no overclock 
//1 = 1.414x of speed 
//2 = 1.540x of speed
//3 = 1.666x of speed

//********************************************************************************

#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"
#include "soc/soc.h"
#include "soc/rtc.h"
#include "soc/dport_reg.h"
#include "rom/ets_sys.h"
#include "soc/rtc.h"
#include "soc/rtc_periph.h"
#include "soc/apb_ctrl_reg.h"
#include "sdkconfig.h"


extern "C" {
  // ROM functions which read/write internal control bus 
  uint8_t rom_i2c_readReg(uint8_t block, uint8_t host_id, uint8_t reg_add);
  uint8_t rom_i2c_readReg_Mask(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t msb, uint8_t lsb);
  void rom_i2c_writeReg(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t data);
  void rom_i2c_writeReg_Mask(uint8_t block, uint8_t host_id, uint8_t reg_add, uint8_t msb, uint8_t lsb, uint8_t data);
}

//********************************************************************************

#define I2C_BBPLL                   0x66
#define I2C_BBPLL_ENDIV5              11
#define I2C_BBPLL_BBADC_DSMP           9
#define I2C_BBPLL_HOSTID               4
#define I2C_BBPLL_OC_LREF              2
#define I2C_BBPLL_OC_DIV_7_0           3
#define I2C_BBPLL_OC_DCUR              5

#define BBPLL_ENDIV5_VAL_320M       0x43
#define BBPLL_BBADC_DSMP_VAL_320M   0x84
#define BBPLL_ENDIV5_VAL_480M       0xc3
#define BBPLL_BBADC_DSMP_VAL_480M   0x74


#define I2C_WRITEREG_MASK_RTC(block, reg_add, indata) \
  rom_i2c_writeReg_Mask(block, block##_HOSTID,  reg_add,  reg_add##_MSB,  reg_add##_LSB,  indata)

#define I2C_READREG_MASK_RTC(block, reg_add) \
  rom_i2c_readReg_Mask(block, block##_HOSTID,  reg_add,  reg_add##_MSB,  reg_add##_LSB)

#define I2C_WRITEREG_RTC(block, reg_add, indata) \
  rom_i2c_writeReg(block, block##_HOSTID,  reg_add, indata)

#define I2C_READREG_RTC(block, reg_add) \
  rom_i2c_readReg(block, block##_HOSTID,  reg_add)




//********************************************************************************

void OVERCLOCK_ME(uint8_t OC_LEVEL) {
  I2C_WRITEREG_RTC(I2C_BBPLL, I2C_BBPLL_ENDIV5, BBPLL_ENDIV5_VAL_480M);
  I2C_WRITEREG_RTC(I2C_BBPLL, I2C_BBPLL_BBADC_DSMP, BBPLL_BBADC_DSMP_VAL_480M);

  uint8_t div_ref;
  uint8_t div7_0;
  uint8_t div10_8;
  uint8_t lref;
  uint8_t dcur;
  uint8_t bw;

  div_ref = 0;

  if (OC_LEVEL==1) {
     div7_0 = 56;  // ~1.41421356x speed (~340MHz?) 
  } else if (OC_LEVEL==2) {
     div7_0 = 56;  // ~1.54044011x speed (~370MHz?)
  } else if (OC_LEVEL==3) {
    div7_0 = 64;   // ~1.66666667x speed (~400MHz?)
  } else {
    div7_0 = 32;   // 1x speed (240MHz)
  }
  div10_8 = 0;
  lref = 0;
  dcur = 6;
  bw = 3;

  uint8_t i2c_bbpll_lref  = (lref << 7) | (div10_8 << 4) | (div_ref);
  uint8_t i2c_bbpll_div_7_0 = div7_0;
  uint8_t i2c_bbpll_dcur = (bw << 6) | dcur;
  I2C_WRITEREG_RTC(I2C_BBPLL, I2C_BBPLL_OC_LREF, i2c_bbpll_lref);
  I2C_WRITEREG_RTC(I2C_BBPLL, I2C_BBPLL_OC_DIV_7_0, i2c_bbpll_div_7_0);
  I2C_WRITEREG_RTC(I2C_BBPLL, I2C_BBPLL_OC_DCUR, i2c_bbpll_dcur);
}
//********************************************************************************

float TIMING;

void setup() {

  if (OVERCLOCK==1) TIMING=1.41421356;        //=SQRT(2);
  else if (OVERCLOCK==2) TIMING=1.54044011;
  else if (OVERCLOCK==3) TIMING=1.66666667;   //=5/3
  else TIMING=1;

  Serial.begin(115200 / TIMING);

  printf("NOT OVERCLOCKED = SERIAL BAD TIMING \n");

  delay(1000);

  OVERCLOCK_ME(OVERCLOCK);

  delay(1000);

  printf("OVERCLOCK = SERIAL CORRECTED TIMING \n");
}


void loop() {

}
