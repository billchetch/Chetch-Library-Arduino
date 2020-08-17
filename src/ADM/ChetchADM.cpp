#include "ChetchUtils.h"
#include "ChetchADM.h"
#include "devices/ChetchDS18B20Array.h"

const char DS18B20[] PROGMEM = "DS18B20";
const char JSN_SR04T[] PROGMEM = "JSN-SR04T";

const char *const DEVICES_TABLE[] PROGMEM = {
	DS18B20,
	JSN_SR04T
};

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

  int ArduinoDeviceManager::getDeviceCount(){
	  return deviceCount;
  }

  ArduinoDevice *ArduinoDeviceManager::getDevice(byte target){
    if(target == 0 || target < 1 || target > deviceCount)return NULL;
    byte idx = target - 1;
    return devices[idx];
  }

  ArduinoDevice *ArduinoDeviceManager::addDevice(byte target, byte category, char *id, char *dname){
    if(target == 0 || deviceCount == MAX_DEVICES)return NULL;
        
    if(getDevice(target) != NULL){
      return NULL; //already exists a device ... this is an error that should be handled
    }
        
	ArduinoDevice *device;

	switch (category) {
	case ArduinoDevice::CATEGORY_TEMPERATURE_SENSOR:
		char stBuffer[16];
		if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, 0, DEVICES_TABLE)) == 0) {
			device = new DS18B20Array(target, category, id, dname);
		} else {
			device = new ArduinoDevice(target, category, id, dname);
		}
		break;

	default:
		device = new ArduinoDevice(target, category, id, dname);
		break;
	}
	
    devices[target - 1] = device;
    deviceCount++;
        
    return device;
  }
} //end namespace
