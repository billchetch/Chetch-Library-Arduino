#include <Arduino.h>
#include "../ChetchArduinoDevice.h"
#include "../ChetchADMMessage.h"

namespace Chetch{
  class DS18B20Array : public ArduinoDevice {
    public:
		DS18B20Array(byte tgt, byte cat, char *did, char *dn);

		void configure(bool initial, ADMMessage *message, ADMMessage *response);
  };
} //end namespace	