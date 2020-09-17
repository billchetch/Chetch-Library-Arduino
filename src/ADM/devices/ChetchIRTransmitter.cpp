#include "ChetchIRTransmitter.h"
#include "ChetchUtils.h"

const unsigned int LGHT_REPEAT[] PROGMEM = { 4500,4400,600,1600,600 };

const unsigned int IR_RAW_CODE_LENGTHS[] PROGMEM = { 5 };
const unsigned int *const IR_RAW_CODES[] PROGMEM = {
	LGHT_REPEAT
};


namespace Chetch{

	IRTransmitter::IRTransmitter(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
		
	}

	IRTransmitter::~IRTransmitter(){
		if(irSender != NULL)delete irSender;
	}

	void IRTransmitter::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		irSender = new IRsend();
		response->addInt("TP", SEND_PIN); //defined by the IRremote library
	}

	bool IRTransmitter::handleCommand(ADMMessage *message, ADMMessage *response) {
		if (irSender == NULL)return false;

		unsigned int n;
		unsigned int raw[8];

		if(message->command == (byte)ADMMessage::COMMAND_TYPE_SEND){
			unsigned long ircommand = message->argumentAsULong(0);
			int bits = message->argumentAsInt(1);
			int protocol = message->argumentAsInt(2);

			switch (protocol) {
				case SAMSUNG: //7
					irSender->sendSAMSUNG(ircommand, bits);
					break;
				case LG: //10
					irSender->sendLG(ircommand, bits);
					break;
				case NEC: //3
					irSender->sendNEC(ircommand, bits);
					break;

				case UNKNOWN: //we send as raw
					n = Utils::getUIntArrayFromProgmem(raw, (int)ircommand, IR_RAW_CODES, IR_RAW_CODE_LENGTHS);
					irSender->sendRaw(raw, n, 38);
					break;

				default:
					break;
			}
		} 

		return false;
	}
} //end namespace
