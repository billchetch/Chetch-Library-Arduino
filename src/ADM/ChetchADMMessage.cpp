#include "ChetchUtils.h"
#include "ChetchADMMessage.h"

namespace Chetch{
  /*
   * Assumes that incoming data is a byte array message
   */
  ADMMessage *ADMMessage::deserialize(char *s){
    int byteCount = strlen(s);
    if(byteCount < 4){
      return NULL;
    }

    //put bytes in to an array and replace any 'zero byte' bytes with literal 0
    //count the number of arguments as well
    byte *bytes = new byte[byteCount];
    bytes[0] = (byte)s[0]; //this is the 'zero byte mask' the value we use to encode 0 (because 0 byte will be interpreted as end of string)
    int argCount = 0;
    int argByteCountIdx = 5; //the byte index at which to find arguments
    for(int i = 1; i < byteCount; i++){
      byte b = (byte)s[i];
      if(b == bytes[0])b = 0; //convert the zero byte mask to actual zero
      bytes[i] = b;
      if(i == argByteCountIdx){
        argCount++;
        argByteCountIdx += bytes[i] + 1;
      }
    }    
    
    ADMMessage *message = new ADMMessage(bytes, byteCount, argCount);
    delete[] bytes;
    
    return message;
  };

 
  /*
   * Helper functions for processing byte arrays
   */
  long ADMMessage::bytesToLong(byte *bytes, int numberOfBytes, bool littleEndian = true){
    //TODO:: allow for littleEndian to be false (i.e. big endian)
    long retVal = 0L;
    for(int i = 0; i < numberOfBytes; i++){
      retVal += (long)bytes[i] << (8*i);
    }
    return retVal;
  }

  unsigned long ADMMessage::bytesToULong(byte *bytes, int numberOfBytes, bool littleEndian = true){
    //TODO:: allow for littleEndian to be false (i.e. big endian)
    unsigned long retVal = 0L;
    for(int i = 0; i < numberOfBytes; i++){
      retVal += (unsigned long)bytes[i] << (8*i);
    }
    return retVal;
  }

  int ADMMessage::bytesToInt(byte *bytes, int numberOfBytes, bool littleEndian = true){
    return (int)ADMMessage::bytesToLong(bytes, numberOfBytes, littleEndian); 
  }

  /*
   * Constructor from byte array
   */
  ADMMessage::ADMMessage(byte* bytes, int byteCount, int argCount){
    type = bytes[1]; //we start from index 1 because the 0 index is the zero byte mask
    tag = bytes[2];
    target = bytes[3];
    command = bytes[4];

    //now parse out the arguments
    argumentCount = argCount;
    if(argumentCount > 0)
    {
        int i = 5;
        arguments = new byte*[argumentCount];
        argumentLengths = new int[argumentCount];
        int ac = 0;
        while(i < byteCount){
          int bc = (int)bytes[i]; //the number of bytes in the argument
          argumentLengths[ac] = bc;
          arguments[ac] = new byte[bc];
          for(int j = 0; j < bc; j++){
            arguments[ac][j] = bytes[i + j + 1];
          }
          ac++;
          i = i + bc + 1;
        }
     }
  }

  ADMMessage::ADMMessage(byte messageType = 0, byte messageTag = 0, byte messageTarget = 0, byte messageCommand = 0){
    type = messageType;
    tag = messageTag;
    target = messageTarget;
    command = messageCommand;

    newID();
  }
  
  ADMMessage::ADMMessage(int maxVals){
	if(maxVals > 0){
		maxValues = maxVals + 4; //add an extra 4 for the Type,Tag,Target,Command params
		values = new char*[maxValues*2];
	}
        newID();
  };

  ADMMessage::~ADMMessage() {
    if(argumentCount > 0){
      for(int i = 0; i < argumentCount; i++){
        delete[] arguments[i];
      }
      delete[] arguments;
      delete[] argumentLengths;
    }
    
    if(maxValues > 0){
      for(int i = 0; i < 2*valuesCount; i++){
	delete[] values[i];
      }
      delete[] values;
    }
  }

   void ADMMessage::newID(){
	id = millis();
   }

   /*
   * Arguments: positional byte arrays
   */
   int ADMMessage::getArgumentCount(){
	return argumentCount;
   }

  long ADMMessage::argumentAsLong(int argIdx){
    if(argIdx >=0 && argIdx < argumentCount){
      return ADMMessage::bytesToLong(arguments[argIdx], argumentLengths[argIdx], littleEndian);
    } else {
      return 0;
    }
  }

  unsigned long ADMMessage::argumentAsULong(int argIdx){
    if(argIdx >=0 && argIdx < argumentCount){
      return ADMMessage::bytesToULong(arguments[argIdx], argumentLengths[argIdx], littleEndian);
    } else {
      return 0;
    }
  }

  int ADMMessage::argumentAsInt(int argIdx){
    return (int)argumentAsLong(argIdx);
  }

  char *ADMMessage::argumentAsCharArray(int argIdx, char *s){
    if(argIdx >=0 && argIdx < argumentCount){
      for(int i = 0; i < argumentLengths[argIdx]; i++){
        s[i] = arguments[argIdx][i];
      }
      s[argumentLengths[argIdx]] = 0;
      return s;
    } else {
      return NULL;
    }
  }

  byte ADMMessage::argumentAsByte(int argIdx){
    if(argIdx >=0 && argIdx < argumentCount && argumentLengths[argIdx] == 1){
      return (byte)arguments[argIdx][0];
    } else {
      return 0;
    }
  }
  
  /*
   * Values: key referenced strings
   */

  char *ADMMessage::getValue(char *key){
    return Utils::getValue(key, values, valuesCount);
  }
  
  void ADMMessage::addValue(char *key, char *value, boolean allowNullOrEmpty){
    if(valuesCount == maxValues){
      return;
    }
    if(getValue(key) != NULL){
      return;
    }
    if(!allowNullOrEmpty && (value == NULL || strlen(value) == 0)){
      return;
    }

    //copy to heap (remember to delete in destructor)
    char *k = new char[strlen(key) + 1];
    for(int i = 0; i < strlen(key); i++){
      k[i] = key[i];
    }
    k[strlen(key)] = 0;
    char *v = new char[strlen(value) + 1];
    for(int i = 0; i < strlen(value); i++){
      v[i] = value[i];
    }
    v[strlen(value)] = 0;
    
    values[2*valuesCount] = k; //ey;
    values[2*valuesCount+ 1] = v; //alue;
    valuesCount++;
  }

  void ADMMessage::addInt(char *key, int value){
    static char c[6];
    sprintf(c, "%d", value);
    addValue(key, c, false);
  }

  void ADMMessage::addLong(char *key, unsigned long value){
    static char c[16];
    sprintf(c, "%lu", value);
    addValue(key, c, false);
  }

  void ADMMessage::addBool(char *key, bool value){
    addByte(key, value ? 1 : 0);
  }

  void ADMMessage::addByte(char *key, byte value){
    static char c[3];
    sprintf(c, "%d", value);
    addValue(key, c, false);
  }

  void ADMMessage::setValue(char *value){
    addValue("Value", value, true);  
  }
  
  /*char *ADMMessage::serialize(boolean encodeUrl = true){
    addByte("Type", type);
    addByte("Tag", tag);
    addByte("Target", target);
    addByte("Command", command);
    
    static char str[512]; //512];
    str[0] = 0;
    return Utils::buildQueryString(str, values, valuesCount, encodeUrl);
  }*/

  void ADMMessage::serialize(char* s, boolean encodeUrl = true){
    addByte("Type", type);
    addByte("Tag", tag);
    addByte("Target", target);
    addByte("Command", command);
    
    s[0] = 0;
    Utils::buildQueryString(s, values, valuesCount, encodeUrl);
  }
  
} //end of namespace