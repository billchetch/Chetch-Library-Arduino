#include <Arduino.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"

namespace Chetch{
  class Counter : public ArduinoDevice {
	private:
		byte _countPin;
		byte _countState;
		byte _prevState;
		unsigned long _lastRead = 0;
		unsigned long _counter = 0;
	
    public:
		Counter(byte tgt, byte cat, char *dn);
		~Counter();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
		ADMMessage* loop();
  };
} //end namespace	