#include "ChetchUtils.h"
#include "ChetchDS18B20Array.h"

const char SENSOR_COUNT[] PROGMEM = "SC";
const char ONE_WIRE_PIN[] PROGMEM = "OP";
const char TEMPERATURE[] PROGMEM = "TP";

const char *const PARAMS_TABLE[] PROGMEM = {
	SENSOR_COUNT,
	ONE_WIRE_PIN,
	TEMPERATURE
};

namespace Chetch{

	DS18B20Array::DS18B20Array(byte tgt, byte cat, char *dn) : ArduinoDevice(tgt, cat, dn) {
		//empty
	}

	DS18B20Array::~DS18B20Array(){
		if (tempSensors != NULL)delete tempSensors;
		if (oneWire != NULL)delete oneWire;
	}

	void DS18B20Array::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		if (oneWire == NULL) {
			int owPin = message->argumentAsInt(2);
			oneWire = new OneWire(owPin);
			tempSensors = new DallasTemperature(oneWire);
			tempSensors->begin();
			numberOfTempSensors = tempSensors->getDeviceCount();
			char stBuffer[3];
			response->addInt(Utils::getStringFromProgmem(stBuffer, 0, PARAMS_TABLE), numberOfTempSensors);
			response->addInt(Utils::getStringFromProgmem(stBuffer, 1, PARAMS_TABLE), owPin);
		}
	}

	bool DS18B20Array::handleCommand(ADMMessage *message, ADMMessage *response) {
		switch ((ADMMessage::CommandType)message->command) {
			case ADMMessage::COMMAND_TYPE_READ:
				if (numberOfTempSensors > 0) {
					tempSensors->requestTemperatures();
					DeviceAddress tempDeviceAddress;
					response->type = (byte)ADMMessage::TYPE_DATA;
					response->target = target;
					char stBuffer[3];
					response->addInt(Utils::getStringFromProgmem(stBuffer, 0, PARAMS_TABLE), numberOfTempSensors);
					char tempKey[6]; //note that this limits the max number of sensors to 100!
					for (int i = 0; i < numberOfTempSensors; i++) {
						tempSensors->getAddress(tempDeviceAddress, i);
						float celsius = tempSensors->getTempC(tempDeviceAddress);
						sprintf(tempKey, "%s-%d", Utils::getStringFromProgmem(stBuffer, 2, PARAMS_TABLE), i);
						response->addFloat(tempKey, celsius);
					}
				}
				return true;

			default:
				return false;
		}
	}
} //end namespace
