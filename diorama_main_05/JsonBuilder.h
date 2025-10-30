#ifndef JSONBUILDER_H
#define JSONBUILDER_H

#include <Arduino.h>

class JsonBuilder {
private:
    String json;
    bool hasData;

public:
    JsonBuilder();

    // Core method for string-based values
    void add(const String &key, const String &value, bool quote = true);

    // Overloads for primitive types
    void add(const String &key, int value);
    void add(const String &key, long value);
    void add(const String &key, float value, uint8_t precision = 2);
    void add(const String &key, double value, uint8_t precision = 2);
    void add(const String &key, bool value);

    String build();
    void clear();
};

#endif
