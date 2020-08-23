#include "ChetchUtils.h"
#include "ChetchADM.h"
#include "devices/ChetchDS18B20Array.h"
#include "devices/ChetchJSN_SR04T.h"
#include "devices/ChetchIRReceiver.h"
#include "devices/ChetchIRTransmitter.h"

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

	switch (category) {
	case ArduinoDevice::CATEGORY_TEMPERATURE_SENSOR:
		if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, 0, DEVICES_TABLE)) == 0) {
			device = new DS18B20Array(target, category, dname);
		} 
		break;

	case ArduinoDevice::CATEGORY_RANGE_FINDER:
		if (strcmp(dname, Utils::getStringFromProgmem(stBuffer, 1, DEVICES_TABLE)) == 0) {
			device = new JSN_SR04T(target, category, dname);
		}
		break;

	case ArduinoDevice::CATEGORY_IR_RECEIVER:
		device = new IRReceiver(target, category, dname);
		break;

	case ArduinoDevice::CATEGORY_IR_TRANSMITTER:
		device = new IRTransmitter(target, category, dname);
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
