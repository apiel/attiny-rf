
#include "RfReceiver.h"
// #include <math.h> // pow not working :-/

int pow(int x,int n)
{
    int i; /* Variable used in loop counter */
    int number = 1;

    for (i = 0; i < n; ++i)
        number *= x;

    return(number);
}

enum {
   numProto = sizeof(protocoles) / sizeof(protocoles[0])
};

RfReceiver rfReceiver = RfReceiver();

RfReceiver::RfReceiver()
{
    _setMainLatch();
}

void handleInterrupt()
{
    Serial.print("d: ");
    rfReceiver.onInterrupt();
}

void RfReceiver::init(void (*callback)(char * result)) {
    _callback = callback;
    _setMainLatch();
}

void RfReceiver::start(int pin, void (*callback)(char * result)) {
    init(callback);
    _attachInterrupt(pin);
}

void RfReceiver::_attachInterrupt(int pin) {
    pinMode(pin, INPUT_PULLUP);
    // Serial.println("_attachInterrupt");
    // printf("attach interrupt %d\n", pin);
    attachInterrupt(pin, handleInterrupt, CHANGE);
}

void RfReceiver::_setMainLatch() { // we could easily write unit test there
    _mainLatch = { 65535, 0 };
    for(unsigned int i = 0; i < numProto; i++) {
        printf("RF: Proto [%d: %s] between %d and %d\n", i, protocoles[i].uid, protocoles[i].latch.min, protocoles[i].latch.max);
        if (protocoles[i].latch.min < _mainLatch.min) {
            _mainLatch.min = protocoles[i].latch.min;
        }
        if (protocoles[i].latch.max > _mainLatch.max) {
            _mainLatch.max = protocoles[i].latch.max;
        }
    }
    printf("RF: Main latch between %d and %d\n", _mainLatch.min, _mainLatch.max);
}

void RfReceiver::onInterrupt() {
    long time = micros();
    unsigned int duration = time - _lastTime;
    // Serial.println(duration);
    // printf(",%d", duration);
    if (duration > _mainLatch.min && duration < _mainLatch.max) {
        _checkForResult(duration);
        _initCurrentProtocole(duration);
        // _currentProtocole = 100; printf("\n");
    }
    // if (_currentProtocole > 0) printf(",%d", duration);
    _logTiming(duration);
    _lastTime = time;
}

// for the moment we support only one protocole at once
// but we could have an array of current protocole with { id, timings, ...}
void RfReceiver::_initCurrentProtocole(unsigned int duration) { // we could easily write unit test there
    for(_currentProtocole = numProto - 1; _currentProtocole > -1 ;_currentProtocole--) {
        if (_isLatch(duration)) {
             break;
        }
    }
    for(int i = 0; i < RF_RESULT_SIZE; i++) {
        _result[i] = '0';
    }
    _falseTimingCount = 0;
    _timingsPos = 0;
}

void RfReceiver::_logTiming(unsigned int duration) {
    if (_currentProtocole > -1 && _timingsPos < RF_MAX_CHANGES) {
        if (_isZero(duration)) {
            _timingsPos++;
        } else if (_isOne(duration)) {
            int pos = _timingsPos/RF_BIN_SPLIT;
            _result[pos] += pow(2, _timingsPos - (pos * RF_BIN_SPLIT));
            // printf("(%d) %d %d %d [%c]\n", RF_BIN_SPLIT, _timingsPos, _timingsPos/RF_BIN_SPLIT, pow(2, _timingsPos - (pos * RF_BIN_SPLIT)), _result[_timingsPos/RF_BIN_SPLIT]);
            _timingsPos++;
        } else {
            _falseTimingCount++;
            if (_falseTimingCount > RF_MAX_FALSE) {
                _currentProtocole = -1;
            }
        }
    }
}

bool RfReceiver::_isInRange(unsigned int duration, const RfMinMax * minMax) {
    return duration > minMax->min && duration < minMax->max;
}

bool RfReceiver::_isLatch(unsigned int duration) {
    return _isInRange(duration, &protocoles[_currentProtocole].latch);
}

bool RfReceiver::_isZero(unsigned int duration) {
    return _isInRange(duration, &protocoles[_currentProtocole].zero);
}

bool RfReceiver::_isOne(unsigned int duration) {
    return _isInRange(duration, &protocoles[_currentProtocole].one);
}

void RfReceiver::_checkForResult(unsigned int duration) {
    _available = false;
    if (_currentProtocole > -1 && _timingsPos > 0 && (_timingsPos >= RF_MAX_CHANGES || _isLatch(duration))) {
        int pos = _timingsPos/RF_BIN_SPLIT;
        if (pos > 4) { // at least for char result
            Serial.print("yoyo: ");
            Serial.println(_resultFound);
            strcpy(_resultFound, _result);
            _resultFound[pos++] = '-';
            _resultFound[pos++] = '0' + _currentProtocole;
            _resultFound[pos++] = '\0';
            // printf("proto: %d resres: %s\n", _currentProtocole, _result);
            // _callback(_result);
            _available = true;
            Serial.print("foundcode: ");
            Serial.println(_resultFound);
            _currentProtocole = -1;
        }
    }
}

char * RfReceiver::getResult() {
    _available = false;
    Serial.print("getResult: ");
    Serial.println(_resultFound);
    return _resultFound;
}

bool RfReceiver::isAvailable() {
    return _available;
}
