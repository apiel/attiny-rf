
#ifndef __RFRECEIVER_H__
#define __RFRECEIVER_H__

#include "Arduino.h"
#include <stdint.h>

#define RF_BIN_SPLIT 6
#define RF_RESULT_SIZE 44
#define RF_MAX_CHANGES RF_BIN_SPLIT*RF_RESULT_SIZE // 264
#define RF_MAX_FALSE 100

struct RfMinMax {
    uint16_t min;
    uint16_t max;
};

struct RfProtocol {
    const char * uid;
    RfMinMax latch;
    RfMinMax latch2;
    RfMinMax zero;
    RfMinMax one;
};

// 78690 could be second latch but doesnt fit in 65535
static const RfProtocol protocoles[] = {
    {"remote", { 7500, 8100 }, { 0, 0 }, { 180, 360 }, { 630, 820 }},
    // {"cercle", { 10000, 15000 }, { 0, 0 }, { 200, 400 }, { 500, 700 }},
};

class RfReceiver
{
    public:
        RfReceiver();
        void onInterrupt();
        char * getResult();
        bool isAvailable();

    protected:
        void (*_callback)(char * result);
        RfMinMax _mainLatch;
        int _currentProtocole = -1;
        unsigned int _timingsPos;
        unsigned long _lastTime = 0;
        unsigned int _falseTimingCount = 0;
        bool _available = false;
        char _result[RF_RESULT_SIZE + 2];
        char _resultFound[RF_RESULT_SIZE + 2]; // might find a better way than to copy the result
        void _setMainLatch();
        void _attachInterrupt(int pin);
        void _initCurrentProtocole(unsigned int duration);
        void _logTiming(unsigned int duration);
        void _checkForResult(unsigned int duration);
        bool _isInRange(unsigned int duration, const RfMinMax * minMax);
        bool _isLatch(unsigned int duration);
        bool _isZero(unsigned int duration);
        bool _isOne(unsigned int duration);
};

extern RfReceiver rfReceiver;

#endif
