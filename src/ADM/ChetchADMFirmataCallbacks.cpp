#include "ChetchADMFirmataCallbacks.h"
#include "ChetchUtils.h"
#include <MemoryFree.h>

#if defined(ARDUINO_AVR_UNO)
//Uno specific code
#define BOARD "UNO"
#define LITTLE_ENDIAN true
#define MAX_MESSAGE_SIZE 256
#elif defined(ARDUINO_AVR_MEGA2560)
//Mega 2560 specific code
#define BOARD "MEGA2560"
#define LITTLE_ENDIAN true
#define MAX_MESSAGE_SIZE 512
#elif defined(ARDUINO_SAM_DUE)
#define BOARD "SAM_DUE"
#else
#error Unsupported hardware
#endif

const char BOARD_PARAM[] PROGMEM = "BD";
const char MAX_DEVICES_PARAM[] PROGMEM = "MD";
const char BOARD_ID_PARAM[] PROGMEM = "BDID";
const char LITTLE_ENDIAN_PARAM[] PROGMEM = "LE";
const char LEDBI_PARAM[] PROGMEM = "LEDBI";
const char CALLBACK_COUNT_PARAM[] PROGMEM = "CC";
const char FREE_MEMORY_PARAM[] PROGMEM = "FM";
const char DEVICE_COUNT_PARAM[] PROGMEM = "DC";
const char DEVICE_TARGET_PARAM[] PROGMEM = "DT";
const char DEVICE_CATEGORY_PARAM[] PROGMEM = "DG";
const char DEVICE_ID_PARAM[] PROGMEM = "DTD";
const char DEVICE_NAME_PARAM[] PROGMEM = "DN";
const char MILLIS_PARAM[] PROGMEM = "MS";
const char MAX_MESSAGE_SIZE_PARAM[] PROGMEM = "MM";

const char ADM_INITIALISED_MESSAGE[] PROGMEM = "ADM Initialised";
const char STATUS_OK_MESSAGE[] PROGMEM = "Status OK";
const char NO_DEVICE_MESSAGE[] PROGMEM = "No device";
const char PARSE_ERROR_MESSAGE[] PROGMEM = "Parse error";
const char BOARD_CONFIGURED_MESSAGE[] PROGMEM = "Board configured";
const char DEVICE_ALREADY_ADDED_MESSAGE[] PROGMEM = "Device already added"; 

const char *const PARAMS_TABLE[] PROGMEM = {
					BOARD_PARAM, 
					MAX_DEVICES_PARAM, 
					BOARD_ID_PARAM, 	
					LITTLE_ENDIAN_PARAM, 
					LEDBI_PARAM,
					CALLBACK_COUNT_PARAM,  	//not used
					FREE_MEMORY_PARAM, 
					DEVICE_COUNT_PARAM, 
					DEVICE_TARGET_PARAM, 
					DEVICE_CATEGORY_PARAM, 
					DEVICE_ID_PARAM, 
					DEVICE_NAME_PARAM,
					MILLIS_PARAM,
					MAX_MESSAGE_SIZE_PARAM	
					};

const char *const MESSAGES_TABLE[] PROGMEM = {
					ADM_INITIALISED_MESSAGE, 
					STATUS_OK_MESSAGE, 
					NO_DEVICE_MESSAGE,
					PARSE_ERROR_MESSAGE,
					BOARD_CONFIGURED_MESSAGE,
					DEVICE_ALREADY_ADDED_MESSAGE 
					};

namespace Chetch{
	ArduinoDeviceManager ADMFirmataCallbacks::ADM;

	//Override this to perform initialisation based on computer-side connection to the board
	//this is NOT the same as the board code constructing an instance (or calling begin) as these
	//may already have happend then the computer disconnects and then connects again
	void ADMFirmataCallbacks::initialise() {
		ADM.initialise();
		handleSystemReset();
	}

	void ADMFirmataCallbacks::sendMessage(ADMMessage *message) {
		//IMPORTANT!!!: the serial connection can return messy data if this function is called too rapidly
		//might be worth putting in a small 'delay' here... Having said that this problem has so far (5/5/20) only
		//emerged when the connected computer has called too frequently which in turn was solved by putting a delay
		//on the computer thread ... so at the time of writing there hasn't occurred a situation where putting a delay
		//here was required...
		char *s = new char[MAX_MESSAGE_SIZE];
		message->serialize(s);
		Firmata.sendString(s);
		delete s;
	}

	void ADMFirmataCallbacks::respond(ADMMessage *message, ADMMessage *response) {
		//this provides a hook to allow modification of response
		response->target = message->target;
		sendMessage(response);
	}

	void ADMFirmataCallbacks::configureDevice(bool initial, ArduinoDevice *device, ADMMessage *message, ADMMessage *response) {
		//provides a hook to prepare before use
	}

	void ADMFirmataCallbacks::handleMessage(ADMMessage *message) {
		
		ADMMessage *response = NULL;
		char stBuffer[32];	//string table buffer				
		
		if (message == NULL) {
			response = new ADMMessage(1);
			response->type = Chetch::ADMMessage::TYPE_ERROR;
			response->setValue(Utils::getStringFromProgmem(stBuffer, 3, PARAMS_TABLE));
			sendMessage(response);
			delete response;
			return;
		}
		
		//we have a valid message to deal with
		ArduinoDevice *device = message->target == 0 ? NULL : ADM.getDevice(message->target);
	
		switch (message->type) {
			case ADMMessage::TYPE_INITIALISE:
				initialise();
				response = new ADMMessage(4);
				response->type = ADMMessage::TYPE_INITIALISE_RESPONSE;
				response->addInt(Utils::getStringFromProgmem(stBuffer, 6, PARAMS_TABLE), freeMemory());
				response->addInt(Utils::getStringFromProgmem(stBuffer, 7, PARAMS_TABLE), ADM.getDeviceCount());
				response->setValue(Utils::getStringFromProgmem(stBuffer, 0, MESSAGES_TABLE));
				respond(message, response);
				break;
		
			case ADMMessage::TYPE_STATUS_REQUEST:
				if (message->target == 0) {
					response = new ADMMessage(8);
					response->type = ADMMessage::TYPE_STATUS_RESPONSE;
					
					//General values for the board
					response->addValue(Utils::getStringFromProgmem(stBuffer, 0, PARAMS_TABLE), BOARD, false);
					if(this->boardID != NULL){
						response->addValue(Utils::getStringFromProgmem(stBuffer, 2, PARAMS_TABLE), this->boardID, true);
					}
					response->addByte(Utils::getStringFromProgmem(stBuffer, 1, PARAMS_TABLE), MAX_DEVICES);
					response->addBool(Utils::getStringFromProgmem(stBuffer, 3, PARAMS_TABLE), LITTLE_ENDIAN);
					response->addByte(Utils::getStringFromProgmem(stBuffer, 4, PARAMS_TABLE), LED_BUILTIN);
					response->addInt(Utils::getStringFromProgmem(stBuffer, 6, PARAMS_TABLE), freeMemory());
					response->addByte(Utils::getStringFromProgmem(stBuffer, 7, PARAMS_TABLE), ADM.getDeviceCount());
					response->addInt(Utils::getStringFromProgmem(stBuffer, 13, PARAMS_TABLE), MAX_MESSAGE_SIZE);
					response->setValue(Utils::getStringFromProgmem(stBuffer, 1, MESSAGES_TABLE));
				}
				else if (device != NULL) {
					response = new ADMMessage(4);
					response->type = ADMMessage::TYPE_STATUS_RESPONSE;
					
					//values for device
					response->addByte(Utils::getStringFromProgmem(stBuffer, 8, PARAMS_TABLE), device->target);
					response->addByte(Utils::getStringFromProgmem(stBuffer, 9, PARAMS_TABLE), device->category);
					response->addValue(Utils::getStringFromProgmem(stBuffer, 10, PARAMS_TABLE), device->id, false);
					response->addValue(Utils::getStringFromProgmem(stBuffer, 11, PARAMS_TABLE), device->name, false);
				}
				else {
					response = new ADMMessage(1);

					//error
					response->type = Chetch::ADMMessage::TYPE_ERROR;
					response->setValue(Utils::getStringFromProgmem(stBuffer, 2, MESSAGES_TABLE));
				}
				respond(message, response);
				break;

			case ADMMessage::TYPE_CONFIGURE:
				if (message->target == 0) { //configure board
					response = new ADMMessage(5);
					response->setValue(Utils::getStringFromProgmem(stBuffer, 4, MESSAGES_TABLE));
				} else { //configure device
					bool initial = true;
					if (device == NULL) { //this is a new device
						response = new ADMMessage(8);
						char deviceId[16];
						char deviceName[32];
						message->argumentAsCharArray(1, deviceId);
						message->argumentAsCharArray(2, deviceName);
						device = ADM.addDevice(message->target, message->argumentAsByte(0), deviceId, deviceName);
						response->addValue("DID", device->id, false);
						response->addValue("DN", device->name, false);
					} else { //we already have a device added
						response = new ADMMessage(5);
						initial = false;
						response->setValue(Utils::getStringFromProgmem(stBuffer, 5, MESSAGES_TABLE));
					}
					configureDevice(initial, device, message, response);
				}
				response->type = ADMMessage::TYPE_CONFIGURE_RESPONSE;
				respond(message, response);
				break;

			case ADMMessage::TYPE_PING:
				response = new ADMMessage(2);
				response->type = ADMMessage::TYPE_PING_RESPONSE;
				response->addInt(Utils::getStringFromProgmem(stBuffer, 6, PARAMS_TABLE), freeMemory());
				response->addInt(Utils::getStringFromProgmem(stBuffer, 12, PARAMS_TABLE), millis());
				respond(message, response);
				break;

			default:
				//do nothing as it can drop through to be handled by derived classes 
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

} //end namespace
