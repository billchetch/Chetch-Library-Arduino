#ifndef CHETCH_MESSAGE_FRAME_H
#define CHETCH_MESSAGE_FRAME_H

#include <Arduino.h>

namespace Chetch{
	
class MessageFrame{

  public:
      enum FrameSchema{
        SMALL_NO_CHECKSUM = 1,      //FrameSchema = 1 byte, Encoding = 1 byte, Payload size = 1 byte, Payload = max 255 bytes
        SMALL_SIMPLE_CHECKSUM,      //FrameSchema = 1 byte, Encoding = 1 byte, Payload size = 1 byte, Payload = max 255 bytes, Checksum = 1 byte
        MEDIUM_NO_CHECKSUM,         //FrameSchema = 1 byte, Encoding = 1 byte, Payload size = 2 bytes, Payload = max 32K bytes
        MEDIUM_SIMPLE_CHECKSUM      //FrameSchema = 1 byte, Encoding = 1 byte, Payload size = 2 bytes, Payload = max 32K bytes, Checksum = 4 bytes
      };
      
      enum MessageEncoding{
        XML = 1,
        QUERY_STRING,
        POSITONAL,
        BYTES_ARRAY,
        JSON
      };

    class Dimensions{
      public:
        byte schema;
        byte encoding;
        byte payloadSize = 0;
        int payload = 0;
        byte checksum;

        Dimensions(FrameSchema frameSchema){
          schema = 1;
          encoding = 1;
          switch(frameSchema){
            case SMALL_NO_CHECKSUM:
              payloadSize = 1;
              checksum = 0;
              break;

            case SMALL_SIMPLE_CHECKSUM:
              payloadSize = 1;
              checksum = 1;
              break;

            case MEDIUM_NO_CHECKSUM:
              payloadSize = 2;
              checksum = 0;
              break;

            case MEDIUM_SIMPLE_CHECKSUM:
              payloadSize = 2;
              checksum = 1;
              break;
          }
        }

        int getSchemaIndex(){ return 0; }
        int getEncodingIndex(){ return getSchemaIndex() + schema; }
        int getPayloadSizeIndex() { return getEncodingIndex() + encoding; }
        int getPayloadIndex() { return getPayloadSizeIndex() + payloadSize; }
        int getChecksumIndex(){ return payload > 0 ? getPayloadIndex() + payload : -1; }
        int getSize(){ return payload > 0 && payloadSize > 0 ? getChecksumIndex() + checksum : -1; }
    };
    
    static long bytesToLong(byte *bytes, int offset, int numberOfBytes){
      long retVal = 0L;
      for(int i = offset; i < offset + numberOfBytes; i++){
        retVal += (long)bytes[i] << (8*(i - offset));
      }
      return retVal;
    } 

    static int bytesToInt(byte *bytes, int offset, int numberOfBytes){
        return (int)bytesToLong(bytes, offset, numberOfBytes); 
    }

    static bool isValidSchema(byte b){
        return b > 0 && b < 5;
    }

    static bool isValidEncoding(byte b){
      return b > 0 && b < 6;
    }

    FrameSchema schema;
    MessageEncoding encoding;
    Dimensions *dimensions = NULL;
    byte *header = NULL;
    byte *payload = NULL;
    byte *checksum = NULL;

    int readPosition = 0;
    byte *bytes;
    bool readComplete = false;

    MessageFrame(){
      
    }
    
    MessageFrame(FrameSchema schema, MessageEncoding encoding, byte *payload, int payloadSize){
      this->schema = schema;
      this->encoding = encoding;
      dimensions = new Dimensions(schema);
      dimensions->payload = payloadSize;
      this->payload = new byte[payloadSize];
      for(int i = 0; i < payloadSize; i++){
        this->payload[i] = payload[i];
      }
    }

    MessageFrame(Stream *stream){
      read(stream);
    }

    ~MessageFrame(){
      reset();
    }
    
    bool read(Stream *stream){
      int offset = 0;
      while(!readComplete && stream->available()){
        byte b = stream->read();
        if(readPosition == 0){
          if(!isValidSchema(b))return true;
          dimensions = new Dimensions(b); //first byte tells you the schema which means we can know how many bytes for everything except the payload
          header = new byte[dimensions->getPayloadIndex()];
          bytes = header;
          offset = readPosition;
        } else if(readPosition == dimensions->getPayloadIndex()) {
          stream->print("Payload size index: ");
          stream->println(dimensions->getPayloadSizeIndex());
          stream->print("Payload size dimension: ");
          stream->println(dimensions->payloadSize);

          dimensions->payload = bytesToInt(header, dimensions->getPayloadSizeIndex(), dimensions->payloadSize);
          stream->print("Payload size: ");
          stream->println(dimensions->payload);
          payload = new byte[dimensions->payload];
          bytes = payload;
          offset = readPosition;
        } else if(dimensions->checksum == 0 && readPosition == dimensions->getChecksumIndex() - 1){
          stream->println("Finished reading payload .. no checksum");
        } else if(dimensions->checksum > 0 && readPosition == dimensions->getChecksumIndex()){
          stream->print("Finished reading payload .. now read checksum of ");
          stream->print(dimensions->checksum);
          stream->println(" bytes");
          checksum = new byte[dimensions->checksum];
          bytes = checksum;
          offset = readPosition;
        } 
        
        if(dimensions != NULL && dimensions->payload > 0 && readPosition == dimensions->getSize() - 1){
          stream->println("Finished reading frame");
          readComplete = true; 
        }

        stream->print(" >>>>>> read position / offset / idx: ");
        stream->print(readPosition);
        stream->print(" / ");
        stream->print(offset);
        stream->print(" / ");
        stream->println(readPosition - offset);
        bytes[readPosition - offset] = b;

        if(readComplete){
          stream->print("Read ");
          stream->print(dimensions->getSize());
          stream->println(" bytes ... read is complete so break");
          break;
        }
        readPosition++;
      }
      
      return readComplete;
    }

    bool validate(){
      if(dimensions == NULL || !readComplete)return false;
      if(payload == NULL || dimensions->payload == 0)return false;
      if(header == NULL)return false;
      if(!isValidSchema(header[0]) || !isValidEncoding(header[1]))return false;
      
      schema = (FrameSchema)header[0];
      encoding = (MessageEncoding)header[1];

      //confirm checksum
      if(dimensions->checksum == 0)return true;
      
      /*switch(schema){
        
      }*/

      return true;
    }

    byte *getPayload(){ return payload; }
    int getPayloadSize(){ return dimensions == NULL ? -1 : dimensions->payload; }
    
    void reset(){
      readPosition = 0;
      readComplete = false;
      if(dimensions != NULL)delete dimensions;
      if(header != NULL)delete header;
      if(payload != NULL)delete payload;
      if(checksum != NULL)delete checksum;
      dimensions = NULL;
      header = NULL;
      payload = NULL;
      checksum = NULL;
    }
};


} //end namespace

#endif