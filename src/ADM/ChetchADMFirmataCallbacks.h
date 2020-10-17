#ifndef CHETCH_ADM_FIRMATA_CALLBACKS_H
#define CHETCH_ADM_FIRMATA_CALLBACKS_H

#include <Arduino.h>
#include "../Firmata/ChetchFirmataCallbacks.h"
#include "ChetchADMMessage.h"
#include "ChetchADM.h"


namespace Chetch{
  class ArduinoDeviceManager;

  class ADMFirmataCallbacks : public FirmataCallbacks{
    	public:
			static ArduinoDeviceManager ADM;
			
			virtual void initialise();
			virtual void sendMessage(ADMMessage *message);
			virtual void respond(ADMMessage *message, ADMMessage *response);
			virtual void configure(ADMMessage *message, ADMMessage *response);
			virtual void handleMessage(ADMMessage *message);
			virtual void handleString(char *s);
			virtual bool handleCommand(ADMMessage *message, ADMMessage *response);
			virtual void loop();
			void heartbeat();
  };
} //end namespace	
#endif