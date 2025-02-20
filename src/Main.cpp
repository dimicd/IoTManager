#include "Main.h"
#include <time.h>
#include "classes/IoTDB.h"
#include "utils/Statistic.h"
#include <Wire.h>

IoTScenario iotScen;  // объект управления сценарием

String volStrForSave = "";
unsigned long currentMillis;
unsigned long prevMillis;

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    //создание экземпляров классов
    // myNotAsyncActions = new NotAsync(do_LAST);

    //инициализация файловой системы
    fileSystemInit();
    Serial.println(F("------------------------"));
    Serial.println("FIRMWARE NAME     " + String(FIRMWARE_NAME));
    Serial.println("FIRMWARE VERSION  " + String(FIRMWARE_VERSION));
    Serial.println("WEB VERSION       " + getWebVersion());
    Serial.println(F("------------------------"));

    //получение chip id
    setChipId();

    //синхронизация глобальных переменных с flash
    globalVarsSync();

    //подключаемся к роутеру
    routerConnect();

//инициализация асинхронного веб сервера и веб сокетов
#ifdef ASYNC_WEB_SERVER
    asyncWebServerInit();
#endif
#ifdef ASYNC_WEB_SOCKETS
    asyncWebSocketsInit();
#endif

//инициализация стандартного веб сервера и веб сокетов
#ifdef STANDARD_WEB_SERVER
    standWebServerInit();
#endif
#ifdef STANDARD_WEB_SOCKETS
    standWebSocketsInit();
#endif

    // NTP
    ntpInit();

    //инициализация mqtt
    mqttInit();

    // настраиваем i2c шину
    int pinSCL, pinSDA, i2cFreq;
    jsonRead(settingsFlashJson, "pinSCL", pinSCL, false);
    jsonRead(settingsFlashJson, "pinSDA", pinSDA, false);
    jsonRead(settingsFlashJson, "i2cFreq", i2cFreq, false);
    if (pinSCL && pinSDA && i2cFreq) {
#ifdef esp32_4mb
        Wire.end();
        Wire.begin(pinSDA, pinSCL, (uint32_t)i2cFreq);
#else
        Wire.begin(pinSDA, pinSCL);
        Wire.setClock(i2cFreq);
#endif
    }
    

    //настраиваем микроконтроллер
    configure("/config.json");

    //инициализация задач переодического выполнения
    periodicTasksInit();

    //синхронизация списка устройств
    addThisDeviceToList();

    //запуск работы udp
    asyncUdpInit();

    //подготавливаем сценарии
    iotScen.loadScenario("/scenario.txt");

    // создаем событие завершения конфигурирования для возможности выполнения блока кода при загрузке
    IoTItems.push_back((IoTItem *)new externalVariable("{\"id\":\"onStart\",\"val\":1,\"int\":60}"));
    generateEvent("onStart", "");

    stInit();

    // test
    Serial.println("-------test start--------");
    Serial.println("--------test end---------");

    // симуляция добавления внешних событий
    // IoTItems.push_back((IoTItem*)new externalVariable("{\"id\":\"rel1\",\"val\":10,\"int\":20}"));
    // IoTItems.push_back((IoTItem*)new externalVariable("{\"id\":\"rel4\",\"val\":34,\"int\":30}"));
    // пример получения JSON всех Items
    // Serial.println(getParamsJson());
    //чтение одного параметра
    // Serial.println(findIoTItem("t1")->getValue());
    //тест перебора пинов из расширения
    // for (int i = 109; i < 112; i++) {
    //     IoTgpio.pinMode(i, OUTPUT);
    //     IoTgpio.digitalWrite(i, !IoTgpio.digitalRead(i));
    //     delay(1000);
    //     IoTgpio.digitalWrite(i, !IoTgpio.digitalRead(i));
    //     delay(1000);
    // }
}

void loop() {
    // if(millis()%2000==0){
    //     //watch->settimeUnix(time(&iotTimeNow));
    //     Serial.println(watch->gettime("d-m-Y, H:i:s, M"));
    //     delay(1);
    // }

    //обновление задач таскера
    ts.update();

//отправка json
#ifdef QUEUE_FROM_STR
    if (sendJsonFiles) sendJsonFiles->loop();
#endif

#ifdef STANDARD_WEB_SERVER
    //обработка web сервера 1
    HTTP.handleClient();
#endif

#ifdef STANDARD_WEB_SOCKETS
    //обработка web сокетов
    standWebSocket.loop();
#endif

    //обновление mqtt
    mqttLoop();

#ifdef STANDARD_WEB_SERVER
    //обработка web сервера 2
    // HTTP.handleClient();
#endif

    // передаем управление каждому элементу конфигурации для выполнения своих функций
    for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        (*it)->loop();
        if ((*it)->iAmDead) {
            delete *it;
            IoTItems.erase(it);
            break;
        }
    }

    handleOrder();

    handleEvent();

#ifdef STANDARD_WEB_SERVER
    //обработка web сервера 3
    // HTTP.handleClient();
#endif

    // сохраняем значения IoTItems в файл каждую секунду, если были изменения (установлены маркеры на сохранение)
    // currentMillis = millis();
    // if (currentMillis - prevMillis >= 1000) {
    //    prevMillis = millis();
    //    volStrForSave = "";
    //    for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
    //        if ((*it)->needSave) {
    //            (*it)->needSave = false;
    //            volStrForSave = volStrForSave + (*it)->getID() + "=" + (*it)->getValue() + ";";
    //        }
    //    }
    //
    //    if (volStrForSave != "") {
    //        Serial.print("volStrForSave: ");
    //        Serial.println(volStrForSave.c_str());
    //    }
    //}
}

// File dir = FileFS.open("/", "r");
// String out;
// printDirectory(dir, out);
// Serial.println(out);

//=======проверка очереди из структур=================

// myDB = new IoTDB;
// QueueItems myItem;
// myItem.myword = "word1";
// myDB->push(myItem);
// myItem.myword = "word2";
// myDB->push(myItem);
// myItem.myword = "word3";
// myDB->push(myItem);
// Serial.println(myDB->front().myword);
// Serial.println(myDB->front().myword);
// Serial.println(myDB->front().myword);

// Serial.println(FileList("lg"));
