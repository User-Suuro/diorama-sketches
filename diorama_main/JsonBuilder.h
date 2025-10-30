#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <Arduino.h>

class JsonBuilder {
public:
  JsonBuilder();

  JsonBuilder& add(const String &key, const String &value);
  JsonBuilder& add(const String &key, int value);
  JsonBuilder& add(const String &key, float value);
  JsonBuilder& add(const String &key, bool value);

  String build() const;

private:
  String json;
  bool first;
};

#endif
