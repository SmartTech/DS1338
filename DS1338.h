#ifndef _DS1338_H_
#define _DS1338_H_

#include <Arduino.h>

#include <Wire.h>

#ifdef _WIREBASE_H_
#include <SoftWire.h>
#endif

// Device address
#define DS1338_DEFAULT_ADDR 0x68

//	These options control the behavior of the SQW/(!INTB) pin
#define DS1338_SQW_32768HZ (0x03)
#define DS1338_SQW_8192HZ  (0x02)
#define DS1338_SQW_4096HZ  (0x01)
#define DS1338_SQW_1HZ     (0x00)

#define DS1338_REG_RAM_BEGIN 0x08
#define DS1338_REG_RAM_END   0x3F
#define DS1338_REG_RAM_SIZE  (DS1338_REG_RAM_END - DS1338_REG_RAM_BEGIN)

/* return codes from endTransmission() */
//#define SUCCESS   0        /* transmission was successful */
//#define EDATA     1        /* too much data */
//#define ENACKADDR 2        /* received nack on transmit of address */
//#define ENACKTRNS 3        /* received nack on transmit of data */
//#define EOTHER    4        /* other error */

/** Represents a time and date. */
struct rtctime_t {
  uint8_t  second;
  uint8_t  minute;
  uint8_t  hour;
  uint8_t  day;
  uint8_t  month;
  uint8_t  year;
};

class DS1338 {
	
	private: //---------------------------------------------------------------
	
	#define USE_WIRE_DELAY
	
	// Registers
	#define DS1338_REG_SECONDS   0x00
	#define DS1338_REG_MINUTES   0x01
	#define DS1338_REG_HOURS     0x02
	#define DS1338_REG_DAY       0x03
	#define DS1338_REG_DATE      0x04
	#define DS1338_REG_MONTH     0x05
	#define DS1338_REG_YEAR      0x06
	#define DS1338_REG_CONTROL   0x07
	
	/**	- If set, in an hour register (DS1338_REG_HOURS, DS1338_REG_A1_HOUR,
	        DS1338_REG_A2_HOUR, the hour is between 0 and 12, and the
	        (!AM)/PM bit indicates AM or PM.
	    - If not set, the hour is between 0 and 23.
	*/
	#define DS1338_HOUR_12 (0x01 << 6)

	/**	If DS1338_HOUR_12 is set:
	      - If set, indicates PM
	      - If not set, indicates AM
	*/
	#define DS1338_PM_MASK (0x01 << 5)

	// If set, the oscillator has stopped since the last time this bit was cleared.
	#define DS1338_OSC_STOP_FLAG (0x01 << 5)

	// Set to disable the oscillator
	#define DS1338_OSC_DISABLE (0x01 << 7)
	
	/**	These options control the behavior of the SQW/(!INTB) pin. */
	#define DS1338_SQWE_FLAG (0x01 << 4)
	#define DS1338_SQW_MASK  (0x03)

	// Occurs when the number of I2C bytes available is less than the number requested.
	#define READ_ERROR 5

	#define decode_bcd(x) ((x >> 4) * 10 + (x & 0x0F))
	#define encode_bcd(x) ((((x / 10) & 0x0F) << 4) + (x % 10))
		
	uint8_t rtc_i2c_addr = DS1338_DEFAULT_ADDR;
	
	uint8_t i2c_read(uint8_t addr, uint8_t *buf, uint8_t num);
	uint8_t i2c_write(uint8_t addr, uint8_t b);
	uint8_t i2c_write(uint8_t addr, uint8_t *buf, uint8_t num);
	uint8_t i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num);
	
	uint8_t getControl(uint8_t *ctrl);
	uint8_t setControl(uint8_t ctrl);
	uint8_t setControlBits(uint8_t mask);
	uint8_t clearControlBits(uint8_t mask);
	
	
	#ifdef _WIREBASE_H_
	WireBase* wire;
	#else
	TwoWire* wire;
	#endif
	
	public: //---------------------------------------------------------------
		
	DS1338();
	#ifdef _WIREBASE_H_
    DS1338(WireBase* _wire);
    #endif
	
	uint8_t begin(uint8_t addr = DS1338_DEFAULT_ADDR);
	
	void makeTime(struct rtctime_t *time, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
	void formatTime(struct rtctime_t *time, char *buf);

	uint8_t getTime(struct rtctime_t *time);
	uint8_t setTime(struct rtctime_t *time);

	uint8_t readRAM(uint8_t address, uint8_t *data, uint8_t length);
	uint8_t writeRAM(uint8_t address, uint8_t *data, uint8_t length);

	uint8_t enableSQW(uint8_t frequence);
	uint8_t disableSQW();

	uint8_t getOSF(uint8_t *osf);
	uint8_t cleanOSF();
	
	
};

#endif // _DS1338_H_
