#include "ChetchUtils.h"
#include "ChetchADM.h"

#include "devices/ChetchCounter.h"

#if (INCLUDE_DEVICES & TEMPERATURE_DEVICES) == TEMPERATURE_DEVICES
#include "devices/ChetchDS18B20Array.h"
#endif

#if (INCLUDE_DEVICES & RANGE_FINDER_DEVICES) == RANGE_FINDER_DEVICES
#include "devices/ChetchJSN_SR04T.h"
#endif

#if (INCLUDE_DEVICES & IR_DEVICES) == IR_DEVICES
#include "devices/ChetchIRReceiver.h"
#include "devices/ChetchIRTransmitter.h"
#endif

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

  void ArduinoDeviceManager::initialise(ADMFirmataCallbacks* fcb){
    for(int i = 0; i < MAX_DEVICES; i++){
      if(devices[i] != NULL){
        delete devices[i];
        devices[i] = NULL;
      }
    }
    deviceCount = 0;
    initialised = true;
	firmataCallbacks = fcb;
  }

  bool ArduinoDeviceManager::isInitialised(){
		return initialised;
  }

  void ArduinoDeviceManager::reset(){
    initialise(firmataCallbacks);
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

  ArduinoDevice *ArduinoDeviceManager::addDevice(byte target, byte category, char *dname){
    if(target == 0 || deviceCount == MAX_DEVICES)return NULL;
        
    if(getDevice(target) != NULL){
      return NULL; //already exists a device ... this is an error that should be handled
    }
        
	ArduinoDevice *device = NULL;
	char stBuffer[DEVICE_NAME_LENGTH];
	switch ((ArduinoDevice::Category)category) {
#if (INCLUDE_DEVICES & TEMPERATURE_DEVICES) == TEMPERATURE_DEVICES
	case ArduinoDevice::TEMPERATURE_SENSOR:
		if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, 0, DEVICES_TABLE)) == 0) {
			device = new DS18B20Array(target, category, dname);
		} 
		break;
#endif
#if (INCLUDE_DEVICES & RANGE_FINDER_DEVICES) == RANGE_FINDER_DEVICES
	case ArduinoDevice::RANGE_FINDER:
		if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, 1, DEVICES_TABLE)) == 0) {
			device = new JSN_SR04T(target, category, dname);
		}
		break;
#endif
#if (INCLUDE_DEVICES & IR_DEVICES) == IR_DEVICES
	case ArduinoDevice::IR_RECEIVER:
		device = new IRReceiver(target, category, dname);
		break;

	case ArduinoDevice::IR_TRANSMITTER:
		device = new IRTransmitter(target, category, dname);
		break;
#endif
	case ArduinoDevice::COUNTER:
		device = new Counter(target, category, dname);
		break;

	default:
		device = NULL;
		break;
	}
	
	if (device == NULL) {
		device = new ArduinoDevice(target, category, dname);
	}

    devices[target - 1] = device;
    deviceCount++;
        
    return device;
  }

  void ArduinoDeviceManager::loop() {
	  ADMMessage *message;
	  for (int i = 0; i < deviceCount; i++) {
		  message = devices[i]->loop();
		  if (message != NULL && firmataCallbacks != NULL) {
			  message->target = devices[i]->target;
			  firmataCallbacks->sendMessage(message);
			  delete message;
			  delay(1);
		  }
	  }
  }
} //end namespace
