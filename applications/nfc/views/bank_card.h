#pragma once
#include <stdint.h>
#include <gui/view.h>

typedef struct BankCard BankCard;

typedef void (*BankCardBackCallback)(void);

BankCard* bank_card_alloc();

void bank_card_free(BankCard* bank_card);

void bank_card_clear(BankCard* bank_card);

View* bank_card_get_view(BankCard* bank_card);

void bank_card_set_name(BankCard* bank_card, char* name);

void bank_card_set_number(BankCard* bank_card, uint8_t* number);

void bank_card_set_exp_date(BankCard* bank_card, uint8_t mon, uint8_t year);

void bank_card_set_cardholder_name(BankCard* bank_card, char* name);
