#include "ChetchUtils.h"
#include "ChetchIRReceiver.h"


namespace Chetch{

	IRReceiver::IRReceiver(byte tgt, byte cat, char *did, char *dn) : ArduinoDevice(tgt, cat, did, dn) {
		//empty
	}

	IRReceiver::~IRReceiver(){
		//empty
	}

	void IRReceiver::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		_receivePin = message->argumentAsByte(3);
		response->addInt("RP", _receivePin);
	}

	bool IRReceiver::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch (message->command) {
			default:
				return false;
		}
	}
} //end namespace
