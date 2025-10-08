#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <Arduino.h>

class JsonBuilder {
private:
    String json;
    bool hasData;

public:
    JsonBuilder();

    // Add key-value pairs (string version)
    JsonBuilder& add(const String& key, const String& value);

    // Add key-value pairs (numeric version)
    JsonBuilder& add(const String& key, float value);
    JsonBuilder& add(const String& key, int value);
    JsonBuilder& add(const String& key, bool value);

    // Convert to JSON string
    String toString() const;

    // Clear builder
    void clear();
};

#endif
