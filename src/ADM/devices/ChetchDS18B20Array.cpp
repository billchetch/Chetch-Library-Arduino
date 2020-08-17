#include "ChetchDS18B20Array.h"

namespace Chetch{

	DS18B20Array::DS18B20Array(byte tgt, byte cat, char *did, char *dn) : ArduinoDevice(tgt, cat, did, dn) {
		//empty
	}

	DS18B20Array::~DS18B20Array(){
		if (tempSensors != NULL)delete tempSensors;
		if (oneWire != NULL)delete oneWire;
	}

	void DS18B20Array::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		if (initial) {
			if (tempSensors != NULL)delete tempSensors;
			if (oneWire != NULL)delete oneWire;
		}

		if (oneWire == NULL) {
			int owPin = message->argumentAsInt(3);
			oneWire = new OneWire(owPin);
			tempSensors = new DallasTemperature(oneWire);
			tempSensors->begin();
			numberOfTempSensors = tempSensors->getDeviceCount();
			response->addInt("SensorCount", numberOfTempSensors);
			response->addInt("OneWirePin", owPin);
		}
	}

	bool DS18B20Array::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch (message->command) {
			case ADMMessage::COMMAND_TYPE_READ:
				if (numberOfTempSensors > 0) {
					tempSensors->requestTemperatures();
					DeviceAddress tempDeviceAddress;
					response->type = ADMMessage::TYPE_DATA;
					response->target = target;
					response->addInt("SensorCount", numberOfTempSensors);
					char tempKey[16];
					for (int i = 0; i < numberOfTempSensors; i++) {
						tempSensors->getAddress(tempDeviceAddress, i);
						float celsius = tempSensors->getTempC(tempDeviceAddress);
						sprintf(tempKey, "Temperature-%d", i);
						response->addFloat(tempKey, celsius);
					}
				}
				return true;

			default:
				return false;
		}
	}
} //end namespace
