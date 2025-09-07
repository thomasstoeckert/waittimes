#include "style.h"


GFont pwt_title_font;
GFont pwt_body_font;

void init_fonts(void)
{
    pwt_title_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CUSTOM_FONT_JERSEY_B_36));
    pwt_body_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CUSTOM_FONT_JERSEY_SOLID_18));
}