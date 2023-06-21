#ifndef _DEBUG_H_
#define _DEBUG_H_

// #define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTF(fmt, ...)
#endif

#endif /* _DEBUG_H_ */