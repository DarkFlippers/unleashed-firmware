#pragma once

#include <gui/view.h>

typedef struct AboutView AboutView;

AboutView* about_view_alloc();

void about_view_free(AboutView* about_view);

View* about_view_get_view(AboutView* about_view);