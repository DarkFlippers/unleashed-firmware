/*
    Unitemp - Universal temperature reader
    Copyright (C) 2022  Victor Nikitchuk (https://github.com/quen0n)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "UnitempViews.h"
#include <gui/modules/variable_item_list.h>
#include <stdio.h>
#include <assets_icons.h>

uint32_t _prev_view_id;

#define VIEW_ID UnitempViewPopup

static void _popup_callback(void* context) {
    UNUSED(context);
    view_dispatcher_switch_to_view(app->view_dispatcher, _prev_view_id);
}

void unitemp_popup(const Icon* icon, char* header, char* message, uint32_t prev_view_id) {
    _prev_view_id = prev_view_id;
    popup_reset(app->popup);
    popup_set_icon(app->popup, 0, 64 - icon_get_height(icon), icon);
    popup_set_header(app->popup, header, 64, 6, AlignCenter, AlignCenter);
    popup_set_text(
        app->popup,
        message,
        (128 - icon_get_width(icon)) / 2 + icon_get_width(icon),
        32,
        AlignCenter,
        AlignCenter);

    popup_set_timeout(app->popup, 5000);
    popup_set_callback(app->popup, _popup_callback);
    popup_enable_timeout(app->popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_ID);
}