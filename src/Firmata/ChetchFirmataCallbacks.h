#ifndef CHETCH_FIRMATA_CALLBACKS_H
#define CHETCH_FIRMATA_CALLBACKS_H

#include <Arduino.h>
#include <Firmata.h>

namespace Chetch{

class FirmataCallbacks{
	
    public:
		static void init(FirmataCallbacks *fcb, byte boardID, long baudRate, int options = 0);
		static void stringCallback(char *s);
		static void sysexCallback(byte command, byte argc, byte *argv);
		static void setPinModeCallback(byte pin, int mode);
		static void setPinValueCallback(byte pin, int value);
		static void analogWriteCallback(byte pin, int value);
		static void digitalWriteCallback(byte port, int value);
		static void reportAnalogCallback(byte analogPin, int value);
		static void reportDigitalCallback(byte port, int value);
		static void systemResetCallback();

		static FirmataCallbacks* FCB;


		/*
		* instance props and methods
		*/

		Stream *FirmataStream;

		byte boardID;						//used to identify this board to other boards/controllers

		/* analog inputs */
		int analogInputsToReport = 0;		// bitwise array to store pin reporting

		/* digital input ports */
		byte reportPINs[TOTAL_PORTS];       // 1 = report this port, 0 = silence
		byte previousPINs[TOTAL_PORTS];     // previous 8 bits sent

		/* pins configuration */
		byte portConfigInputs[TOTAL_PORTS]; // each bit: 1 = pin in INPUT, 0 = anything else

		/* timer variables */
		unsigned long currentMillis;        // store the current value from millis()
		unsigned int analogSamplingInterval = 19; // how often to run the analog pin loop (in ms)
		unsigned long analogPreviousMillis = 0;
		unsigned int digitalSamplingInterval = 20; //how often to run the check digital pins code (the reason for the interval is that the process of sampling sends data and so the throttling is to prevent serial bufffer overloads)
		unsigned long digitalPreviousMillis = 0;
		boolean samplingEnabled = false;

		boolean isResetting = false;

		virtual void handleString(char *s);
		virtual void handleSysex(byte command, byte argc, byte *argv);
		virtual void handleSetPinMode(byte pin, int mode);
		virtual void handleSetPinValue(byte pin, int value);
		virtual void handleAnalogWrite(byte pin, int value);
		virtual void handleDigitalWrite(byte port, int value);
		virtual void handleReportAnalog(byte analogPin, int value);
		virtual void handleReportDigital(byte port, int value);
		virtual void handleSystemReset();

		virtual void begin(byte boardID, long baudRate, int options = 0);
		virtual void loop();
		void processSerialInput();
		void outputPort(byte portNumber, byte portValue, byte forceSend);
		void checkDigitalInputs(void);
};

} //end namespace
#endif