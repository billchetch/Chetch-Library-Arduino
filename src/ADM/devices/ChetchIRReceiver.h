#include <Arduino.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"

namespace Chetch{
  class IRReceiver : public ArduinoDevice {
	private:
		byte receivePin;
		IRrecv *irReceiver = NULL;
		bool recording = false;
		
    public:
		IRReceiver(byte tgt, byte cat, char *did, char *dn);
		~IRReceiver();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
		ADMMessage* loop();
  };
} //end namespace	