#include "ChetchUtils.h"
#include "ChetchCounter.h"

const char COUNT[] PROGMEM = "CT"; //Count
const char INTERVAL[] PROGMEM = "IV"; //Interval

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
		float rate = 0;
		switch (message->commandType()) {
			case ADMMessage::COMMAND_TYPE_READ:
				interval = millis() - _lastRead;
				switch(message->commandIndex()){
					case 0: //Count
						response->addLong(_counter);
						response->addLong(interval);
						break;
					case 1: //Rate
						rate = ((float)_counter / (float)interval) * 1000.0;
						response->addFloat(rate);
						break;

					default:
						break;

				}
				
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
