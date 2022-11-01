#include "DHT.h"

#define lineDown() furi_hal_gpio_write(sensor->GPIO, false)
#define lineUp() furi_hal_gpio_write(sensor->GPIO, true)
#define getLine() furi_hal_gpio_read(sensor->GPIO)
#define Delay(d) furi_delay_ms(d)

DHT_data DHT_getData(DHT_sensor* sensor) {
    DHT_data data = {-128.0f, -128.0f};

#if DHT_POLLING_CONTROL == 1
    /* Ограничение по частоте опроса датчика */
    //Определение интервала опроса в зависимости от датчика
    uint16_t pollingInterval;
    if(sensor->type == DHT11) {
        pollingInterval = DHT_POLLING_INTERVAL_DHT11;
    } else {
        pollingInterval = DHT_POLLING_INTERVAL_DHT22;
    }

    //Если интервал маленький, то возврат последнего удачного значения
    if((furi_get_tick() - sensor->lastPollingTime < pollingInterval) &&
       sensor->lastPollingTime != 0) {
        data.hum = sensor->lastHum;
        data.temp = sensor->lastTemp;
        return data;
    }
    sensor->lastPollingTime = furi_get_tick() + 1;
#endif

    //Опускание линии данных на 18 мс
    lineDown();
#ifdef DHT_IRQ_CONTROL
    //Выключение прерываний, чтобы ничто не мешало обработке данных
    __disable_irq();
#endif
    Delay(18);

    //Подъём линии
    lineUp();

    /* Ожидание ответа от датчика */
    uint16_t timeout = 0;
    while(!getLine()) {
        timeout++;
        if(timeout > DHT_TIMEOUT) {
#ifdef DHT_IRQ_CONTROL
            __enable_irq();
#endif
            //Если датчик не отозвался, значит его точно нет
            //Обнуление последнего удачного значения, чтобы
            //не получать фантомные значения
            sensor->lastHum = -128.0f;
            sensor->lastTemp = -128.0f;

            return data;
        }
    }
    //Ожидание спада
    while(getLine()) {
        timeout++;
        if(timeout > DHT_TIMEOUT) {
#ifdef DHT_IRQ_CONTROL
            __enable_irq();
#endif
            //Если датчик не отозвался, значит его точно нет
            //Обнуление последнего удачного значения, чтобы
            //не получать фантомные значения
            sensor->lastHum = -128.0f;
            sensor->lastTemp = -128.0f;

            return data;
        }
    }
    timeout = 0;
    //Ожидание подъёма
    while(!getLine()) {
        timeout++;
        if(timeout > DHT_TIMEOUT) {
            if(timeout > DHT_TIMEOUT) {
#ifdef DHT_IRQ_CONTROL
                __enable_irq();
#endif
                //Если датчик не отозвался, значит его точно нет
                //Обнуление последнего удачного значения, чтобы
                //не получать фантомные значения
                sensor->lastHum = -128.0f;
                sensor->lastTemp = -128.0f;

                return data;
            }
        }
    }
    timeout = 0;
    //Ожидание спада
    while(getLine()) {
        timeout++;
        if(timeout > DHT_TIMEOUT) {
#ifdef DHT_IRQ_CONTROL
            __enable_irq();
#endif
            //Если датчик не отозвался, значит его точно нет
            //Обнуление последнего удачного значения, чтобы
            //не получать фантомные значения
            sensor->lastHum = -128.0f;
            sensor->lastTemp = -128.0f;
            return data;
        }
    }

    /* Чтение ответа от датчика */
    uint8_t rawData[5] = {0, 0, 0, 0, 0};
    for(uint8_t a = 0; a < 5; a++) {
        for(uint8_t b = 7; b != 255; b--) {
            uint16_t hT = 0, lT = 0;
            //Пока линия в низком уровне, инкремент переменной lT
            while(!getLine() && lT != 65535) lT++;
            //Пока линия в высоком уровне, инкремент переменной hT
            timeout = 0;
            while(getLine() && hT != 65535) hT++;
            //Если hT больше lT, то пришла единица
            if(hT > lT) rawData[a] |= (1 << b);
        }
    }
#ifdef DHT_IRQ_CONTROL
    //Включение прерываний после приёма данных
    __enable_irq();
#endif
    /* Проверка целостности данных */
    if((uint8_t)(rawData[0] + rawData[1] + rawData[2] + rawData[3]) == rawData[4]) {
        //Если контрольная сумма совпадает, то конвертация и возврат полученных значений
        if(sensor->type == DHT22) {
            data.hum = (float)(((uint16_t)rawData[0] << 8) | rawData[1]) * 0.1f;
            //Проверка на отрицательность температуры
            if(!(rawData[2] & (1 << 7))) {
                data.temp = (float)(((uint16_t)rawData[2] << 8) | rawData[3]) * 0.1f;
            } else {
                rawData[2] &= ~(1 << 7);
                data.temp = (float)(((uint16_t)rawData[2] << 8) | rawData[3]) * -0.1f;
            }
        }
        if(sensor->type == DHT11) {
            data.hum = (float)rawData[0];
            data.temp = (float)rawData[2];
            //DHT11 производства ASAIR имеют дробную часть в температуре
            //А ещё температуру измеряет от -20 до +60 *С
            //Вот прикол, да?
            if(rawData[3] != 0) {
                //Проверка знака
                if(!(rawData[3] & (1 << 7))) {
                    //Добавление положительной дробной части
                    data.temp += rawData[3] * 0.1f;
                } else {
                    //А тут делаем отрицательное значение
                    rawData[3] &= ~(1 << 7);
                    data.temp += rawData[3] * 0.1f;
                    data.temp *= -1;
                }
            }
        }
    }

#if DHT_POLLING_CONTROL == 1
    sensor->lastHum = data.hum;
    sensor->lastTemp = data.temp;
#endif

    return data;
}