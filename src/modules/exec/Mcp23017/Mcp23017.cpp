#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"
#include <Adafruit_MCP23X17.h>

void scanI2C();

class Mcp23017Driver : public IoTGpio {
   private:
    Adafruit_MCP23X17 _mcp;

   public:
    Mcp23017Driver(int index, String addr) : IoTGpio(index) {
        if (!_mcp.begin_I2C(hexStringToUint8(addr))) {
            Serial.println("MCP23X17 Init Error.");
        }
    }

    void pinMode(uint8_t pin, uint8_t mode) {
        _mcp.pinMode(pin, mode);
    }

    void digitalWrite(uint8_t pin, uint8_t val) {
        _mcp.digitalWrite(pin, val);
    }

    int digitalRead(uint8_t pin) {
        return _mcp.digitalRead(pin);
    }

    void digitalInvert(uint8_t pin) {
        _mcp.digitalWrite(pin, 1 - _mcp.digitalRead(pin));
    }

    ~Mcp23017Driver() {};
};


class Mcp23017 : public IoTItem {
   private:
    Mcp23017Driver* _driver;

   public:
    Mcp23017(String parameters) : IoTItem(parameters) {
        _driver = nullptr;
        
        String addr;
        jsonRead(parameters, "addr", addr);
        if (addr == "") {
            scanI2C();
            return;
        }

        int index;
        jsonRead(parameters, "index", index);
        if (index > 4) {
            Serial.println("MCP23X17 wrong index. Must be 0 - 4");
            return;
        }
        
        _driver = new Mcp23017Driver(index, addr);        
    }

    void doByInterval() {}

    //возвращает ссылку на экземпляр класса Mcp23017Driver
    IoTGpio* getGpioDriver() {
        return _driver;
    }

    ~Mcp23017() {
        delete _driver;
    };
};

void* getAPI_Mcp23017(String subtype, String param) {
    if (subtype == F("Mcp23017")) {
        return new Mcp23017(param);
    } else {
        return nullptr;
    }
}
