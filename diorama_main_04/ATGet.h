#ifndef ATGET_H
#define ATGET_H

#include <Arduino.h>
#include "ATUtils.h"

enum ATGetState {
    ATGET_IDLE,
    ATGET_CONNECTING,
    ATGET_SENDING,
    ATGET_READING,
    ATGET_DONE,
    ATGET_ERROR
};

class ATGet {
private:
    ATUtils& at;
    String host;
    int port;
    String endpoint;
    String lastResponse;
    String accumulated;
    unsigned long stateStart;
    unsigned long readTimeout;
    bool debugMode;
    bool success = false;
    bool connected = false;
    ATGetState state = ATGET_IDLE;

public:
    ATGet(ATUtils& atUtils, bool debug = false);

    void beginAsync(const String& host, int port, const String& endpoint, unsigned long timeout = 8000);
    void update();
    bool isFinished() const;
    bool isSuccess() const;
    String getLastResponse() const;
    bool isConnected() const;
    void setConnected(bool state);

};

#endif
