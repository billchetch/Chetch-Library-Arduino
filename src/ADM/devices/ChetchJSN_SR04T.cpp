#include "ChetchUtils.h"
#include "ChetchJSN_SR04T.h"

namespace Chetch{

	JSN_SR04T::JSN_SR04T(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
	}

	JSN_SR04T::~JSN_SR04T(){
		//empty
	}

	void JSN_SR04T::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		_transmitPin = message->argumentAsByte(2);
		_receivePin = message->argumentAsByte(3);
	}

	bool JSN_SR04T::handleCommand(ADMMessage *message, ADMMessage *response) {
		long duration;
		switch (message->commandType()) {
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
				response->type = (byte)ADMMessage::TYPE_DATA;
				response->addLong(duration);
				return true;

			default:
				return false;
		}
	}
} //end namespace
