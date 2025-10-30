#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <Arduino.h>

class JsonBuilder {
public:
    JsonBuilder();
    void reset();
    void begin();
    void add(const String &key, const String &value);
    void add(const String &key, bool value);
    void add(const String &key, int value);
    void end();
    String str() const;

private:
    String _json;
    bool _first = true;
};

#endif
