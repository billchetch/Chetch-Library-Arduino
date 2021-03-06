#ifndef CHETCH_ADM_ADM_H
#define CHETCH_ADM_ADM_H

#include <Arduino.h>
#include "ChetchArduinoDevice.h"
#include "ChetchADMFirmataCallbacks.h"

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

#define TEMPERATURE_DEVICES 1
#define RANGE_FINDER_DEVICES 2
#define IR_DEVICES 4

#define INCLUDE_DEVICES TEMPERATURE_DEVICES


namespace Chetch{
  class ADMFirmataCallbacks;

  class ArduinoDeviceManager{
    private:
      ArduinoDevice *devices[MAX_DEVICES];
      int deviceCount = 0;
      bool initialised = false;
	  ADMFirmataCallbacks* firmataCallbacks = NULL; //set in initialise

    public:	  
      ArduinoDeviceManager();
      ~ArduinoDeviceManager();

	  int getDeviceCount();
	  void initialise(ADMFirmataCallbacks* fcb); //should be when ADM on computer connects
	  bool isInitialised();
      void reset();
      
	  ArduinoDevice *getDevice(byte target);
      ArduinoDevice *addDevice(byte target, byte category, char *dname = NULL);
	  
	  void loop();
	  void sendMessage(ADMMessage* message);

  };
} //end namespace
#endif	