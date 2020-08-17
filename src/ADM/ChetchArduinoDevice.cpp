#include "ChetchArduinoDevice.h"

namespace Chetch{

  ArduinoDevice::ArduinoDevice(){
  }
  
  ArduinoDevice::ArduinoDevice(byte tgt, byte cat, char *did, char *dn){
    target = tgt;
    category = cat;

    if(did != NULL){
      for(int i = 0; i < strlen(did); i++){
        id[i] = did[i];
      }
      id[strlen(did)] = 0;
    }
    
    if(dn != NULL){
      for(int i = 0; i < strlen(dn); i++){
        name[i] = dn[i];
      }
      name[strlen(dn)] = 0;
    }
  }


  void ArduinoDevice::handleStatusRequest(ADMMessage *message, ADMMessage *response) {
	  //A hook
  }

  void ArduinoDevice::configure(bool initial, ADMMessage *message, ADMMessage *response) {
	  //A hook
  }

  bool ArduinoDevice::handleCommand(ADMMessage *message, ADMMessage *response) {
	  //A hook... return true to send response, false to cancel send
	  return false;
  }
} //end namespace
