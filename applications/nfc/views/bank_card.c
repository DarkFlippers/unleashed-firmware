#include "bank_card.h"
#include "../gui_widget/gui_widget.h"
#include <m-string.h>

struct BankCard {
    GuiWidget* widget;
};

BankCard* bank_card_alloc() {
    BankCard* bank_card = furi_alloc(sizeof(BankCard));
    bank_card->widget = gui_widget_alloc();
    return bank_card;
}

void bank_card_free(BankCard* bank_card) {
    furi_assert(bank_card);
    gui_widget_free(bank_card->widget);
    free(bank_card);
}

View* bank_card_get_view(BankCard* bank_card) {
    furi_assert(bank_card);
    return gui_widget_get_view(bank_card->widget);
}

void bank_card_clear(BankCard* bank_card) {
    furi_assert(bank_card);
    gui_widget_clear(bank_card->widget);
}

void bank_card_set_name(BankCard* bank_card, char* name) {
    furi_assert(bank_card);
    furi_assert(name);
    gui_widget_add_string_element(
        bank_card->widget, 64, 6, AlignCenter, AlignTop, FontSecondary, name);
}

void bank_card_set_number(BankCard* bank_card, uint8_t* number) {
    furi_assert(bank_card);
    furi_assert(number);
    string_t num_str;
    string_init(num_str);
    for(uint8_t i = 0; i < 8; i += 2) {
        string_cat_printf(num_str, "%02X%02X ", number[i], number[i + 1]);
    }
    gui_widget_add_string_element(
        bank_card->widget, 25, 22, AlignLeft, AlignTop, FontSecondary, string_get_cstr(num_str));
    gui_widget_add_icon_element(bank_card->widget, 6, 20, &I_EMV_Chip_14x11);
    string_clear(num_str);
}

void bank_card_set_exp_date(BankCard* bank_card, uint8_t mon, uint16_t year) {
    furi_assert(bank_card);
    char exp_date_str[16];
    snprintf(exp_date_str, sizeof(exp_date_str), "Exp: %02d/%02d", mon, year % 100);
    gui_widget_add_string_element(
        bank_card->widget, 122, 54, AlignRight, AlignBottom, FontSecondary, exp_date_str);
}

void bank_card_set_cardholder_name(BankCard* bank_card, char* name) {
    furi_assert(bank_card);
    furi_assert(name);
    gui_widget_add_string_element(
        bank_card->widget, 6, 37, AlignLeft, AlignTop, FontSecondary, name);
}
