#include "ChetchADM.h"

namespace Chetch{

  ArduinoDeviceManager::ArduinoDeviceManager(){
    for(int i = 0; i < MAX_DEVICES; i++){
      devices[i] = NULL;
    }
  }

  void ArduinoDeviceManager::initialise(){
    for(int i = 0; i < MAX_DEVICES; i++){
      if(devices[i] != NULL){
        delete devices[i];
        devices[i] = NULL;
      }
    }
    deviceCount = 0;
    initialised = true;
  }

  void ArduinoDeviceManager::reset(){
    initialise();  
  }
  
  ArduinoDeviceManager::~ArduinoDeviceManager(){
    reset();
  }


  ArduinoDevice *ArduinoDeviceManager::getDevice(byte target){
    if(target == 0)return NULL;
    byte idx = target - 1;
    return devices[idx];
  }

  ArduinoDevice *ArduinoDeviceManager::addDevice(byte target, byte category, char *id, char *dname){
    if(target == 0 || deviceCount == MAX_DEVICES)return NULL;
        
    if(getDevice(target) != NULL){
      return NULL; //already exists a device ... this is an error that should be handled
    }
        
    ArduinoDevice *device = new ArduinoDevice(target, category, id, dname);
    devices[target - 1] = device;
    deviceCount++;
        
    return device;
  }
} //end namespace
