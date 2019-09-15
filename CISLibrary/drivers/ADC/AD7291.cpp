#include "AD7291.h"

/**
 * @brief Construct a new AD7291::AD7291 object
 *
 * @param i2c connected to the ADC
 * @param addr address of the ADC
 * @param refVoltage reference voltage in volts
 */
AD7291::AD7291(I2C & i2c, AD7291Addr_t addr, double refVoltage) :
  ADC(refVoltage, 12), i2c(i2c), addr(addr) {
  setTemperatureConversionFactor(0.25);
}

/**
 * @brief Destroy the AD7291::AD7291 object
 *
 */
AD7291::~AD7291() {}

/**
 * @brief Read the raw conversion result of a channel
 *
 * @param channel to read
 * @param value to return in counts
 * @param blocking will wait until data is present if true
 * @return mbed_error_status_t
 */
mbed_error_status_t AD7291::readRaw(
    ADCChannel_t channel, int32_t & value, bool blocking) {
  uint16_t            raw    = 0;
  mbed_error_status_t result = MBED_SUCCESS;
  if (channel == ADCChannel_t::TEMP) {
    result = read(Register_t::RESULT_TEMP, raw);
    // Set the MSB bits to the same as the 11th
    // Handles negative temperatures
    value = raw | (0xFFFFF000 * ((raw >> 11) & 0x1));
  } else if (channel < ADCChannel_t::CM_08) {
    // Set the control's channel to the selected channel
    channels = 1 << static_cast<uint8_t>(channel);
    result   = writeControlRegister();
    if (result)
      return result;

    result = read(Register_t::RESULT_VOLTAGE, raw);
    if (result)
      return result;

    if ((raw >> 12) != static_cast<uint8_t>(channel))
      return MBED_ERROR_INVALID_DATA_DETECTED;

    value = raw & 0x00000FFF;
  } else {
    return MBED_ERROR_INVALID_ARGUMENT;
  }
  return result;
}

/**
 * @brief Reset all registers to their default value
 *
 * @return mbed_error_status_t
 */
mbed_error_status_t AD7291::reset() {
  mbed_error_status_t result = write(Register_t::CONTROL, 2);
  if (result)
    return result;
  channels          = 0;
  tempSense         = true;
  noiseDelayed      = true;
  externalReference = false;
  alertActiveLow    = false;
  clearAlert        = false;
  autocycle         = false;
  result            = writeControlRegister();
  return result;
}

/**
 * @brief Read the value of a register
 *
 * @param register to read
 * @param value to return
 * @param registerOffset (used when reading DATA_*_CH0-7)
 * @return mbed_error_status_t
 */
mbed_error_status_t AD7291::read(
    Register_t reg, uint16_t & value, uint8_t registerOffset) {
  char buf[2];
  buf[0] = static_cast<char>(reg) + registerOffset;
  if (i2c.write(static_cast<int>(addr), buf, 1))
    return MBED_ERROR_WRITE_FAILED;

  if (i2c.read(static_cast<int>(addr), buf, 2))
    return MBED_ERROR_READ_FAILED;

  value = buf[0] << 8 | buf[1];
  return MBED_SUCCESS;
}

/**
 * @brief Write the value of a register
 *
 * @param register to write
 * @param value to write
 * @param registerOffset (used when writing DATA_*_CH0-7)
 * @return mbed_error_status_t
 */
mbed_error_status_t AD7291::write(
    Register_t reg, uint16_t value, uint8_t registerOffset) {
  char buf[3];
  buf[0] = static_cast<char>(reg) + registerOffset;
  buf[1] = (value >> 8) & 0xFF;
  buf[2] = (value >> 0) & 0xFF;

  if (i2c.write(static_cast<int>(addr), buf, 3))
    return MBED_ERROR_WRITE_FAILED;
  return MBED_SUCCESS;
}

/**
 * @brief Write the control register
 *
 * @return mbed_error_status_t
 */
mbed_error_status_t AD7291::writeControlRegister() {
  uint16_t buf = 0;

  buf = buf | (channels << 8);
  buf = buf | (static_cast<uint16_t>(tempSense) << 7);
  buf = buf | (static_cast<uint16_t>(noiseDelayed) << 5);
  buf = buf | (static_cast<uint16_t>(externalReference) << 4);
  buf = buf | (static_cast<uint16_t>(alertActiveLow) << 3);
  buf = buf | (static_cast<uint16_t>(clearAlert) << 2);
  buf = buf | (static_cast<uint16_t>(autocycle) << 0);

  return write(Register_t::CONTROL, buf);
}