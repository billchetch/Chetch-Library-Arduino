#include "ChetchFirmataCallbacks.h"

namespace Chetch{

FirmataCallbacks *FirmataCallbacks::FCB = NULL;
void FirmataCallbacks::init(FirmataCallbacks* fcb, const char* boardID, int options = 0) {

	if (FirmataCallbacks::FCB == NULL) {
		fcb->boardID = boardID;
		FirmataCallbacks::FCB = fcb;
		
		Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
		Firmata.attach(STRING_DATA, FirmataCallbacks::stringCallback);
		Firmata.attach(START_SYSEX, FirmataCallbacks::sysexCallback);
		Firmata.attach(DIGITAL_MESSAGE, FirmataCallbacks::digitalWriteCallback);
		Firmata.attach(SET_PIN_MODE, FirmataCallbacks::setPinModeCallback);
		Firmata.attach(SET_DIGITAL_PIN_VALUE, FirmataCallbacks::setPinValueCallback);
		Firmata.attach(ANALOG_MESSAGE, FirmataCallbacks::analogWriteCallback);
		//Firmata.attach(REPORT_ANALOG, FirmataCallbacks::reportAnalogCallback);
		//Firmata.attach(REPORT_DIGITAL, FirmataCallbacks::reportDigitalCallback);
		//Firmata.attach(SYSTEM_RESET, FirmataCallbacks::systemResetCallback);
		
		Firmata.begin(57600);
	}
}

void FirmataCallbacks::stringCallback(char *s){
	FCB->handleString(s);
}

void FirmataCallbacks::sysexCallback(byte command, byte argc, byte *argv){
	FCB->handleSysex(command, argc, argv);
}

void FirmataCallbacks::setPinModeCallback(byte pin, int mode) {
	FCB->handleSetPinMode(pin, mode);
}

void FirmataCallbacks::setPinValueCallback(byte pin, int value) {
	FCB->handleSetPinValue(pin, value);
}

void FirmataCallbacks::analogWriteCallback(byte pin, int value) {
	FCB->handleAnalogWrite(pin, value);
}

void FirmataCallbacks::digitalWriteCallback(byte port, int value) {
	FCB->handleDigitalWrite(port, value);
}

void FirmataCallbacks::reportAnalogCallback(byte analogPin, int value) {
	FCB->handleReportAnalog(analogPin, value);
}

void FirmataCallbacks::reportDigitalCallback(byte port, int value) {
	FCB->handleReportDigital(port, value);
}
//END STATIC

//Instance methods
void FirmataCallbacks::handleString(char *s) {
	//NOT USED (yet)
}

/*==============================================================================
* SYSEX-BASED commands
*============================================================================*/

void FirmataCallbacks::handleSysex(byte command, byte argc, byte *argv)
{
	byte mode;
	byte stopTX;
	byte slaveAddress;
	byte data;
	int slaveRegister;
	unsigned int delayTime;

	switch (command) {
	case CAPABILITY_QUERY:
		Firmata.write(START_SYSEX);
		Firmata.write(CAPABILITY_RESPONSE);
		for (byte pin = 0; pin < TOTAL_PINS; pin++) {
			if (IS_PIN_DIGITAL(pin)) {
				Firmata.write((byte)INPUT);
				Firmata.write(1);
				Firmata.write((byte)PIN_MODE_PULLUP);
				Firmata.write(1);
				Firmata.write((byte)OUTPUT);
				Firmata.write(1);
			}
			if (IS_PIN_ANALOG(pin)) {
				Firmata.write(PIN_MODE_ANALOG);
				Firmata.write(10); // 10 = 10-bit resolution
			}
			if (IS_PIN_PWM(pin)) {
				Firmata.write(PIN_MODE_PWM);
				Firmata.write(DEFAULT_PWM_RESOLUTION);
			}
			if (IS_PIN_DIGITAL(pin)) {
				Firmata.write(PIN_MODE_SERVO);
				Firmata.write(14);
			}
			if (IS_PIN_I2C(pin)) {
				Firmata.write(PIN_MODE_I2C);
				Firmata.write(1);  // TODO: could assign a number to map to SCL or SDA
			}
#ifdef FIRMATA_SERIAL_FEATURE
			serialFeature.handleCapability(pin);
#endif
			Firmata.write(127);
		}
		Firmata.write(END_SYSEX);
		break;
	case PIN_STATE_QUERY:
		if (argc > 0) {
			byte pin = argv[0];
			Firmata.write(START_SYSEX);
			Firmata.write(PIN_STATE_RESPONSE);
			Firmata.write(pin);
			if (pin < TOTAL_PINS) {
				Firmata.write(Firmata.getPinMode(pin));
				Firmata.write((byte)Firmata.getPinState(pin) & 0x7F);
				if (Firmata.getPinState(pin) & 0xFF80) Firmata.write((byte)(Firmata.getPinState(pin) >> 7) & 0x7F);
				if (Firmata.getPinState(pin) & 0xC000) Firmata.write((byte)(Firmata.getPinState(pin) >> 14) & 0x7F);
			}
			Firmata.write(END_SYSEX);
		}
		break;
	case ANALOG_MAPPING_QUERY:
		Firmata.write(START_SYSEX);
		Firmata.write(ANALOG_MAPPING_RESPONSE);
		for (byte pin = 0; pin < TOTAL_PINS; pin++) {
			Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
		}
		Firmata.write(END_SYSEX);
		break;

	case SAMPLING_INTERVAL:
		/*if (argc > 1) {
		samplingInterval = argv[0] + (argv[1] << 7);
		if (samplingInterval < MINIMUM_SAMPLING_INTERVAL) {
		samplingInterval = MINIMUM_SAMPLING_INTERVAL;
		}
		} else {
		//Firmata.sendString("Not enough data");
		}*/
		break;

	case SERIAL_MESSAGE:
#ifdef FIRMATA_SERIAL_FEATURE
		serialFeature.handleSysex(command, argc, argv);
#endif
		break;
	}
}



void FirmataCallbacks::handleSetPinMode(byte pin, int mode) {
	if (Firmata.getPinMode(pin) == PIN_MODE_IGNORE)
		return;

	if (IS_PIN_ANALOG(pin)) {
		reportAnalogCallback(PIN_TO_ANALOG(pin), mode == PIN_MODE_ANALOG ? 1 : 0); // turn on/off reporting
	}
	if (IS_PIN_DIGITAL(pin)) {
		if (mode == INPUT || mode == PIN_MODE_PULLUP) {
			portConfigInputs[pin / 8] |= (1 << (pin & 7));
		}
		else {
			portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
		}
	}
	Firmata.setPinState(pin, 0);
	switch (mode) {
	case PIN_MODE_ANALOG:
		if (IS_PIN_ANALOG(pin)) {
			if (IS_PIN_DIGITAL(pin)) {
				pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
														// deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
				digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
			}
			Firmata.setPinMode(pin, PIN_MODE_ANALOG);
		}
		break;
	case INPUT:
		if (IS_PIN_DIGITAL(pin)) {
			pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
													// deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
			digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
			Firmata.setPinMode(pin, INPUT);
		}
		break;
	case PIN_MODE_PULLUP:
		if (IS_PIN_DIGITAL(pin)) {
			pinMode(PIN_TO_DIGITAL(pin), INPUT_PULLUP);
			Firmata.setPinMode(pin, PIN_MODE_PULLUP);
			Firmata.setPinState(pin, 1);
		}
		break;
	case OUTPUT:
		if (IS_PIN_DIGITAL(pin)) {
			if (Firmata.getPinMode(pin) == PIN_MODE_PWM) {
				// Disable PWM if pin mode was previously set to PWM.
				digitalWrite(PIN_TO_DIGITAL(pin), LOW);
			}
			pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
			Firmata.setPinMode(pin, OUTPUT);
		}
		break;
	case PIN_MODE_PWM:
		if (IS_PIN_PWM(pin)) {
			pinMode(PIN_TO_PWM(pin), OUTPUT);
			analogWrite(PIN_TO_PWM(pin), 0);
			Firmata.setPinMode(pin, PIN_MODE_PWM);
		}
		break;

	case PIN_MODE_SERIAL:
#ifdef FIRMATA_SERIAL_FEATURE
		serialFeature.handlePinMode(pin, PIN_MODE_SERIAL);
#endif
		break;
	default:
		Firmata.sendString("Unknown pin mode"); // TODO: put error msgs in EEPROM
	}
}

void FirmataCallbacks::handleSetPinValue(byte pin, int value)
{
	if (pin < TOTAL_PINS && IS_PIN_DIGITAL(pin)) {
		if (Firmata.getPinMode(pin) == OUTPUT) {
			Firmata.setPinState(pin, value);
			digitalWrite(PIN_TO_DIGITAL(pin), value);
		}
	}
}

void FirmataCallbacks::handleAnalogWrite(byte pin, int value)
{
	if (pin < TOTAL_PINS) {
		switch (Firmata.getPinMode(pin)) {
		case PIN_MODE_PWM:
			if (IS_PIN_PWM(pin))
				analogWrite(PIN_TO_PWM(pin), value);
			Firmata.setPinState(pin, value);
			break;
		}
	}
}


void FirmataCallbacks::handleDigitalWrite(byte port, int value)
{
	byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

	if (port < TOTAL_PORTS) {
		// create a mask of the pins on this port that are writable.
		lastPin = port * 8 + 8;
		if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
		for (pin = port * 8; pin < lastPin; pin++) {
			// do not disturb non-digital pins (eg, Rx & Tx)
			if (IS_PIN_DIGITAL(pin)) {
				// do not touch pins in PWM, ANALOG, SERVO or other modes
				if (Firmata.getPinMode(pin) == OUTPUT || Firmata.getPinMode(pin) == INPUT) {
					pinValue = ((byte)value & mask) ? 1 : 0;
					if (Firmata.getPinMode(pin) == OUTPUT) {
						pinWriteMask |= mask;
					}
					else if (Firmata.getPinMode(pin) == INPUT && pinValue == 1 && Firmata.getPinState(pin) != 1) {
						// only handle INPUT here for backwards compatibility
#if ARDUINO > 100
						pinMode(pin, INPUT_PULLUP);
#else
						// only write to the INPUT pin to enable pullups if Arduino v1.0.0 or earlier
						pinWriteMask |= mask;
#endif
					}
					Firmata.setPinState(pin, pinValue);
				}
			}
			mask = mask << 1;
		}
		writePort(port, (byte)value, pinWriteMask);
	}
}

void FirmataCallbacks::outputPort(byte portNumber, byte portValue, byte forceSend)
{
	// pins not configured as INPUT are cleared to zeros
	portValue = portValue & portConfigInputs[portNumber];
	// only send if the value is different than previously sent
	if (forceSend || previousPINs[portNumber] != portValue) {
		Firmata.sendDigitalPort(portNumber, portValue);
		previousPINs[portNumber] = portValue;
	}
}

void FirmataCallbacks::handleReportAnalog(byte analogPin, int value)
{
	if (analogPin < TOTAL_ANALOG_PINS) {
		if (value == 0) {
			analogInputsToReport = analogInputsToReport & ~(1 << analogPin);
		}
		else {
			analogInputsToReport = analogInputsToReport | (1 << analogPin);
			// prevent during system reset or all analog pin values will be reported
			// which may report noise for unconnected analog pins
			if (!isResetting) {
				// Send pin value immediately. This is helpful when connected via
				// ethernet, wi-fi or bluetooth so pin states can be known upon
				// reconnecting.
				Firmata.sendAnalog(analogPin, analogRead(analogPin));
			}
		}
	}
	// TODO: save status to EEPROM here, if changed
}

void FirmataCallbacks::handleReportDigital(byte port, int value)
{
	if (port < TOTAL_PORTS) {
		reportPINs[port] = (byte)value;
		// Send port value immediately. This is helpful when connected via
		// ethernet, wi-fi or bluetooth so pin states can be known upon
		// reconnecting.
		if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
	}
	// do not disable analog reporting on these 8 pins, to allow some
	// pins used for digital, others analog.  Instead, allow both types
	// of reporting to be enabled, but check if the pin is configured
	// as analog when sampling the analog inputs.  Likewise, while
	// scanning digital pins, portConfigInputs will mask off values from any
	// pins configured as analog
}

} //end namespace