#include "JsonBuilder.h"

JsonBuilder::JsonBuilder() {
    json = "{";
    hasData = false;
}

// Add string key-value
JsonBuilder& JsonBuilder::add(const String& key, const String& value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":\"" + value + "\"";
    hasData = true;
    return *this;
}

// Add float key-value
JsonBuilder& JsonBuilder::add(const String& key, float value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value, 2);
    hasData = true;
    return *this;
}

// Add int key-value
JsonBuilder& JsonBuilder::add(const String& key, int value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value);
    hasData = true;
    return *this;
}

// Add boolean key-value
JsonBuilder& JsonBuilder::add(const String& key, bool value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value ? "true" : "false");
    hasData = true;
    return *this;
}

// Finalize JSON string
String JsonBuilder::toString() const {
    return json + "}";
}

// Reset builder
void JsonBuilder::clear() {
    json = "{";
    hasData = false;
}
