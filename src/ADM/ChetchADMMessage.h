#ifndef CHETCH_ADM_ADMMESSAGE_H
#define CHETCH_ADM_ADMMESSAGE_H

#include <Arduino.h>

namespace Chetch{
  class ADMMessage{
    public:
      static const byte TYPE_INFO = 3;
      static const byte TYPE_WARNING = 4;
      static const byte TYPE_ERROR = 5;
      static const byte TYPE_PING = 6;
      static const byte TYPE_PING_RESPONSE = 7;
      static const byte TYPE_STATUS_REQUEST = 8;
      static const byte TYPE_STATUS_RESPONSE = 9;
      static const byte TYPE_COMMAND = 10;
      static const byte TYPE_ERROR_TEST = 11;
      static const byte TYPE_ECHO = 12;
      static const byte TYPE_ECHO_RESPONSE = 13;
      static const byte TYPE_CONFIGURE = 14;
      static const byte TYPE_CONFIGURE_RESPONSE = 15;
      static const byte TYPE_RESET = 16;
      static const byte TYPE_INITIALISE = 17;
      static const byte TYPE_DATA = 18; 
      static const byte TYPE_INITIALISE_RESPONSE = 28;
	  
      static const byte COMMAND_TYPE_SEND = 1;
      static const byte COMMAND_TYPE_RESET = 2;
      static const byte COMMAND_TYPE_INITIALISE = 3;
      static const byte COMMAND_TYPE_RECEIVE = 4;
      static const byte COMMAND_TYPE_START = 5;
      static const byte COMMAND_TYPE_STOP = 6;
      static const byte COMMAND_TYPE_OPEN = 7;
      static const byte COMMAND_TYPE_CLOSE = 8;
      static const byte COMMAND_TYPE_SAVE = 10;
      static const byte COMMAND_TYPE_READ = 11;
     

    private:
      /*
       * Arguments are byte arrays and currently used for incoming messages (because of a Firmata limig) 
       * where the position of the bytes are assumed by whatever inteprets the message
       */
      int argumentCount = 0;
      byte **arguments;
      int *argumentLengths;
      bool littleEndian = true;

      char **values;
      int maxValues = 0;
      int valuesCount = 0;
    
    public:  
      unsigned long id = 0; 
      byte type = 0; //should take messsage type value
      byte tag = 0; //tagging data sent from computer ... can be re-used to send back to make comms linked
      byte target = 0; //used to select a 'device'
      byte command = 0; //should take a command Type value
    
      static ADMMessage *deserialize(char *s);
      static long bytesToLong(byte *bytes, int numberOfBytes, bool littleEndian = true);
      static unsigned long bytesToULong(byte *bytes, int numberOfBytes, bool littleEndian = true);
      static int bytesToInt(byte *bytes, int numberOfBytes, bool littleEndian = true);
      
      ADMMessage(byte *bytes, int byteCount, int argCount);
      ADMMessage(byte messageType = 0, byte messageTag = 0, byte messageTarget = 0, byte messageCommand = 0);
      ADMMessage(int maxVals);
      ~ADMMessage();

      /*
       * Arguments are byte arrays and currently used for incoming messages (because of a Firmata limig) 
       * where the position of the bytes are assumed by whatever inteprets the message
       */
      int getArgumentCount();
      long argumentAsLong(int argIdx);
      unsigned long argumentAsULong(int argIdx);
      int argumentAsInt(int argIdx);
      char *argumentAsCharArray(int argIdx, char *s);
      byte argumentAsByte(int argIdx);

      /*
       * Values are char arrays with a corresponding 'key' and thereofre not positional.  They
       * are currently used for outgoing messages.
       */
      char *getValue(const char *key); 
      void addValue(const char *key, const char *value, boolean allowNullOrEmpty);
      void addByte(const char *key, byte value);	
      void addLong(const char *key, unsigned long value);
      void addInt(const char *key, int value);
      void addBool(const char *key, bool value);
	  void addFloat(const char *key, float value, int precision = 1, int width = 4);
	  void addDouble(const char *key, double value, int precision = 1, int width = 4);

      void setValue(const char *value); //adds the 'Value' value with the passed string
      
      void serialize(char *s, boolean encodeUrl = true);
      //char *serialize(boolean encodeUrl = true);

    private:
      void newID();
  };
}  //end namepace

#endif