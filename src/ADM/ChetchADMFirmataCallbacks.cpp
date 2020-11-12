#include "ChetchADMFirmataCallbacks.h"
#include "ChetchUtils.h"
#include <MemoryFree.h>

#if defined(ARDUINO_AVR_UNO)
//Uno specific code
#define BOARD "UNO"
#define LITTLE_ENDIAN true
#define MAX_MESSAGE_SIZE 128
#define IR_TRANSMITTER_PIN 13
#elif defined(ARDUINO_AVR_MEGA2560)
//Mega 2560 specific code
#define BOARD "MEGA2560"
#define LITTLE_ENDIAN true
#define MAX_MESSAGE_SIZE 512
#define IR_TRANSMITTER_PIN 13
#elif defined(ARDUINO_SAM_DUE)
#define BOARD "SAM_DUE"
#else
#error Unsupported hardware
#endif


namespace Chetch{
	ArduinoDeviceManager ADMFirmataCallbacks::ADM;

	//Override this to perform initialisation based on computer-side connection to the board
	//this is NOT the same as the board code constructing an instance (or calling begin) as these
	//may already have happend then the computer disconnects and then connects again
	void ADMFirmataCallbacks::initialise() {
		ADM.initialise(this);
		handleSystemReset();
	}

	void ADMFirmataCallbacks::sendMessage(ADMMessage *message) {
		//IMPORTANT!!!: the serial connection can return messy data if this function is called too rapidly
		//might be worth putting in a small 'delay' here... Having said that this problem has so far (5/5/20) only
		//emerged when the connected computer has called too frequently which in turn was solved by putting a delay
		//on the computer thread ... so at the time of writing there hasn't occurred a situation where putting a delay
		//here was required...

		message->sender = boardID;
		int length = message->bytesRequired() + 1;
		char *bytes = new char[length]; //we add 1 extra as a 0 for the end
		message->serialize(bytes);
		bytes[length - 1] = 0;
		char stBuffer[24];
		if(bytes[0] == 0){
			sprintf(stBuffer, "0 at first byte of %d", message->bytesRequired());
			Firmata.sendString(stBuffer);
		} else {
			Firmata.sendString(bytes);
		}
		delete[] bytes;
		FirmataStream->flush();
	}

	void ADMFirmataCallbacks::respond(ADMMessage *message, ADMMessage *response) {
		//this provides a hook to allow modification of response
		response->target = message->target;
		response->tag = message->tag;
		sendMessage(response);
	}

	void ADMFirmataCallbacks::configure(ADMMessage *message, ADMMessage *response) {
		//provides a hook to prepare before use
	}

	bool ADMFirmataCallbacks::handleCommand(ADMMessage *message, ADMMessage *response) {
		//a hook.. return true to send response
		return false;
	}

	void ADMFirmataCallbacks::handleMessage(ADMMessage *message) {
		
		ADMMessage *response = NULL;
		
		if (message == NULL) {
			response = new ADMMessage(1);
			response->type = (byte)Chetch::ADMMessage::TYPE_ERROR;
			response->addInt(ADMMessage::error); 
			sendMessage(response);
			delete response;
			return;
		}

		
		//we have a valid message to deal with
		ArduinoDevice *device = message->target == 0 ? NULL : ADM.getDevice(message->target);
	
		switch ((ADMMessage::MessageType)message->type) {
			case ADMMessage::TYPE_INITIALISE:
				initialise();
				response = new ADMMessage(4);
				response->type = (byte)ADMMessage::TYPE_INITIALISE_RESPONSE;
				response->addBool(LITTLE_ENDIAN);
				response->addInt(freeMemory());
				response->addByte(MAX_DEVICES);
				response->addByte(LED_BUILTIN);
				respond(message, response);
				break;
		
			case ADMMessage::TYPE_STATUS_REQUEST:
				if (message->target == 0) {
					response = new ADMMessage(4);
					response->type = (byte)ADMMessage::TYPE_STATUS_RESPONSE;
					
					//General values for the board
					response->addInt(freeMemory());
					response->addString(BOARD);
					response->addBool(ADM.isInitialised());
					response->addByte(ADM.getDeviceCount());
				}
				else if (device != NULL) {
					response = new ADMMessage(4);
					response->type = (byte)ADMMessage::TYPE_STATUS_RESPONSE;
					
					//values for device
					response->addByte(device->target);
					response->addByte(device->category);
					response->addString(device->name);

					device->handleStatusRequest(message, response);
				} 
				else {
					response = new ADMMessage(3);

					//error
					response->type = (byte)Chetch::ADMMessage::TYPE_ERROR;
					response->addInt(ADMMessage::ERROR_UNKNOWN);
					response->addByte(message->type);
					response->addByte(message->target);
				}
				respond(message, response);
				break;

			case ADMMessage::TYPE_CONFIGURE:
				if (message->target == 0) { //configure board (not yet used)
					response = new ADMMessage(4);
					response->addByte(message->target);
					configure(message, response);
				} else { //configure device
					bool initial = true;
					response = new ADMMessage(8);
					if (device == NULL) { //this is a new device
						char deviceName[DEVICE_NAME_LENGTH];
						message->argumentAsCharArray(MSG_DEVICE_NAME_INDEX, deviceName);
						device = ADM.addDevice(message->target, message->argumentAsByte(MSG_DEVICE_CATEGROY_INDEX), deviceName);
					} else { //we already have a device added
						initial = false;
					}
					response->addString(device->name);
					device->configure(initial, message, response);
				}
				response->type = (byte)ADMMessage::TYPE_CONFIGURE_RESPONSE;
				respond(message, response);
				break;

			case ADMMessage::TYPE_PING:
				response = new ADMMessage(3);
				response->type = (byte)ADMMessage::TYPE_PING_RESPONSE;
				response->addInt(freeMemory());
				response->addBool(ADM.isInitialised());
				response->addLong(millis());
				respond(message, response);
				break;

			case ADMMessage::TYPE_COMMAND:
				response = new ADMMessage(8);
				response->type = (byte)ADMMessage::TYPE_COMMAND_RESPONSE;
				response->command = message->command;
				if (message->target == 0) {
					handleCommand(message, response);
				} else if(device != NULL) {
					device->handleCommand(message, response);
				}
				respond(message, response);
				break;

			default:
				response = new ADMMessage(2);

				//error
				response->type = (byte)Chetch::ADMMessage::TYPE_ERROR;
				response->addInt(ADMMessage::ERROR_UNRECOGNISED_MESSAGE_TYPE);
				response->addByte(message->type);
				break;
		}
		if(response != NULL){
			delete response;
		}
	}

	void ADMFirmataCallbacks::handleString(char *s){
		FirmataCallbacks::handleString(s);

		ADMMessage *message = ADMMessage::deserialize(s);
		handleMessage(message);
		delete message;
 	}

	void ADMFirmataCallbacks::loop() {
		samplingEnabled = ADM.isInitialised();	

		FirmataCallbacks::loop();
	
		heartbeat();
		
		ADM.loop();
	}


	void ADMFirmataCallbacks::heartbeat(){
		int diff = currentMillis - heartbeatMillis;
		if(ADM.isInitialised()){
			if(diff <= 250){
			   digitalWrite(LED_BUILTIN, HIGH);
			} else {
			   digitalWrite(LED_BUILTIN, LOW);
			}
		} else {
			digitalWrite(LED_BUILTIN, LOW);
		}
		if(diff > 5000)heartbeatMillis = currentMillis;
	}
} //end namespace
