#pragma once

#include "barcode_app.h"

int calculate_check_digit(BarcodeData* barcode_data);
int calculate_ean_upc_check_digit(BarcodeData* barcode_data);
void ean_upc_loader(BarcodeData* barcode_data);
void upc_a_loader(BarcodeData* barcode_data);
void ean_8_loader(BarcodeData* barcode_data);
void ean_13_loader(BarcodeData* barcode_data);
void code_39_loader(BarcodeData* barcode_data);
void code_128_loader(BarcodeData* barcode_data);
void code_128c_loader(BarcodeData* barcode_data);
void codabar_loader(BarcodeData* barcode_data);
void barcode_loader(BarcodeData* barcode_data);
