#include "ATUtils.h"

ATUtils::ATUtils(Stream &stream, Stream *debugSerial) : stream(stream), debug(debugSerial) {}

String ATUtils::sendCommand(const String &cmd, unsigned long timeoutMs, unsigned long silentMs) {
  // Flush any existing input (avoid blocking)
  while (stream.available()) stream.read();

  if (debug) {
    debug->print(F("📤 CMD → "));
    debug->println(cmd);
  }

  // Send the command followed by CRLF (println sends \r\n on Arduino)
  stream.println(cmd);

  // Small initial delay to let device begin replying (important for slow-to-start devices)
  // not too long to avoid unnecessary blocking
  delay(10);

  String resp = readResponse(cmd, timeoutMs, silentMs);

  if (debug) {
    debug->println(F("📥 RESPONSE START ====="));
    debug->println(resp);
    debug->println(F("===== RESPONSE END ====="));
  }

  return resp;
}

String ATUtils::readResponse(const String &cmd, unsigned long timeoutMs, unsigned long silentMs) {
  String response;
  unsigned long start = millis();
  unsigned long lastRead = millis();

  // Read loop: collect bytes until either overall timeout, or silent period after data flow stops
  while (millis() - start < timeoutMs) {
    while (stream.available()) {
      int ch = stream.read();
      if (ch >= 0) {
        response += (char)ch;
        lastRead = millis();
      }
    }

    // If we've received something and no new data for silentMs => assume done
    if (response.length() > 0 && (millis() - lastRead) > silentMs) {
      break;
    }

    // Quick yield to allow background tasks and avoid starving SoftwareSerial
    // but keep it minimal
    delay(1);

    // If response contains explicit terminal tokens, we can finish early
    // (handles cases like "...OK\r\n" or "...ERROR\r\n")
    if (response.indexOf("\r\nOK") >= 0 || response.indexOf("\nOK") >= 0 ||
        response.indexOf("\r\nERROR") >= 0 || response.indexOf("\nERROR") >= 0) {
      // give a tiny grace window for any trailing bytes
      unsigned long graceStart = millis();
      while (millis() - graceStart < 30) {
        while (stream.available()) {
          int ch = stream.read();
          if (ch >= 0) {
            response += (char)ch;
            lastRead = millis();
          }
        }
      }
      break;
    }
  }

  // Normalize line endings: convert lone '\r' or '\n' combos into '\n' for easier processing
  // (do this by replacing "\r\n" -> "\n", then "\r" -> "\n")
  response.replace("\r\n", "\n");
  response.replace('\r', '\n');

  // Trim leading/trailing whitespace/newlines
  response = trimBoth(response);

  // Remove echoed command if the first line exactly equals the command (case-insensitive)
  // Split into first-line / rest
  int nl = response.indexOf('\n');
  if (nl >= 0) {
    String firstLine = response.substring(0, nl);
    String rest = response.substring(nl + 1);
    if (trimBoth(firstLine).equalsIgnoreCase(trimBoth(cmd))) {
      response = trimBoth(rest);
    }
  } else {
    // Single-line response — if it exactly matches the command, clear it
    if (trimBoth(response).equalsIgnoreCase(trimBoth(cmd))) {
      response = "";
    }
  }

  return response;
}

String ATUtils::trimBoth(const String &s) {
  int start = 0;
  int end = s.length() - 1;
  while (start <= end && isspace((unsigned char)s[start])) start++;
  while (end >= start && isspace((unsigned char)s[end])) end--;
  if (end < start) return String();
  return s.substring(start, end + 1);
}
