#ifndef CHETCH_ADM_ARDUINODEVICE_h
#define CHETCH_ADM_ARDUINODEVICE_h

#include <Arduino.h>
#include "ChetchADMMessage.h"

namespace Chetch{
  class ArduinoDevice{
    public:
      static const byte CATEGORY_DIAGNOSTICS = 1;
      static const byte CATEGORY_IR_TRANSMITTER = 2;
      static const byte CATEGORY_IR_RECEIVER = 3;
	  static const byte CATEGORY_TEMPERATURE_SENSOR = 4;
	  static const byte CATEGORY_COUNTER = 5;
	  static const byte CATEGORY_RANGE_FINDER = 6;

      byte target = 0;
      byte category = 0;
      char id[8];
      char name[10];

      ArduinoDevice();
      ArduinoDevice(byte target, byte category, char *did = NULL, char *dn = NULL);

	  virtual void handleStatusRequest(ADMMessage *message, ADMMessage *response);
	  virtual void configure(bool initial, ADMMessage *message, ADMMessage *response);
	  virtual bool handleCommand(ADMMessage *message, ADMMessage *response);
  };
} //end namespace

#endif