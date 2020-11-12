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
			delete[] temperatures;
		}
	}

	void DS18B20Array::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		if (oneWire == NULL) {
			int owPin = message->argumentAsInt(MSG_DEVICE_PARAMS_START_INDEX);
			oneWire = new OneWire(owPin);
			tempSensors = new DallasTemperature(oneWire);
			tempSensors->begin();
			tempSensors->setResolution(message->argumentAsInt(MSG_DEVICE_PARAMS_START_INDEX + 1)); //set the resolution

			//get number of sensors and their addresses (this is to speed up the handlecommand loop)
			numberOfTempSensors = tempSensors->getDeviceCount();

			if(numberOfTempSensors > 0){
				deviceAddresses = new uint8_t*[numberOfTempSensors];
				temperatures = new float[numberOfTempSensors];

				DeviceAddress tempDeviceAddress;
				for (int i = 0; i < numberOfTempSensors; i++) {
					tempSensors->getAddress(tempDeviceAddress, i);
					deviceAddresses[i] = new uint8_t[8];
					for(int j = 0; j < 8; j++){
						deviceAddresses[i][j] = tempDeviceAddress[j];
					}
					temperatures[i] = 0.0f;
				}
			}
			response->addInt(numberOfTempSensors);
			response->addInt(owPin);

			readInterval = readInterval / numberOfTempSensors;
			lastRead = millis();
		}
	}

	bool DS18B20Array::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch (message->commandType()) {
			case ADMMessage::COMMAND_TYPE_READ:
				if (numberOfTempSensors > 0) {
					response->type = (byte)ADMMessage::TYPE_DATA;
					response->target = target;
					response->addInt(numberOfTempSensors);
					for(int i = 0; i < numberOfTempSensors; i++){
						response->addFloat(temperatures[i]);
					}
				}
				return true;

			default:
				return false;
		}
	}

	ADMMessage* DS18B20Array::loop(){
		if(numberOfTempSensors <= 0 || lastRead <= 0 || readInterval < 1000)return NULL; //we are not ready to read yet

		if(millis() - lastRead > readInterval){
			tempSensors->requestTemperaturesByAddress(deviceAddresses[currentIndex]);

			float celsius = tempSensors->getTempC(deviceAddresses[currentIndex]);
			temperatures[currentIndex] = celsius;
			lastRead = millis();
			currentIndex++;
			if(currentIndex == numberOfTempSensors)currentIndex = 0;
		}
		//
		return NULL;
	}
} //end namespace
