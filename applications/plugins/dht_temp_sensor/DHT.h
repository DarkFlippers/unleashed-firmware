#ifndef DHT_H_
#define DHT_H_

#include <furi_hal_resources.h>

/* Настройки */
#define DHT_TIMEOUT 65534 //Количество итераций, после которых функция вернёт пустые значения
#define DHT_POLLING_CONTROL 1 //Включение проверки частоты опроса датчика
#define DHT_POLLING_INTERVAL_DHT11 \
    2000 //Интервал опроса DHT11 (0.5 Гц по даташиту). Можно поставить 1500, будет работать
#define DHT_POLLING_INTERVAL_DHT22 1000 //Интервал опроса DHT22 (1 Гц по даташиту)
#define DHT_IRQ_CONTROL //Выключать прерывания во время обмена данных с датчиком
/* Структура возвращаемых датчиком данных */
typedef struct {
    float hum;
    float temp;
} DHT_data;

/* Тип используемого датчика */
typedef enum { DHT11, DHT22 } DHT_type;

/* Структура объекта датчика */
typedef struct {
    char name[11];
    const GpioPin* GPIO; //Пин датчика
    DHT_type type; //Тип датчика (DHT11 или DHT22)

//Контроль частоты опроса датчика. Значения не заполнять!
#if DHT_POLLING_CONTROL == 1
    uint32_t lastPollingTime; //Время последнего опроса датчика
    float lastTemp; //Последнее значение температуры
    float lastHum; //Последнее значение влажности
#endif
} DHT_sensor;

/* Прототипы функций */
DHT_data DHT_getData(DHT_sensor* sensor); //Получить данные с датчика

#endif
