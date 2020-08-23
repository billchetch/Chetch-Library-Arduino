#include <Arduino.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"

namespace Chetch{
  class JSN_SR04T : public ArduinoDevice {
	private:
		byte _transmitPin;
		byte _receivePin;

    public:
		JSN_SR04T(byte tgt, byte cat, char *dn);
		~JSN_SR04T();
		void configure(bool initial, ADMMessage *message, ADMMessage *response);
		bool handleCommand(ADMMessage *message, ADMMessage *response);
  };
} //end namespace	