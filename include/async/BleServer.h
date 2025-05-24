#include <async/Tick.h>
#include <async/Setting.h>
#include <async/FastList.h>
#include <NimBLEDevice.h>

namespace async { 
    template <typename T>
    class CharacteristicCallbacks {
        static_assert(sizeof(T) == 0, "Unsupported type for Setting");
    };

    template<>
    class CharacteristicCallbacks<int> : public NimBLECharacteristicCallbacks {
        private:
        Setting<int> * setting;
        NimBLECharacteristic* characteristic;

        void onRead(NimBLECharacteristic* pCharacteristic){
            Serial.print(pCharacteristic->getUUID().toString().c_str());
            Serial.print(": onRead(), value: ");
            Serial.println(pCharacteristic->getValue().c_str());
        };

        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
            Serial.print(pCharacteristic->getUUID().toString().c_str());
            Serial.print(": onWrite(), value: ");
            setting->set(atoi(pCharacteristic->getValue().c_str()));
        }

        public: CharacteristicCallbacks(Setting<int> * setting, NimBLECharacteristic* characteristic): setting(setting), characteristic(characteristic) {
            setting->onChange([characteristic](int newValue, int oldValue) {
                characteristic->setValue(newValue);
                characteristic->notify();
            });
        }
    };

    template<>
    class CharacteristicCallbacks<float> : public NimBLECharacteristicCallbacks {
        private:
        Setting<float> * setting;
        NimBLECharacteristic* characteristic;

        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
            setting->set(atof(pCharacteristic->getValue().c_str()));
        }

        public: CharacteristicCallbacks(Setting<float> * setting, NimBLECharacteristic* characteristic): setting(setting), characteristic(characteristic) {}
    };

    template<>
    class CharacteristicCallbacks<double> : public NimBLECharacteristicCallbacks {
        private:
        Setting<double> * setting;
        NimBLECharacteristic* characteristic;

        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
            setting->set(atof(pCharacteristic->getValue().c_str()));
        }

        public: CharacteristicCallbacks(Setting<double> * setting, NimBLECharacteristic* characteristic): setting(setting), characteristic(characteristic) {}
    };

    template<>
    class CharacteristicCallbacks<bool> : public NimBLECharacteristicCallbacks {
        private:
        Setting<bool> * setting;
        NimBLECharacteristic* characteristic;

        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
            setting->set(pCharacteristic->getValue().c_str() == "true");
        }

        public: CharacteristicCallbacks(Setting<bool> * setting, NimBLECharacteristic* characteristic): setting(setting), characteristic(characteristic) {}
    };

    template<>
    class CharacteristicCallbacks<String> : public NimBLECharacteristicCallbacks {
        private:
        Setting<String> * setting;
        NimBLECharacteristic* characteristic;

        void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
            setting->set(pCharacteristic->getValue());
        }

        public: CharacteristicCallbacks(Setting<String> * setting, NimBLECharacteristic* characteristic): setting(setting), characteristic(characteristic) {
            setting->onChange([characteristic](String newValue, String oldValue) {
                characteristic->setValue(newValue);
                characteristic->notify();
            });
        }
    };


    class BleServer : public Tick, NimBLEServerCallbacks  {
        private:
            const char * name;
            NimBLEServer * pServer;
            NimBLEService * pSettingService;
            NimBLEAdvertising * pAdvertising;
           // VoidCallback onConnect;

            void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
                Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());

                /**
                 *  We can use the connection handle here to ask for different connection parameters.
                 *  Args: connection handle, min connection interval, max connection interval
                 *  latency, supervision timeout.
                 *  Units; Min/Max Intervals: 1.25 millisecond increments.
                 *  Latency: number of intervals allowed to skip.
                 *  Timeout: 10 millisecond increments.
                 */
                pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
            }

            void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
                Serial.printf("Client disconnected - start advertising\n");
                NimBLEDevice::startAdvertising();
            }

        public:
            BleServer(const char * name) : name(name) {
            }

            bool start() {
                NimBLEDevice::init(name);

                pServer = NimBLEDevice::createServer();
                pServer->setCallbacks(this);
                pSettingService = pServer->createService("00000000-0000-0000-0000-000000000000");
                pSettingService->start();

                pAdvertising = NimBLEDevice::getAdvertising();
                pAdvertising->addServiceUUID(pSettingService->getUUID());
                pAdvertising->enableScanResponse(true);
                pAdvertising->start();

                return true;
            }

            void addSetting(Setting<int> * setting) {
                auto charachetistic = pSettingService->createCharacteristic(setting->getUuid16(), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
                charachetistic->setValue(setting->get());
                charachetistic->setCallbacks(new CharacteristicCallbacks<int>(setting, charachetistic));
            }

            void addSetting(Setting<float> * setting) {
                auto charachetistic = pSettingService->createCharacteristic(setting->getUuid16(), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
                charachetistic->setValue(setting->get());
                charachetistic->setCallbacks(new CharacteristicCallbacks<float>(setting, charachetistic));
            }

            void addSetting(Setting<double> * setting) {
                auto charachetistic = pSettingService->createCharacteristic(setting->getUuid16(), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
                charachetistic->setValue(setting->get());
                charachetistic->setCallbacks(new CharacteristicCallbacks<double>(setting, charachetistic));
            }

            void addSetting(Setting<bool> * setting) {
                auto charachetistic = pSettingService->createCharacteristic(setting->getUuid16(), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
                charachetistic->setValue(setting->get());
                charachetistic->setCallbacks(new CharacteristicCallbacks<bool>(setting, charachetistic));
            }

            void addSetting(Setting<String> * setting) {
                auto charachetistic = pSettingService->createCharacteristic(setting->getUuid16(), NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
                charachetistic->setValue(setting->get());
                charachetistic->setCallbacks(new CharacteristicCallbacks<String>(setting, charachetistic));
            }

            bool tick() {
                return true;
            }
    };
}