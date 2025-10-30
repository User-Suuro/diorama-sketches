#ifndef ATGET_H
#define ATGET_H

#include "ATUtils.h"

class ATGet {
public:
  ATGet(ATUtils &util, const String &host, int port);

  bool beginAsync(const String &endpoint);
  bool process();
  bool isFinished() const;
  String getResponse() const;

private:
  enum State {
    IDLE,
    CONNECTING,
    REQUESTING,
    READING,
    DONE,
    ERROR
  };

  ATUtils &_util;
  String _host;
  int _port;
  String _endpoint;
  String _response;
  State _state;
  unsigned long _lastAction;
  unsigned long _timeout;

  // Temporary dynamic buffer for raw data
  char *_tempBuffer;
  size_t _bufferSize;
  size_t _dataLength;

  bool _connect();
  bool _sendRequest();
  void _readResponse();
  void _freeBuffer(); // memory management helper
};

#endif
