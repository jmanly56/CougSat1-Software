#include "Radio.h"

#include <spdlog/spdlog.h>

namespace GUI {

/**
 * @brief Construct a new Radio:: Radio object
 *
 * @param gui
 */
Radio::Radio(EBGUI_t gui) : Page(gui, "") {}

/**
 * @brief Destroy the Radio:: Radio object
 *
 */
Radio::~Radio() {}

/**
 * @brief Handle when the page first loads
 * Likely send initial values
 *
 * @return Result
 */
Result Radio::onLoad() {
  try {
    createNewEBMessage();

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }

  return sendUpdate();
}

/**
 * @brief Send an update to the page
 * Likely send real time values
 *
 * @return Result
 */
Result Radio::sendUpdate() {
  try {
    createNewEBMessage();

    // TODO get real values
    messageSetProp("link-status", "innerHTML", "Connected");
    messageSetProp("antenna-az", "num", 120.113);
    messageSetProp("antenna-el", "num", 20.73);
    messageSetProp("data-rate", "num", 2130.73);
    messageSetProp("aos", "num", 34.1);
    messageSetProp("los", "num", -547.1);

    messageSetProp("freq-offset", "num", 6821.1);
    messageSetProp("snr", "num", 8.1);
    messageSetProp("ber", "num", 21.13e-6);
    messageSetProp("frames", "num", (int64_t)123);

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Handle the input from the GUI
 *
 * @param msg to handle
 * @return Result
 */
Result Radio::handleInput(const EBMessage_t & msg) {
  try {
    createNewEBMessage();

    switch (msg.id.get()) {
      case Hash::calculateHash("main"):
        return onLoad();
      case Hash::calculateHash("rx-source"):
      case Hash::calculateHash("tx-destination"):
        // TODO implement settings changes
        break;
      default:
        spdlog::info("Unknown id for Radio");
        break;
    }

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }
  return ResultCode_t::SUCCESS;
}

/**
 * @brief Get the buffer for the constellation diagram
 *
 * @return CircularBuffer<PairDouble_t>*
 */
CircularBuffer<PairDouble_t> * Radio::getConstellationBuffer() {
  return &data;
}

/**
 * @brief Send out the constellation once the buffer has at least 100 symbols
 *
 * @return Result
 */
Result Radio::updateConstellation() {
  if (data.size() < 100)
    return ResultCode_t::INCOMPLETE;

  try {
    createNewEBMessage();

    PairDouble_t pair;
    uint8_t      i = 0;
    while (data.pop(pair) == ResultCode_t::SUCCESS && i < 100) {
      messageSetProp("iq-diagram", "dataI-" + std::to_string(i), pair.a);
      messageSetProp("iq-diagram", "dataQ-" + std::to_string(i), pair.b);
      i++;
    }

    enqueueEBMessage();
  } catch (const std::exception & e) {
    return ResultCode_t::EXCEPTION_OCCURRED + e.what();
  }
  return ResultCode_t::SUCCESS;
}

} // namespace GUI