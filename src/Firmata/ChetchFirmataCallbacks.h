#include <Arduino.h>
#include <Firmata.h>

namespace Chetch{

class FirmataCallbacks{
	
    public:
		static void init(FirmataCallbacks *fcb, const char* boardID, int options = 0);
		static void stringCallback(char *s);
		static void sysexCallback(byte command, byte argc, byte *argv);
		static void setPinModeCallback(byte pin, int mode);
		static void setPinValueCallback(byte pin, int value);
		static void analogWriteCallback(byte pin, int value);
		static void digitalWriteCallback(byte port, int value);
		static void reportAnalogCallback(byte analogPin, int value);
		static void reportDigitalCallback(byte port, int value);

		static FirmataCallbacks* FCB;


		/*
		* instance props and methods
		*/

		char* boardID; //used to identify this board to other boards/controllers

		/* analog inputs */
		int analogInputsToReport = 0;		// bitwise array to store pin reporting

		/* digital input ports */
		byte reportPINs[TOTAL_PORTS];       // 1 = report this port, 0 = silence
		byte previousPINs[TOTAL_PORTS];     // previous 8 bits sent

		/* pins configuration */
		byte portConfigInputs[TOTAL_PORTS]; // each bit: 1 = pin in INPUT, 0 = anything else

		/* timer variables */
		unsigned long currentMillis;        // store the current value from millis()
		unsigned long previousMillis;       // for comparison with currentMillis
		unsigned int samplingInterval = 19; // how often to run the main loop (in ms)

		boolean isResetting = false;

		virtual void handleString(char *s);
		virtual void handleSysex(byte command, byte argc, byte *argv);
		virtual void handleSetPinMode(byte pin, int mode);
		virtual void handleSetPinValue(byte pin, int value);
		virtual void handleAnalogWrite(byte pin, int value);
		virtual void handleDigitalWrite(byte port, int value);
		virtual void outputPort(byte portNumber, byte portValue, byte forceSend);
		virtual void handleReportAnalog(byte analogPin, int value);
		virtual void handleReportDigital(byte port, int value);
};

} //end namespace