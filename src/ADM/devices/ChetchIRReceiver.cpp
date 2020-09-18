#include "ChetchUtils.h"
#include "ChetchIRReceiver.h"

const char STARTED_MESSAGE[] PROGMEM = "Started rec.";
const char STOPPED_MESSAGE[] PROGMEM = "Stopped rec.";
const char CODE_PARAM[] PROGMEM = "Code";
const char PROTOCOL_PARAM[] PROGMEM = "Protocol";
const char BITS_PARAM[] PROGMEM = "Bits";

const char *const MESSAGES_TABLE[] PROGMEM = {
	STARTED_MESSAGE,
	STOPPED_MESSAGE
};

const char *const PARAMS_TABLE[] PROGMEM = {
	CODE_PARAM,
	PROTOCOL_PARAM,
	BITS_PARAM
};

namespace Chetch{

	IRReceiver::IRReceiver(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRReceiver::~IRReceiver(){
		if(irReceiver != NULL)delete irReceiver;
	}

	void IRReceiver::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		receivePin = message->argumentAsByte(2);
		irReceiver = new IRrecv(receivePin);
		irReceiver->enableIRIn();
		recording = false;
	}

	bool IRReceiver::handleCommand(ADMMessage *message, ADMMessage *response) {
		char stBuffer[16];
		switch ((ADMMessage::CommandType)message->command) {
			case ADMMessage::COMMAND_TYPE_START:
				irReceiver->resume();
				recording = true;
				response->type = (byte)ADMMessage::TYPE_INFO;
				response->setValue(Utils::getStringFromProgmem(stBuffer, 0, MESSAGES_TABLE));
				return true;

			case ADMMessage::COMMAND_TYPE_STOP:
				irReceiver->resume();
				recording = false;
				response->type = (byte)ADMMessage::TYPE_INFO;
				response->setValue(Utils::getStringFromProgmem(stBuffer, 1, MESSAGES_TABLE));
				return true;

			default:
				return false;
		}
	}

	ADMMessage* IRReceiver::loop() {
		if (irReceiver == NULL || !recording)return NULL;

		static unsigned long elapsed = 0;
		if ((millis() - elapsed > 100) && irReceiver->decode(&irReceiverResults)) {
			elapsed = millis();
			
			char stBuffer[8];
			ADMMessage *message = new ADMMessage(4);
			message->type = (byte)ADMMessage::TYPE_DATA;
			message->addLong(Utils::getStringFromProgmem(stBuffer, 0, PARAMS_TABLE), irReceiverResults.value); //Code
			message->addInt(Utils::getStringFromProgmem(stBuffer, 1, PARAMS_TABLE), irReceiverResults.decode_type); //Protocol
			message->addInt(Utils::getStringFromProgmem(stBuffer, 2, PARAMS_TABLE), irReceiverResults.bits); //Bits

			irReceiver->resume(); //ready for next result
			
			return message;
		} else {
			return NULL;
		}
	}

} //end namespace
