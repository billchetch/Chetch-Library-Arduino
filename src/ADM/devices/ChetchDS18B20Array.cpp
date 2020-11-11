#include "ChetchUtils.h"
#include "ChetchDS18B20Array.h"

namespace Chetch{

	DS18B20Array::DS18B20Array(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
	}

	DS18B20Array::~DS18B20Array(){
		if (tempSensors != NULL)delete tempSensors;
		if (oneWire != NULL)delete oneWire;
		if(numberOfTempSensors > 0){
			for(int i = 0; i < numberOfTempSensors; i++){
				delete[] deviceAddresses[i];
			}
			delete[] deviceAddresses;
		}
	}

	void DS18B20Array::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		if (oneWire == NULL) {
			int owPin = message->argumentAsInt(2);
			oneWire = new OneWire(owPin);
			tempSensors = new DallasTemperature(oneWire);
			tempSensors->begin();
			tempSensors->setResolution(9); //0.5 degrees celsius .... faster look up times than 11 or 12 bit

			//get number of sensors and their addresses (this is to speed up the handlecommand loop)
			numberOfTempSensors = tempSensors->getDeviceCount();
			deviceAddresses = new uint8_t*[numberOfTempSensors];
			DeviceAddress tempDeviceAddress;
			for (int i = 0; i < numberOfTempSensors; i++) {
				tempSensors->getAddress(tempDeviceAddress, i);
				deviceAddresses[i] = new uint8_t[8];
				for(int j = 0; j < 8; j++){
					deviceAddresses[i][j] = tempDeviceAddress[j];
				}
			}

			response->addInt(numberOfTempSensors);
			response->addInt(owPin);
		}
	}

	bool DS18B20Array::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch (message->commandType()) {
			case ADMMessage::COMMAND_TYPE_READ:
				if (numberOfTempSensors > 0) {
					tempSensors->requestTemperatures();
					response->type = (byte)ADMMessage::TYPE_DATA;
					response->target = target;
					response->addInt(numberOfTempSensors);
					
					for (int i = 0; i < numberOfTempSensors; i++) {
						float celsius = tempSensors->getTempC(deviceAddresses[i]);
						response->addFloat(celsius);
					}
				}
				return true;

			default:
				return false;
		}
	}
} //end namespace
