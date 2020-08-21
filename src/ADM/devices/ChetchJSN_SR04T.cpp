#include "ChetchUtils.h"
#include "ChetchJSN_SR04T.h"

const char DURATION[] PROGMEM = "Duration";

const char *const PARAMS_TABLE[] PROGMEM = {
	DURATION
};

namespace Chetch{

	JSN_SR04T::JSN_SR04T(byte tgt, byte cat, char *did, char *dn) : ArduinoDevice(tgt, cat, did, dn) {
		//empty
	}

	JSN_SR04T::~JSN_SR04T(){
		//empty
	}

	void JSN_SR04T::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		_transmitPin = message->argumentAsByte(3);
		_receivePin = message->argumentAsByte(4);
	}

	bool JSN_SR04T::handleCommand(ADMMessage *message, ADMMessage *response) {
		long duration;
		switch (message->command) {
			case ADMMessage::COMMAND_TYPE_READ:
				
				for (int i = 0; i < 2; i++) {
					digitalWrite(_transmitPin, LOW);

					delayMicroseconds(5);
					// Trigger the sensor by setting the transmitPin high for 10 microseconds:
					digitalWrite(_transmitPin, HIGH);
					delayMicroseconds(10);
					digitalWrite(_transmitPin, LOW);

					// Read the receivePin. pulseIn() returns the duration (length of the pulse) in microseconds:
					duration = pulseIn(_receivePin, HIGH);
					if (duration > 0)break;
				}
				response->type = ADMMessage::TYPE_DATA;
				char stBuffer[16];
				response->addLong(Utils::getStringFromProgmem(stBuffer, 0, PARAMS_TABLE), duration);
				return true;

			default:
				return false;
		}
	}
} //end namespace
