#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
#include "ChetchArduinoDevice.h"

#if defined(ARDUINO_AVR_UNO)
	//Uno specific code
	#define MAX_DEVICES 8	
#elif defined(ARDUINO_AVR_MEGA2560)
	//Mega 2560 specific code
	#define MAX_DEVICES 64
#elif defined(ARDUINO_SAM_DUE)
	#define MAX_DEVICES 16
#else
#error Unsupported hardware
#endif

namespace Chetch{
  class ArduinoDeviceManager{
    private:
      ArduinoDevice *devices[MAX_DEVICES];
      int deviceCount = 0;
      bool initialised = false;

    public:
      ArduinoDeviceManager();
      ~ArduinoDeviceManager();

      int getDeviceCount(){
        return deviceCount;
      }

      void initialise(); //should be when ADM on computer connects
      void reset();
      
      ArduinoDevice *getDevice(byte target);
      ArduinoDevice *addDevice(byte target, byte category, char *id = NULL, char *dname = NULL);
  };
} //end namespace

#endif	