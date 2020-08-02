#ifndef CHETCH_UTILS_H
#define CHETCH_UTILS_H

#include <Arduino.h>

namespace Chetch{

class Utils{
  private:
    static char hexDigit(char c);
    
  public:
    static char *urlencode(char* dst, char* src, char *specialChars = "$&+,/:;=?@ <>#%{}|~[]`");
    static char *urldecode(char* src);
    static int parseNameValuePair(char *s2parse, char *delimiter, char *results[], int maxResults, boolean decodeUrl);
    static int parseQueryString(char *s2parse, char *results[], int maxResults, boolean decodeUrl = true);
    static char *getValue(char *pname, char *results[], int resultsCount);
    static void addValue(char *params[], char *pname, char *pvalue, int i);
    static char *buildNameValueString(char *str, char *params[], int paramCount, char *delimiter, boolean encodeUrl);
    static char *buildQueryString(char *str, char *params[], int paramCount, boolean encodeUrl = true);
	
    static char *getStringFromProgmem(char *buffer, byte idx, char* const stringTable[]);
};

} //end namespace

#endif