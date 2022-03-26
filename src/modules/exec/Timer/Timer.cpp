#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class Timer : public IoTItem {
   private:
    bool _unfin = false;
    bool _ticker = false;
    bool _repeat = false;

   public:
    Timer(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "countDown", value.valD);
        jsonRead(parameters, "ticker", _ticker);
        jsonRead(parameters, "repeat", _repeat);
        if (_repeat) _repeat = value.valD;  // если в параметрах просят повторить, то запоминаем настроечное значение отчета
        _unfin = !value.valD;

        
    }

    void doByInterval() {
        if (!_unfin && value.valD) {
            value.valD--;
            if (value.valD == 0) {
                regEvent(value.valD, "Time's up");
                if (_repeat) value.valD = _repeat;
            }
        }

        if (_ticker) regEvent(value.valD, "Timer tick");
    }

    ~Timer() {};
};

void* getAPI_Timer(String subtype, String param) {
    if (subtype == F("Timer")) {
        return new Timer(param);
    } else {
        return nullptr;
    }
}
