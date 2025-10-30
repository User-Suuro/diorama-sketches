#include "JsonBuilder.h"

JsonBuilder::JsonBuilder() { reset(); }

void JsonBuilder::reset() {
    _json = "";
    _first = true;
}

void JsonBuilder::begin() {
    _json = "{";
    _first = true;
}

void JsonBuilder::add(const String &key, const String &value) {
    if (!_first) _json += ",";
    _json += "\"" + key + "\":\"" + value + "\"";
    _first = false;
}

void JsonBuilder::add(const String &key, bool value) {
    if (!_first) _json += ",";
    _json += "\"" + key + "\":" + String(value ? "true" : "false");
    _first = false;
}

void JsonBuilder::add(const String &key, int value) {
    if (!_first) _json += ",";
    _json += "\"" + key + "\":" + String(value);
    _first = false;
}

void JsonBuilder::end() { _json += "}"; }

String JsonBuilder::str() const { return _json; }
