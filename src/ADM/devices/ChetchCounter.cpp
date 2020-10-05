#include "ChetchUtils.h"
#include "ChetchCounter.h"

const char COUNT[] PROGMEM = "Count";
const char INTERVAL[] PROGMEM = "Interval";

const char *const PARAMS_TABLE[] PROGMEM = {
	COUNT,
	INTERVAL
};

namespace Chetch{

	Counter::Counter(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
	}

	Counter::~Counter(){
		//empty
	}

	void Counter::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		_countPin = message->argumentAsByte(2);
		_countState = message->argumentAsByte(3);

		_prevState = digitalRead(_countPin);

		_lastRead = millis();
	}

	bool Counter::handleCommand(ADMMessage *message, ADMMessage *response) {
		unsigned long interval = 0;
		switch ((ADMMessage::CommandType)message->command) {
			case ADMMessage::COMMAND_TYPE_READ:
				interval = millis() - _lastRead;
				char stBuffer[16];
				response->addLong(Utils::getStringFromProgmem(stBuffer, 0, PARAMS_TABLE), _counter);
				response->addLong(Utils::getStringFromProgmem(stBuffer, 1, PARAMS_TABLE), interval);
				_lastRead = millis();
				_counter = 0; //reset
				return true;

			default:
				return false;
		}
	}

	ADMMessage* Counter::loop() {
		if (category > 0) { // means already configured
			byte state = digitalRead(_countPin);
			if (state != _prevState) {
				if (state == _countState)_counter++;
				_prevState = state;
			}
		}
		return NULL;
	}
} //end namespace
