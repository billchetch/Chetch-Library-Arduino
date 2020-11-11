#include "ChetchUtils.h"
#include "ChetchADMMessage.h"

namespace Chetch{

   ADMMessage::ErrorCode ADMMessage::error = ADMMessage::NO_ERROR;

  /*
   * Assumes that incoming data is a byte array message
   */
  ADMMessage *ADMMessage::deserialize(char *s){

    ADMMessage::error = ADMMessage::NO_ERROR;

    int byteCount = strlen(s) - 1; //because the last byte is the checksum
    
    if(byteCount < 6){
      ADMMessage::error = ADMMessage::ERROR_BADLY_FORMED;
      return NULL;
    }

    //put bytes in to an array and replace any 'zero byte' bytes with literal 0
    //count the number of arguments as well
    byte zeroByte = (byte)s[0];
    byte checkbyte = (byte)s[byteCount]; //checksum of all bytes other than the zero byte and the checksum byte
    if(checkbyte == zeroByte)checkbyte = 0;

    byte checksum = 0;
    byte *bytes = new byte[byteCount - 1]; //we reduce by 1 as we don't need zero mask'
    int argCount = 0;
    int argByteCountIdx = 6; //the byte index at which to find arguments (we start at 6 and not 5 because the first byte is the zero byte mask)
    for(int i = 1; i < byteCount; i++){
      byte b = (byte)s[i];
      if(b == zeroByte)b = 0; //convert the zero byte mask to actual zero
      bytes[i - 1] = b;
      if(i == argByteCountIdx){
        argCount++;
        argByteCountIdx += bytes[i - 1] + 1; //the position of the next argument
      }
      checksum += b;
    }    
    
    //check sum error
    if(checksum != checkbyte){
        ADMMessage::error = ADMMessage::ERROR_CHECKSUM;
        return NULL;
    }

    ADMMessage *message = new ADMMessage(bytes, byteCount - 1, argCount); //byte count reduces by 1 we have excluded 0 byte
    delete[] bytes;
    
    return message;
  };

 
  /*
   * Helper functions for processing byte arrays
   */
  long ADMMessage::bytesToLong(byte *bytes, int numberOfBytes, bool littleEndian){
    //TODO:: allow for littleEndian to be false (i.e. big endian)
    long retVal = 0L;
    for(int i = 0; i < numberOfBytes; i++){
      retVal += (long)bytes[i] << (8*i);
    }
    return retVal;
  }

  unsigned long ADMMessage::bytesToULong(byte *bytes, int numberOfBytes, bool littleEndian){
    //TODO:: allow for littleEndian to be false (i.e. big endian)
    unsigned long retVal = 0L;
    for(int i = 0; i < numberOfBytes; i++){
      retVal += (unsigned long)bytes[i] << (8*i);
    }
    return retVal;
  }

  int ADMMessage::bytesToInt(byte *bytes, int numberOfBytes, bool littleEndian){
    return (int)ADMMessage::bytesToLong(bytes, numberOfBytes, littleEndian); 
  }


  /*
   * Constructor from byte array
   */
  ADMMessage::ADMMessage(byte* bytes, int byteCount, byte argCount){
    type = bytes[0];
    tag = bytes[1];
    target = bytes[2];
    command = bytes[3];
    sender = bytes[4];

    //now parse out the arguments
    argumentCount = argCount;
    maxArguments = argumentCount;
    if(argumentCount > 0)
    {
        int i = 5;
        arguments = new byte*[argumentCount];
        argumentLengths = new byte[argumentCount];
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
  };

  ADMMessage::ADMMessage(byte messageType, byte messageTag, byte messageTarget, byte messageCommand){
    type = messageType;
    tag = messageTag;
    target = messageTarget;
    command = messageCommand;

    newID();
  };
  
  ADMMessage::ADMMessage(int maxArgs){
	if(maxArgs > 0){
        maxArguments = maxArgs;
		arguments = new byte*[maxArguments];
        argumentLengths = new byte[maxArguments];
        for(int i = 0; i < maxArgs; i++){
            argumentLengths[i] = 0;
        }
	}
    newID();
  };

  ADMMessage::~ADMMessage() {
    if(maxArguments > 0){
      for(int i = 0; i < argumentCount; i++){
        delete[] arguments[i];
      }
      delete[] arguments;
      delete[] argumentLengths;
    }
    
    /*if(maxValues > 0){
      for(int i = 0; i < 2*valuesCount; i++){
		delete[] values[i];
      }
      delete[] values;
    }*/
  }

   void ADMMessage::newID(){
	id = millis();
   }

   /*
   * Arguments: positional byte arrays
   */

   byte ADMMessage::bytesRequired(){
    byte sz = 0;
    for(int i = 0; i < argumentCount; i++){
        sz += 1 + argumentLengths[i];
    }
    return sz + 5 + 2; //Add 5 for Type, Tag, Target, Command, Sender, Add 2 for zeroByte and checksum
   }

   byte ADMMessage::getArgumentCount(){
	return argumentCount;
   }

  long ADMMessage::argumentAsLong(byte argIdx){
    if(argIdx >=0 && argIdx < argumentCount){
      return ADMMessage::bytesToLong(arguments[argIdx], argumentLengths[argIdx], littleEndian);
    } else {
      return 0;
    }
  }

  unsigned long ADMMessage::argumentAsULong(byte argIdx){
    if(argIdx >=0 && argIdx < argumentCount){
      return ADMMessage::bytesToULong(arguments[argIdx], argumentLengths[argIdx], littleEndian);
    } else {
      return 0;
    }
  }

  int ADMMessage::argumentAsInt(byte argIdx){
    return (int)argumentAsLong(argIdx);
  }

  char *ADMMessage::argumentAsCharArray(byte argIdx, char *s){
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

  byte ADMMessage::argumentAsByte(byte argIdx){
    if(argIdx >=0 && argIdx < argumentCount && argumentLengths[argIdx] == 1){
      return (byte)arguments[argIdx][0];
    } else {
      return 0;
    }
  }

  void ADMMessage::addBytes(byte *bytev, byte bytec){
    if(argumentCount == maxArguments)return;
    
    arguments[argumentCount] = new byte[bytec];
    argumentLengths[argumentCount] = bytec;
    for(int i = 0; i < bytec; i++){
      arguments[argumentCount][i] = bytev[i];
    }
    argumentCount++;
  }

  void ADMMessage::addByte(byte argv){
    addBytes((byte*)&argv, sizeof(argv));
  }

  void ADMMessage::addBool(bool argv){
    addByte((byte)argv);
  }
  
  void ADMMessage::addInt(int argv){
    addBytes((byte*)&argv, sizeof(argv));
  }

  void ADMMessage::addLong(long argv){
    addBytes((byte*)&argv, sizeof(argv));
  }

  void ADMMessage::addString(const char *argv){
    addBytes((byte*)argv, strlen(argv));
  }
  
  void ADMMessage::addFloat(float argv){
    addBytes((byte*)&argv, sizeof(argv));
  }
  
  void ADMMessage::serialize(char* bytes){
    bytes[1] = type;
    bytes[2] = tag;
    bytes[3] = target;
    bytes[4] = command;
    bytes[5] = sender;

    byte idx = 6;
    for(int i = 0; i < argumentCount; i++){
        bytes[idx] = argumentLengths[i];
        idx++;
        for(int j = 0; j < argumentLengths[i]; j++){
            byte b = arguments[i][j];
            bytes[idx] = b;
            idx++;
        }
    }

    byte checksum = 0;
    for(int i = 1; i < idx; i++){
        byte b = bytes[i];
        checksum += b;
    }
    
    //look for zeroByte
    byte zeroByte = 1;
    while(zeroByte <= 255){
        bool useable = true;
        for(int i = 1; i < idx; i++){
            if(bytes[i] == zeroByte){
                useable = false;
                break;
            }
        }
        if(useable)break;
        zeroByte++;
    }
        
    //now replace all zeros with zerobyte
    for(int i = 1; i < idx; i++){
        if(bytes[i] == 0)bytes[i] = zeroByte;
    }
    
    bytes[0] = zeroByte;
    bytes[idx] = checksum;
  }
  
  ADMMessage::CommandType ADMMessage::commandType(){
        return (ADMMessage::CommandType)(command & 0xF);
  }

  byte ADMMessage::commandIndex(){
        return (byte)(command >> 4);
  }

} //end of namespace