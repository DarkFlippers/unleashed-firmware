#pragma once

#include <gui/view.h>

typedef struct SubghzAnalyze SubghzAnalyze;

SubghzAnalyze* subghz_analyze_alloc();

void subghz_analyze_free(SubghzAnalyze* subghz_analyze);

View* subghz_analyze_get_view(SubghzAnalyze* subghz_analyze);
