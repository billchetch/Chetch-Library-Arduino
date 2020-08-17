#include "ChetchDS18B20Array.h"

namespace Chetch{

	DS18B20Array::DS18B20Array(byte tgt, byte cat, char *did, char *dn) : ArduinoDevice(tgt, cat, did, dn) {
		//empty
	}


	void DS18B20Array::configure(bool initial, ADMMessage *message, ADMMessage *response) {
		ArduinoDevice::configure(initial, message, response);

		response->addValue("Yep", "Cool", false);
	}
} //end namespace
