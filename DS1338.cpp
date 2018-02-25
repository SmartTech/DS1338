#include "DS1338.h"

extern "C" {
#include <inttypes.h>
}

/* Public methods ================================================= */

DS1338::DS1338() :
	wire(&Wire)
{
	
}

#ifdef _WIREBASE_H_
DS1338::DS1338(WireBase* _wire) :
	wire(_wire)
{	
}
#endif

uint8_t DS1338::begin(uint8_t addr) {
	uint8_t res = 5;
	wire->begin();
	wire->beginTransmission(rtc_i2c_addr = addr);
	res = wire->endTransmission();
	#ifdef USE_WIRE_DELAY
    delay(1);
    #endif
	return !res;
}

uint8_t DS1338::cleanOSF() {
	if(clearControlBits(DS1338_OSC_STOP_FLAG)) return false;
	return true;
}

uint8_t DS1338::enableSQW(uint8_t frequence) {
	uint8_t ctrl=0;
	frequence = frequence & DS1338_SQW_MASK;
	ctrl |= frequence;
	ctrl |= DS1338_SQWE_FLAG;
	clearControlBits(DS1338_SQW_MASK);
	if(setControlBits(ctrl)) return false;
	return true;
}

uint8_t DS1338::disableSQW() {
	if(clearControlBits(DS1338_SQWE_FLAG|DS1338_SQW_MASK)) return false;
	return true;
}

/** 
 * \brief Read the current time. 
 * \param time A pointer to a struct rtctime_t instance in which to store the time. 
 */
uint8_t DS1338::getTime(struct rtctime_t *time) {
	uint8_t buf[7];
	uint8_t res = i2c_write(rtc_i2c_addr, DS1338_REG_SECONDS);
	if(res) return false;
	res = i2c_read(rtc_i2c_addr, buf, 7);
	if(res) return false;
	time->second = decode_bcd(buf[0]);
	time->minute = decode_bcd(buf[1]);
	if (buf[2] & DS1338_HOUR_12) {
		time->hour = ((buf[2] >> 4) & 0x01) * 12 + ((buf[2] >> 5) & 0x01) * 12;
	}
	else {
		time->hour = decode_bcd(buf[2]);
	}
	time->day = decode_bcd(buf[4]);
	time->month = decode_bcd(buf[5] & 0x1F);
	time->year = 100 * ((buf[5] >> 7) & 0x01) + decode_bcd(buf[6]);
	return true;
}

/** 
 * \brief Set the time. 
 * \param time A pointer to a struct rtctime_t instance containing the time to set. 
 */
uint8_t DS1338::setTime(struct rtctime_t *time) {
	uint8_t buf[8];
	buf[0] = DS1338_REG_SECONDS;
	buf[1] = encode_bcd(time->second);
	buf[2] = encode_bcd(time->minute);
	buf[3] = encode_bcd(time->hour); // Time always stored in 24-hour format
	buf[4] = 1;						 // Not used
	buf[5] = encode_bcd(time->day);
	buf[6] = ((time->year / 100) << 7) | encode_bcd(time->month);
	buf[7] = encode_bcd((time->year) % 100);
	uint8_t res = i2c_write(rtc_i2c_addr, buf, 8);
	return !res;
}

uint8_t DS1338::readRAM(uint8_t address, uint8_t *data, uint8_t length) {
	if (address > DS1338_REG_RAM_END || address < DS1338_REG_RAM_BEGIN) {
		return false;
	}
	if (address + length > DS1338_REG_RAM_END) {
		return false;
	}
	uint8_t res = 5;
	res	= i2c_write(rtc_i2c_addr, address);
	if (res) return false;
	res = i2c_read(rtc_i2c_addr, data, length);
	return !res;
}

uint8_t DS1338::writeRAM(uint8_t address, uint8_t *data, uint8_t length) {
	if (address > DS1338_REG_RAM_END || address < DS1338_REG_RAM_BEGIN)	{
		return false;
	}
	if (address + length > DS1338_REG_RAM_END) {
		return false;
	}
	uint8_t res = i2c_write_reg(rtc_i2c_addr, address, data, length);
	return !res;
}

/** 
 * \brief Set the values in a struct rtctime_t instance. 
 * \param time A pointer to a struct rtctime_t instance. 
 * \param year The year.
 * \param month The month. 
 * \param day The day. 
 * \param hour The hour. 
 * \param minute The minute. 
 * \param second The second. 
 */
void DS1338::makeTime(struct rtctime_t *time, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
	time->year = year;
	time->month = month;
	time->day = day;
	time->hour = hour;
	time->minute = minute;
	time->second = second;
}

/**
 * \brief Output a string representation of a struct rtctime_t object.
 * Requires a buffer size of at least 20 characters. 
 * \param time A pointer to a struct rtctime_t instance. 
 * \param buf A pointer to a character buffer in which to store the string.
 */
void DS1338::formatTime(struct rtctime_t *time, char *buf) {
	// Year
	buf[0] = '2';
	if (time->year < 10) {
		buf[1] = '0';
		buf[2] = '0';
		buf[3] = '0' + time->year;
	}
	else if (time->year < 100) {
		buf[1] = '0';
		buf[2] = '0' + (time->year / 10);
		buf[3] = '0' + (time->year % 10);
	}
	else {
		buf[1] = '1';
		buf[2] = '0' + ((time->year - 100) / 10);
		buf[3] = '0' + ((time->year - 100) % 10);
	}
	buf[4] = '-';

	// Month
	if (time->month < 10) {
		buf[5] = '0';
		buf[6] = '0' + time->month;
	}
	else {
		buf[5] = '0' + (time->month / 10);
		buf[6] = '0' + (time->month % 10);
	}
	buf[7] = '-';

	// Day
	if (time->day < 10) {
		buf[8] = '0';
		buf[9] = '0' + time->day;
	}
	else {
		buf[8] = '0' + (time->day / 10);
		buf[9] = '0' + (time->day % 10);
	}
	buf[10] = 'T';

	// Hour
	if (time->hour < 10) {
		buf[11] = '0';
		buf[12] = '0' + time->hour;
	}
	else {
		buf[11] = '0' + (time->hour / 10);
		buf[12] = '0' + (time->hour % 10);
	}
	buf[13] = ':';

	// Minute
	if (time->minute < 10) {
		buf[14] = '0';
		buf[15] = '0' + time->minute;
	}
	else {
		buf[14] = '0' + (time->minute / 10);
		buf[15] = '0' + (time->minute % 10);
	}
	buf[16] = ':';

	// Second
	if (time->second < 10) {
		buf[17] = '0';
		buf[18] = '0' + time->second;
	}
	else {
		buf[17] = '0' + (time->second / 10);
		buf[18] = '0' + (time->second % 10);
	}

	buf[19] = 0;
}


/* Private methods ================================================= */

/**
 * \brief Read data from an I2C device. 
 * \param addr The address of the device from which to read. 
 * \param buf A pointer to a buffer in which to store the data. 
 * \param num The number of bytes to read. 
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::i2c_read(uint8_t addr, uint8_t *buf, uint8_t num) {
	wire->requestFrom(addr, num);
	if (wire->available() < num) return READ_ERROR;
	for (uint8_t i = 0; i < num; i++) buf[i] = wire->read();
	return 0;
}

/**
 * \brief Write a single byte to an I2C device. 
 * \param addr The address of the device to which to write. 
 * \param b The byte to write. 
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::i2c_write(uint8_t addr, uint8_t b) {
	uint8_t res = 5;
	wire->beginTransmission(addr);
	wire->write(b);
	res = wire->endTransmission();
	#ifdef USE_WIRE_DELAY
    delay(1);
    #endif
	return res;
}

/**
 * \brief Write data to an I2C device. 
 * \param addr The address of the device to which to write. 
 * \param buf A pointer to a buffer from which to read the data. 
 * \param num The number of bytes to write. 
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::i2c_write(uint8_t addr, uint8_t *buf, uint8_t num) {
	uint8_t res = 5;
	wire->beginTransmission(addr);
	for (uint8_t i = 0; i < num; i++) wire->write(buf[i]);
	res = wire->endTransmission();
	#ifdef USE_WIRE_DELAY
    delay(1);
    #endif
	return res;
}

uint8_t DS1338::i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num) {
	uint8_t res = 5;
	wire->beginTransmission(addr);
	wire->write(reg);
	for (uint8_t i = 0; i < num; i++) wire->write(buf[i]);
	res = wire->endTransmission();
	#ifdef USE_WIRE_DELAY
    delay(1);
    #endif
	return res;
}

/**
 * \defgroup ds1338_control Control Register Methods
 */

/**
 * \brief Get the value of the control register.
 * \param ctrl A pointer to a value in which to store the value of the control register. 
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::getControl(uint8_t *ctrl) {
	uint8_t res = i2c_write(rtc_i2c_addr, DS1338_REG_CONTROL);
	if (res) return res;
	res = i2c_read(rtc_i2c_addr, ctrl, 1);
	if (res) return res;
	return 0;
}

/**
 * \brief Set the value of the control register.
 * \param ctrl The value to set.
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::setControl(uint8_t ctrl) {
	uint8_t buf[2];
	buf[0] = DS1338_REG_CONTROL;
	buf[1] = ctrl;
	return i2c_write(rtc_i2c_addr, buf, 2);
}

/**
 * \brief Set the specified bits in the control register.
 * \param mask A mask specifying which bits to set. (High bits will be set.)
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::setControlBits(uint8_t mask) { // set bits
	uint8_t ctrl;
	uint8_t res = getControl(&ctrl);
	if (res) return res;
	ctrl |= mask;
	return setControl(ctrl);
}

/**
 * \brief Clear the specified bits in the control register.
 * \param mask A mask specifying which bits to clear. (High bits will be cleared.) 
 * \return 0 on success; otherwise an I2C error.
 */
uint8_t DS1338::clearControlBits(uint8_t mask) {
	return setControl(~mask);
}

uint8_t DS1338::getOSF(uint8_t *osf) {
	uint8_t ctrl;
	uint8_t res = getControl(&ctrl);
	if (res) return res;
	if (ctrl & DS1338_OSC_STOP_FLAG == 0) {
		*osf = 0;
	}
	else {
		*osf = 1;
	}
	return 0;
}
