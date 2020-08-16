#include <Arduino.h>

namespace Chetch{
  class ArduinoDevice{
    public:
      static const byte CATEGORY_DIAGNOSTICS = 1;
      static const byte CATEGORY_IR_TRANSMITTER = 2;
      static const byte CATEGORY_IR_RECEIVER = 3;
	  static const byte CATEGORY_TEMPERATURE_SENSOR = 4;
	  static const byte CATEGORY_COUNTER = 5;

      byte target = 0;
      byte category = 0;
      char id[16];
      char name[32];

      ArduinoDevice();
      ArduinoDevice(byte target, byte category, char *did = NULL, char *dn = NULL);
  };
} //end namespace	