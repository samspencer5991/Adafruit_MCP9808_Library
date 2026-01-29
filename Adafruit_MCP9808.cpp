/*!
 *  @file Adafruit_MCP9808.cpp
 *
 *  @mainpage Adafruit MCP9808 I2C Temp Sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for Microchip's MCP9808 I2C Temp sensor
 *
 * 	This is a library for the Adafruit MCP9808 breakout:
 * 	http://www.adafruit.com/products/1782
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section author Author
 *
 *  K.Townsend (Adafruit Industries)
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Adafruit_MCP9808.h"

/*!
 *    @brief  Instantiates a new MCP9808 class
 */
Adafruit_MCP9808::Adafruit_MCP9808() {}

/*!
 *    @brief  Setups the HW
 *    @param  *theWire
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin(TwoWire *theWire)
{
	return begin(MCP9808_I2CADDR_DEFAULT, theWire);
}

/*!
 *    @brief  Setups the HW
 *    @param  addr
 *    @return True if initialization was successful, otherwise false.
 */

bool Adafruit_MCP9808::begin(uint8_t addr) { return begin(addr, &Wire); }

/*!
 *    @brief  Setups the HW
 *    @param  addr
 *    @param  *theWire
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin(uint8_t addr, TwoWire *theWire)
{
	if (i2c_dev)
	{
		delete i2c_dev;
	}
	i2c_dev = new Adafruit_I2CDevice(addr, theWire);

	return init();
}

/*!
 *    @brief  Setups the HW with default address
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::begin() { return begin(MCP9808_I2CADDR_DEFAULT, &Wire); }

/*!
 *    @brief  init function
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_MCP9808::init()
{
	if (!i2c_dev->begin())
	{
		return false;
	}

	if (read16(MCP9808_REG_MANUF_ID) != 0x0054)
		return false;
	if (read16(MCP9808_REG_DEVICE_ID) != 0x0400)
		return false;

	write16(MCP9808_REG_CONFIG, 0x0);
	return true;
}

/*!
 *   @brief  Reads the 16-bit temperature register and returns the Centigrade
 *           temperature as a float.
 *   @return Temperature in Centigrade.
 */
float Adafruit_MCP9808::readTempC()
{
	float temp = NAN;
	uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

	if (t != 0xFFFF)
	{
		temp = t & 0x0FFF;
		temp /= 16.0;
		if (t & 0x1000)
			temp -= 256;
	}

	return temp;
}

/*!
 *   @brief  Reads the 16-bit temperature register and returns the Fahrenheit
 *           temperature as a float.
 *   @return Temperature in Fahrenheit.
 */
float Adafruit_MCP9808::readTempF()
{
	float temp = NAN;
	uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

	if (t != 0xFFFF)
	{
		temp = t & 0x0FFF;
		temp /= 16.0;
		if (t & 0x1000)
			temp -= 256;

		temp = temp * 9.0 / 5.0 + 32;
	}

	return temp;
}

/*!
 *   @brief  Set Sensor to Shutdown-State or wake up (Conf_Register BIT8)
 *   @param  sw true = shutdown / false = wakeup
 */
void Adafruit_MCP9808::shutdown_wake(boolean sw)
{
	uint16_t conf_shutdown;
	uint16_t conf_register = read16(MCP9808_REG_CONFIG);
	if (sw == true)
	{
		conf_shutdown = conf_register | MCP9808_REG_CONFIG_SHUTDOWN;
		write16(MCP9808_REG_CONFIG, conf_shutdown);
	}
	if (sw == false)
	{
		conf_shutdown = conf_register & ~MCP9808_REG_CONFIG_SHUTDOWN;
		write16(MCP9808_REG_CONFIG, conf_shutdown);
	}
}

/*!
 *   @brief  Shutdown MCP9808
 */
void Adafruit_MCP9808::shutdown() { shutdown_wake(true); }

/*!
 *   @brief  Wake up MCP9808
 */
void Adafruit_MCP9808::wake()
{
	shutdown_wake(false);
	delay(260);
}

/*!
 *   @brief  Get Resolution Value
 *   @return Resolution value
 */
uint8_t Adafruit_MCP9808::getResolution()
{
	return read8(MCP9808_REG_RESOLUTION);
}

/*!
 *   @brief  Set Resolution Value
 *   @param  value
 */
void Adafruit_MCP9808::setResolution(uint8_t value)
{
	write8(MCP9808_REG_RESOLUTION, value & 0x03);
}

/*!
 * @brief Convert temperature float to MCP9808 register format
 * @param temp Temperature in Celsius
 * @return 16-bit register value
 */
uint16_t Adafruit_MCP9808::tempToReg(float temp)
{
	// Temperature is stored in 12 bits (11-0) with 0.25°C resolution for limit registers
	// Bit 12 is sign bit, bits 15-13 should be 0

	int16_t tempInt;
	uint16_t regValue;

	// Convert to 0.25°C resolution (shift left by 2 for bits 11-2)
	tempInt = (int16_t)(temp / 0.25);

	if (temp < 0)
	{
		// Two's complement for negative temperatures
		regValue = (tempInt & 0x0FFF) | 0x1000; // Set sign bit
	}
	else
	{
		regValue = tempInt & 0x0FFF;
	}

	// Shift left by 2 to position in bits 11-2 (bits 1-0 are unused)
	regValue = regValue << 2;

	return regValue;
}

/*!
 * @brief Convert MCP9808 register format to temperature float
 * @param reg 16-bit register value
 * @return Temperature in Celsius
 */
float Adafruit_MCP9808::regToTemp(uint16_t reg)
{
	// Shift right by 2 to get actual temperature bits
	reg = reg >> 2;

	float temp;

	// Check sign bit (bit 12 after shift, which is now bit 10)
	if (reg & 0x1000)
	{
		// Negative temperature
		reg = reg & 0x0FFF; // Clear sign bit
		temp = -(256 - (reg * 0.25));
	}
	else
	{
		temp = reg * 0.25;
	}

	return temp;
}

/*!
 * @brief Set upper temperature boundary
 * @param temp Temperature in Celsius (-40 to +125)
 * @return true on success
 */
bool Adafruit_MCP9808::setUpperTemp(float temp)
{
	if (temp < -40.0 || temp > 125.0)
	{
		return false;
	}

	uint16_t regValue = tempToReg(temp);
	write16(MCP9808_REG_UPPER_TEMP, regValue);
	return true;
}

/*!
 * @brief Set lower temperature boundary
 * @param temp Temperature in Celsius (-40 to +125)
 * @return true on success
 */
bool Adafruit_MCP9808::setLowerTemp(float temp)
{
	if (temp < -40.0 || temp > 125.0)
	{
		return false;
	}

	uint16_t regValue = tempToReg(temp);
	write16(MCP9808_REG_LOWER_TEMP, regValue);
	return true;
}

/*!
 * @brief Set temperature alert hysteresis
 * @param hyst Hysteresis value (MCP9808_HYST_0C, MCP9808_HYST_1_5C,
 *             MCP9808_HYST_3C, or MCP9808_HYST_6C)
 * @return true on success
 * @note Hysteresis applies when temperature decreases below the limit.
 *       For example, with TCRIT=30°C and 1.5°C hysteresis:
 *       - Alert asserts at 30.0°C
 *       - Alert deasserts at 28.5°C
 */
bool Adafruit_MCP9808::setHysteresis(uint8_t hyst)
{
	uint16_t config = read16(MCP9808_REG_CONFIG);

	// Clear hysteresis bits (10-9)
	config &= ~MCP9808_CONFIG_HYST_MASK;

	// Set new hysteresis value (shift hyst value to bits 10-9)
	switch (hyst)
	{
	case MCP9808_HYST_0C:
		config |= MCP9808_CONFIG_HYST_0C;
		break;
	case MCP9808_HYST_1_5C:
		config |= MCP9808_CONFIG_HYST_1_5C;
		break;
	case MCP9808_HYST_3C:
		config |= MCP9808_CONFIG_HYST_3C;
		break;
	case MCP9808_HYST_6C:
		config |= MCP9808_CONFIG_HYST_6C;
		break;
	default:
		return false;
	}

	write16(MCP9808_REG_CONFIG, config);
	return true;
}

/*!
 * @brief Get current temperature alert hysteresis setting
 * @return Current hysteresis setting (0=0°C, 1=1.5°C, 2=3°C, 3=6°C)
 */
uint8_t Adafruit_MCP9808::getHysteresis()
{
	uint16_t config = read16(MCP9808_REG_CONFIG);
	return (config & MCP9808_CONFIG_HYST_MASK) >> 9;
}

/*!
 * @brief Set critical temperature boundary
 * @param temp Temperature in Celsius (-40 to +125)
 * @return true on success
 */
bool Adafruit_MCP9808::setCriticalTemp(float temp)
{
	if (temp < -40.0 || temp > 125.0)
	{
		return false;
	}

	uint16_t regValue = tempToReg(temp);
	write16(MCP9808_REG_CRIT_TEMP, regValue);
	return true;
}

/*!
 * @brief Set alert output polarity
 * @param activeHigh true for active-high, false for active-low
 * @return true on success
 */
bool Adafruit_MCP9808::setAlertPolarity(bool activeHigh)
{
	uint16_t config = read16(MCP9808_REG_CONFIG);

	if (activeHigh)
	{
		config |= MCP9808_REG_CONFIG_ALERTPOL;
	}
	else
	{
		config &= ~MCP9808_REG_CONFIG_ALERTPOL;
	}

	write16(MCP9808_REG_CONFIG, config);
	return true;
}

/*!
 * @brief Set alert output mode
 * @param interruptMode true for interrupt mode, false for comparator mode
 * @return true on success
 */
bool Adafruit_MCP9808::setAlertMode(bool interruptMode)
{
	uint16_t config = read16(MCP9808_REG_CONFIG);

	if (interruptMode)
	{
		config |= MCP9808_REG_CONFIG_ALERTMODE;
	}
	else
	{
		config &= ~MCP9808_REG_CONFIG_ALERTMODE;
	}

	write16(MCP9808_REG_CONFIG, config);
	return true;
}

/*!
 * @brief Enable or disable alert output
 * @param enable true to enable, false to disable
 * @return true on success
 */
bool Adafruit_MCP9808::enableAlert(bool enable)
{
	uint16_t config = read16(MCP9808_REG_CONFIG);

	if (enable)
	{
		config |= MCP9808_REG_CONFIG_ALERTCTRL;
	}
	else
	{
		config &= ~MCP9808_REG_CONFIG_ALERTCTRL;
	}

	write16(MCP9808_REG_CONFIG, config);
	return true;
}

/*!
 * @brief Configure which temperature boundaries trigger the alert output
 * @param critOnly Alert selection mode
 *                 - true: Alert only for TCRIT (critical temperature only)
 *                 - false: Alert for TUPPER, TLOWER, and TCRIT (window + critical)
 * @return true on success
 *
 * @note By default, the MCP9808 monitors all three boundaries (TUPPER, TLOWER, TCRIT).
 *       Set to true when using only setCriticalTemp() for simple over-temperature alerts.
 *       Set to false when using setUpperTemp() and setLowerTemp() for temperature window monitoring.
 */
bool Adafruit_MCP9808::setAlertSelectCriticalOnly(bool critOnly)
{
	uint16_t config = read16(MCP9808_REG_CONFIG);

	if (critOnly)
	{
		config |= MCP9808_REG_CONFIG_ALERTSEL; // Set bit 2 - critical only
	}
	else
	{
		config &= ~MCP9808_REG_CONFIG_ALERTSEL; // Clear bit 2 - all boundaries
	}

	write16(MCP9808_REG_CONFIG, config);
	return true;
}

/*!
 * @brief Convenience function to configure critical temperature alert
 * @param temp Critical temperature threshold in Celsius (-40 to +125)
 * @param activeHigh Alert output polarity
 *                   - true: Active-high (alert pin goes HIGH when triggered)
 *                   - false: Active-low (alert pin goes LOW when triggered)
 * @param alertMode Alert output mode
 *                  - true: Interrupt mode (latching, requires clearInterrupt())
 *                  - false: Comparator mode (automatic, typical for hardware control)
 * @param hysteresis Temperature hysteresis for deassert threshold
 *                   - 0 or MCP9808_HYST_0C: 0°C (no hysteresis)
 *                   - 1 or MCP9808_HYST_1_5C: 1.5°C
 *                   - 2 or MCP9808_HYST_3C: 3.0°C
 *                   - 3 or MCP9808_HYST_6C: 6.0°C
 * @param critOnly Use the critical temperature only
 * @return true on success, false if temperature out of range
 */
bool Adafruit_MCP9808::setTempAlert(float temp, bool activeHigh, bool alertMode, uint8_t hysteresis, bool critOnly)
{
	// Set critical temperature
	if (!setCriticalTemp(temp))
	{
		return false;
	}

	// Set polarity
	setAlertPolarity(activeHigh);

	// Set to comparator mode (typical for simple alerts)
	setAlertMode(alertMode);

	setHysteresis(hysteresis);

	setAlertSelectCriticalOnly(critOnly);

	// Enable alert output
	enableAlert(true);

	return true;
}

/*!
 * @brief Get upper temperature boundary
 * @return Temperature in Celsius
 */
float Adafruit_MCP9808::getUpperTemp()
{
	uint16_t reg = read16(MCP9808_REG_UPPER_TEMP);
	return regToTemp(reg);
}

/*!
 * @brief Get lower temperature boundary
 * @return Temperature in Celsius
 */
float Adafruit_MCP9808::getLowerTemp()
{
	uint16_t reg = read16(MCP9808_REG_LOWER_TEMP);
	return regToTemp(reg);
}

/*!
 * @brief Get critical temperature boundary
 * @return Temperature in Celsius
 */
float Adafruit_MCP9808::getCriticalTemp()
{
	uint16_t reg = read16(MCP9808_REG_CRIT_TEMP);
	return regToTemp(reg);
}

/*!
 * @brief Check if alert is currently asserted
 * @return true if alert condition is met
 */
bool Adafruit_MCP9808::getAlertStatus()
{
	uint16_t config = read16(MCP9808_REG_CONFIG);
	return (config & 0x0010) != 0; // Bit 4 is Alert Status
}

/*!
 *    @brief  Low level 16 bit write procedures
 *    @param  reg
 *    @param  value
 */
void Adafruit_MCP9808::write16(uint8_t reg, uint16_t value)
{
	Adafruit_BusIO_Register reg16 =
		Adafruit_BusIO_Register(i2c_dev, reg, 2, MSBFIRST);

	reg16.write(value);
}

/*!
 *    @brief  Low level 16 bit read procedure
 *    @param  reg
 *    @return value
 */
uint16_t Adafruit_MCP9808::read16(uint8_t reg)
{
	Adafruit_BusIO_Register reg16 =
		Adafruit_BusIO_Register(i2c_dev, reg, 2, MSBFIRST);

	return reg16.read();
}

/*!
 *    @brief  Low level 8 bit write procedure
 *    @param  reg
 *    @param  value
 */
void Adafruit_MCP9808::write8(uint8_t reg, uint8_t value)
{
	Adafruit_BusIO_Register reg8 = Adafruit_BusIO_Register(i2c_dev, reg, 1);

	reg8.write(value);
}

/*!
 *    @brief  Low level 8 bit read procedure
 *    @param  reg
 *    @return value
 */
uint8_t Adafruit_MCP9808::read8(uint8_t reg)
{
	Adafruit_BusIO_Register reg8 = Adafruit_BusIO_Register(i2c_dev, reg, 1);

	return reg8.read();
}

/**************************************************************************/
/*!
	@brief  Gets the pressure sensor and temperature values as sensor events

	@param  temp Sensor event object that will be populated with temp data
	@returns True
*/
/**************************************************************************/
bool Adafruit_MCP9808::getEvent(sensors_event_t *temp)
{
	uint32_t t = millis();

	// use helpers to fill in the events
	memset(temp, 0, sizeof(sensors_event_t));
	temp->version = sizeof(sensors_event_t);
	temp->sensor_id = _sensorID;
	temp->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
	temp->timestamp = t;
	temp->temperature = readTempC();
	return true;
}

/**************************************************************************/
/*!
	@brief  Gets the overall sensor_t data including the type, range and
   resulution
	@param  sensor Pointer to Adafruit_Sensor sensor_t object that will be
   filled with sensor type data
*/
/**************************************************************************/
void Adafruit_MCP9808::getSensor(sensor_t *sensor)
{
	/* Clear the sensor_t object */
	memset(sensor, 0, sizeof(sensor_t));

	/* Insert the sensor name in the fixed length char array */
	strncpy(sensor->name, "MCP9808", sizeof(sensor->name) - 1);
	sensor->name[sizeof(sensor->name) - 1] = 0;
	sensor->version = 1;
	sensor->sensor_id = _sensorID;
	sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
	sensor->min_delay = 0;
	sensor->max_value = 100.0;
	sensor->min_value = -20.0;
	sensor->resolution = 0.0625;
}
/*******************************************************/
