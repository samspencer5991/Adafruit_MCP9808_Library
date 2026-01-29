/*!
 *  @file Adafruit_MCP9808.h
 *
 * 	I2C Driver for Microchip's MCP9808 I2C Temp sensor
 *
 * 	This is a library for the Adafruit MCP9808 breakout:
 * 	http://www.adafruit.com/products/1782
 *
 * 	Adafruit invests time and resources providing this open source code,
 *please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */

#ifndef _ADAFRUIT_MCP9808_H
#define _ADAFRUIT_MCP9808_H

#include "Adafruit_BusIO_Register.h"
#include "Arduino.h"
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>

#define MCP9808_I2CADDR_DEFAULT 0x18 ///< I2C address
#define MCP9808_REG_CONFIG 0x01		 ///< MCP9808 config register

#define MCP9808_REG_CONFIG_SHUTDOWN 0x0100	 ///< shutdown config
#define MCP9808_REG_CONFIG_CRITLOCKED 0x0080 ///< critical trip lock
#define MCP9808_REG_CONFIG_WINLOCKED 0x0040	 ///< alarm window lock
#define MCP9808_REG_CONFIG_INTCLR 0x0020	 ///< interrupt clear
#define MCP9808_REG_CONFIG_ALERTSTAT 0x0010	 ///< alert output status
#define MCP9808_REG_CONFIG_ALERTCTRL 0x0008	 ///< alert output control
#define MCP9808_REG_CONFIG_ALERTSEL 0x0004	 ///< alert output select
#define MCP9808_REG_CONFIG_ALERTPOL 0x0002	 ///< alert output polarity
#define MCP9808_REG_CONFIG_ALERTMODE 0x0001	 ///< alert output mode

#define MCP9808_REG_UPPER_TEMP 0x02	  ///< upper alert boundary
#define MCP9808_REG_LOWER_TEMP 0x03	  ///< lower alert boundery
#define MCP9808_REG_CRIT_TEMP 0x04	  ///< critical temperature
#define MCP9808_REG_AMBIENT_TEMP 0x05 ///< ambient temperature
#define MCP9808_REG_MANUF_ID 0x06	  ///< manufacture ID
#define MCP9808_REG_DEVICE_ID 0x07	  ///< device ID
#define MCP9808_REG_RESOLUTION 0x08	  ///< resolutin

#define MCP9808_CONFIG_HYST_0C 0x0000	// Bits 10-9 = 00
#define MCP9808_CONFIG_HYST_1_5C 0x0200 // Bits 10-9 = 01
#define MCP9808_CONFIG_HYST_3C 0x0400	// Bits 10-9 = 10
#define MCP9808_CONFIG_HYST_6C 0x0600	// Bits 10-9 = 11
#define MCP9808_CONFIG_HYST_MASK 0x0600 // Mask for bits 10-9

#define MCP9808_HYST_0C 0
#define MCP9808_HYST_1_5C 1
#define MCP9808_HYST_3C 2
#define MCP9808_HYST_6C 3

/*!
 *    @brief  Class that stores state and functions for interacting with
 *            MCP9808 Temp Sensor
 */
class Adafruit_MCP9808 : public Adafruit_Sensor
{
public:
	Adafruit_MCP9808();
	bool begin();
	bool begin(TwoWire *theWire);
	bool begin(uint8_t addr);
	bool begin(uint8_t addr, TwoWire *theWire);

	bool init();
	float readTempC();
	float readTempF();
	uint8_t getResolution(void);
	void setResolution(uint8_t value);

	void shutdown_wake(boolean sw);
	void shutdown();
	void wake();

	// Temperature alert configuration functions
	bool setUpperTemp(float temp);
	bool setLowerTemp(float temp);
	bool setCriticalTemp(float temp);
	bool setAlertPolarity(bool activeHigh);
	bool setAlertMode(bool interruptMode);
	bool enableAlert(bool enable);
	bool setAlertSelectCriticalOnly(bool critOnly);

	bool setHysteresis(uint8_t hyst);
	uint8_t getHysteresis();

	// Convenience function - sets critical temp and polarity, enables alert
	bool setTempAlert(float temp, bool activeHigh, bool alertMode, uint8_t hysteresis, bool critOnly);

	// Read alert settings
	float getUpperTemp();
	float getLowerTemp();
	float getCriticalTemp();
	bool getAlertStatus();

	void write16(uint8_t reg, uint16_t val);
	uint16_t read16(uint8_t reg);

	void write8(uint8_t reg, uint8_t val);
	uint8_t read8(uint8_t reg);

	/* Unified Sensor API Functions */
	bool getEvent(sensors_event_t *);
	void getSensor(sensor_t *);

private:
	uint16_t _sensorID = 9808; ///< ID number for temperature
	Adafruit_I2CDevice *i2c_dev = NULL;
	/* Helper Functions */
	uint16_t tempToReg(float temp);
	float regToTemp(uint16_t reg);
};

#endif
