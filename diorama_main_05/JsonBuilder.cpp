#include "JsonBuilder.h"

JsonBuilder::JsonBuilder() {
    json = "{";
    hasData = false;
}

void JsonBuilder::add(const String &key, const String &value, bool quote) {
    if (hasData) json += ",";
    json += "\"" + key + "\":";
    if (quote)
        json += "\"" + value + "\"";
    else
        json += value;
    hasData = true;
}

void JsonBuilder::add(const String &key, int value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value);
    hasData = true;
}

void JsonBuilder::add(const String &key, long value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value);
    hasData = true;
}

void JsonBuilder::add(const String &key, float value, uint8_t precision) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value, precision);
    hasData = true;
}

void JsonBuilder::add(const String &key, double value, uint8_t precision) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + String(value, precision);
    hasData = true;
}

void JsonBuilder::add(const String &key, bool value) {
    if (hasData) json += ",";
    json += "\"" + key + "\":" + (value ? "true" : "false");
    hasData = true;
}

String JsonBuilder::build() {
    json += "}";
    return json;
}

void JsonBuilder::clear() {
    json = "{";
    hasData = false;
}
