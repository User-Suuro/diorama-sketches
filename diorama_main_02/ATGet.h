#ifndef ATGET_H
#define ATGET_H

#include <Arduino.h>
#include "ATSend.h"

class ATGet {
public:
    ATGet(ATSend &at, Stream &serialPort, const String &host, int port);

    // synchronous (blocking)
    String getRequest(const String &path, bool printDebug = false);

    // asynchronous (non-blocking)
    void beginAsync(const String &path, bool printDebug = false);
    void updateAsync();
    bool isBusy() const;
    bool hasResponse() const;
    String getResponse();
    void clearResponse();

private:
    ATSend &_at;
    Stream &_serial;
    String _host;
    int _port;

    // state machine
    enum AsyncState { IDLE, CONNECTING, WAIT_PROMPT, SENDING, READING, DONE, ERROR };
    AsyncState _state = IDLE;
    String _pendingPath;
    String _response;
    String _sendBuffer;
    bool _printDebug = false;
    unsigned long _lastAction = 0;

    String _readFullResponse(unsigned long idleTimeout);
    String _extractRawBody(const String &raw);
};

#endif
