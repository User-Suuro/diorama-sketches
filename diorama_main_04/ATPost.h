#ifndef ATPOST_H
#define ATPOST_H

#include <Arduino.h>
#include "ATUtils.h"

enum ATPostState {
    ATPOST_IDLE,
    ATPOST_CONNECTING,
    ATPOST_SENDING_HEADER,
    ATPOST_SENDING_BODY,
    ATPOST_READING,
    ATPOST_DONE,
    ATPOST_ERROR
};

class ATPost {
private:
    ATUtils& at;
    String host;
    int port;
    String endpoint;
    String payload;
    String accumulated;
    String lastResponse;

    unsigned long stateStart;
    unsigned long readTimeout;
    bool debugMode;
    bool success;
    bool connected;
    ATPostState state;

public:
    ATPost(ATUtils& atUtils, bool debug = false);

    // Begin async POST
    void beginAsync(const String& host, int port, const String& endpoint, const String& payload, unsigned long timeout = 8000);

    // Update state machine
    void update();

    // Getters
    bool isFinished() const;
    bool isSuccess() const;
    String getLastResponse() const;
    bool isConnected() const;
    void setConnected(bool value);
};

#endif
