#include "JsonBuilder.h"

JsonBuilder::JsonBuilder() : json("{"), first(true) {}

JsonBuilder& JsonBuilder::add(const String &key, const String &value) {
  if (!first) json += ",";
  json += "\"" + key + "\":\"" + value + "\"";
  first = false;
  return *this;
}

JsonBuilder& JsonBuilder::add(const String &key, int value) {
  if (!first) json += ",";
  json += "\"" + key + "\":" + String(value);
  first = false;
  return *this;
}

JsonBuilder& JsonBuilder::add(const String &key, float value) {
  if (!first) json += ",";
  json += "\"" + key + "\":" + String(value, 2); // 2 decimal places
  first = false;
  return *this;
}

JsonBuilder& JsonBuilder::add(const String &key, bool value) {
  if (!first) json += ",";
  json += "\"" + key + "\":" + String(value ? "true" : "false");
  first = false;
  return *this;
}

String JsonBuilder::build() const {
  return json + "}";
}
