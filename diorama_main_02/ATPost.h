#ifndef ATPOST_H
#define ATPOST_H

#include <Arduino.h>
#include "ATSend.h"

class ATPost {
public:
    ATPost(ATSend &at, Stream &serialPort, const String &host, int port);

    // blocking version
    String postRequest(const String &path, const String &jsonBody, bool printDebug = false);

    // async version
    void beginAsync(const String &path, const String &jsonBody, bool printDebug = false);
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

    enum AsyncState { IDLE, CONNECTING, WAIT_PROMPT, SENDING, READING, DONE, ERROR };
    AsyncState _state = IDLE;

    String _pendingPath;
    String _pendingBody;
    String _response;
    String _sendBuffer;
    bool _printDebug = false;
    unsigned long _lastAction = 0;

    String _readFullResponse(unsigned long idleTimeout);
    String _extractRawBody(const String &raw);
};

#endif
