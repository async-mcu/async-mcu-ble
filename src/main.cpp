#include <Arduino.h>
#include <async/Executor.h>
#include <async/Setting.h>
#include <async/BleServer.h>

using namespace async;

Executor executor;
BleServer server("test");

Setting<int> settingInt("int", 0x0000, 10);
Setting<float> settingFloat("float", 0x0001, 0.5);
Setting<double> settingDouble("double", 0x0002, 10);
Setting<bool> settingBool("bool", 0x0003, true);
Setting<String> settingString("string", 0x0004, "123");

void setup() {
  Serial.begin(115200);
  Serial.println("setup");

  executor.start();
  executor.add(&server);
  executor.add(&settingInt);
  executor.add(&settingFloat);
  executor.add(&settingDouble);
  executor.add(&settingBool);
  executor.add(&settingString);

  server.onConnect([](NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());
  });

  server.onDisconnect([](NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    Serial.printf("Client disconnected - start advertising\n");
  });

  info("settingInt  %d", settingInt.get());
  info("settingFloat  %f", settingFloat.get());
  info("settingDouble  %f", settingDouble.get());
  info("settingBool  %d", settingBool.get());
  info("settingString %s", settingString.get());

  settingInt.onChange([](int current, int last) {
    info("settingInt curr %d, last %d", current, last);
  });

  settingString.onChange([](String current, String last) {
    info("settingString curr %s, last %s", current, last);
  });


  executor.onRepeat(2000, []() {
    settingInt.getAndSet([](int val) {
      info("getAndSet settingInt curr %d", val);
      return val + 5;
    });

    settingFloat.getAndSet([](float val) {
      info("getAndSet settingFloat curr %f", val);
      return val + 0.35;
    });

    settingDouble.getAndSet([](double val) {
      info("getAndSet settingDouble curr %f", val);
      return val + 0.35;
    });

    settingBool.getAndSet([](bool val) {
      info("getAndSet settingBool curr %d", val);
      return !val;
    });

    settingString.getAndSet([](String val) {
      info("getAndSet settingString curr %s", val);
      return String(atoi(val.c_str()) + 1);
    });
  });

  server.addSetting(&settingInt);
  server.addSetting(&settingFloat);
  server.addSetting(&settingDouble);
  server.addSetting(&settingBool);
  server.addSetting(&settingString);
}

void loop() {
  executor.tick();
}