#include "ChetchUtils.h"
#include "ChetchIRReceiver.h"
#include "ChetchUtils.h"
#include <MemoryFree.h>

const char STARTED_MESSAGE[] PROGMEM = "Started rec.";
const char STOPPED_MESSAGE[] PROGMEM = "Stopped rec.";

const char *const MESSAGES_TABLE[] PROGMEM = {
	STARTED_MESSAGE,
	STOPPED_MESSAGE
};

namespace Chetch{

	IRReceiver::IRReceiver(byte tgt, byte cat, char *did, char *dn) : ArduinoDevice(tgt, cat, did, dn) {
		//empty
		
	}

	IRReceiver::~IRReceiver(){
		if(irReceiver != NULL)delete irReceiver;
	}

	void IRReceiver::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		receivePin = message->argumentAsByte(3);
		irReceiver = new IRrecv(receivePin);
		irReceiver->enableIRIn();
	}

	bool IRReceiver::handleCommand(ADMMessage *message, ADMMessage *response) {
		char stBuffer[16];
		switch (message->command) {
			case ADMMessage::COMMAND_TYPE_START:
				irReceiver->resume();
				recording = true;
				response->type = ADMMessage::TYPE_INFO;
				response->setValue(Utils::getStringFromProgmem(stBuffer, 0, MESSAGES_TABLE));
				return true;

			case ADMMessage::COMMAND_TYPE_STOP:
				irReceiver->resume();
				recording = false;
				response->type = ADMMessage::TYPE_INFO;
				response->setValue(Utils::getStringFromProgmem(stBuffer, 1, MESSAGES_TABLE));
				return true;

			default:
				return false;
		}
	}

	ADMMessage* IRReceiver::loop() {
		static unsigned long elapsed = 0;

		if (millis() - elapsed > 1000) {
			ADMMessage *message = new ADMMessage(4);
			message->type = ADMMessage::TYPE_DATA;
			message->addInt("FM", freeMemory());
			elapsed = millis();
			return message;
		} else {
			return NULL;
		}
	}

} //end namespace
