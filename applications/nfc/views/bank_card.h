#pragma once
#include <stdint.h>
#include <gui/view.h>
#include <gui/modules/widget.h>

typedef struct BankCard BankCard;

BankCard* bank_card_alloc();

void bank_card_free(BankCard* bank_card);

void bank_card_clear(BankCard* bank_card);

View* bank_card_get_view(BankCard* bank_card);

void bank_card_set_back_callback(BankCard* bank_card, ButtonCallback callback, void* context);

void bank_card_set_name(BankCard* bank_card, char* name);

void bank_card_set_number(BankCard* bank_card, uint8_t* number);

void bank_card_set_exp_date(BankCard* bank_card, uint8_t mon, uint8_t year);

void bank_card_set_country_name(BankCard* bank_card, uint16_t country_code);

void bank_card_set_currency_name(BankCard* bank_card, uint16_t currency_code);
