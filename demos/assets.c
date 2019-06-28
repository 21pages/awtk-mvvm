#include "awtk.h"
#include "base/assets_manager.h"
#ifndef WITH_FS_RES
#include "assets/inc/strings/en_US.data"
#include "assets/inc/strings/zh_CN.data"
#include "assets/inc/styles/default.data"
#include "assets/inc/styles/dialog.data"
#include "assets/inc/styles/dialog_confirm.data"
#include "assets/inc/styles/dialog_info.data"
#include "assets/inc/styles/dialog_toast.data"
#include "assets/inc/styles/dialog_warn.data"
#include "assets/inc/styles/main.data"
#include "assets/inc/styles/window1.data"
#include "assets/inc/ui/books.data"
#include "assets/inc/ui/calculator.data"
#include "assets/inc/ui/custom_handlers.data"
#include "assets/inc/ui/demo9_main.data"
#include "assets/inc/ui/home.data"
#include "assets/inc/ui/humidity.data"
#include "assets/inc/ui/js_address.data"
#include "assets/inc/ui/js_books.data"
#include "assets/inc/ui/js_calculator.data"
#include "assets/inc/ui/js_demo9_main.data"
#include "assets/inc/ui/js_home.data"
#include "assets/inc/ui/js_humidity.data"
#include "assets/inc/ui/js_room_settings.data"
#include "assets/inc/ui/js_shape.data"
#include "assets/inc/ui/js_temperature1.data"
#include "assets/inc/ui/js_temperature10.data"
#include "assets/inc/ui/js_temperature14.data"
#include "assets/inc/ui/js_temperature2.data"
#include "assets/inc/ui/js_temperature3.data"
#include "assets/inc/ui/js_temperature4.data"
#include "assets/inc/ui/js_temperature5.data"
#include "assets/inc/ui/js_temperature6.data"
#include "assets/inc/ui/js_temperature9.data"
#include "assets/inc/ui/js_temperature_humidity.data"
#include "assets/inc/ui/room_settings.data"
#include "assets/inc/ui/shape.data"
#include "assets/inc/ui/temperature1.data"
#include "assets/inc/ui/temperature10.data"
#include "assets/inc/ui/temperature14.data"
#include "assets/inc/ui/temperature2.data"
#include "assets/inc/ui/temperature3.data"
#include "assets/inc/ui/temperature4.data"
#include "assets/inc/ui/temperature5.data"
#include "assets/inc/ui/temperature6.data"
#include "assets/inc/ui/temperature9.data"
#include "assets/inc/ui/temperature_humidity.data"
#ifdef WITH_STB_IMAGE
#include "assets/inc/images/arrow_down_n.res"
#include "assets/inc/images/arrow_down_o.res"
#include "assets/inc/images/arrow_down_p.res"
#include "assets/inc/images/arrow_left_n.res"
#include "assets/inc/images/arrow_left_o.res"
#include "assets/inc/images/arrow_left_p.res"
#include "assets/inc/images/arrow_right_n.res"
#include "assets/inc/images/arrow_right_o.res"
#include "assets/inc/images/arrow_right_p.res"
#include "assets/inc/images/arrow_up_n.res"
#include "assets/inc/images/arrow_up_o.res"
#include "assets/inc/images/arrow_up_p.res"
#include "assets/inc/images/check.res"
#include "assets/inc/images/checked.res"
#include "assets/inc/images/earth.res"
#include "assets/inc/images/edit_clear_n.res"
#include "assets/inc/images/edit_clear_o.res"
#include "assets/inc/images/edit_clear_p.res"
#include "assets/inc/images/empty.res"
#include "assets/inc/images/info.res"
#include "assets/inc/images/question.res"
#include "assets/inc/images/radio_checked.res"
#include "assets/inc/images/radio_unchecked.res"
#include "assets/inc/images/unchecked.res"
#else
#include "assets/inc/images/arrow_down_n.data"
#include "assets/inc/images/arrow_down_o.data"
#include "assets/inc/images/arrow_down_p.data"
#include "assets/inc/images/arrow_left_n.data"
#include "assets/inc/images/arrow_left_o.data"
#include "assets/inc/images/arrow_left_p.data"
#include "assets/inc/images/arrow_right_n.data"
#include "assets/inc/images/arrow_right_o.data"
#include "assets/inc/images/arrow_right_p.data"
#include "assets/inc/images/arrow_up_n.data"
#include "assets/inc/images/arrow_up_o.data"
#include "assets/inc/images/arrow_up_p.data"
#include "assets/inc/images/check.data"
#include "assets/inc/images/checked.data"
#include "assets/inc/images/earth.data"
#include "assets/inc/images/edit_clear_n.data"
#include "assets/inc/images/edit_clear_o.data"
#include "assets/inc/images/edit_clear_p.data"
#include "assets/inc/images/empty.data"
#include "assets/inc/images/info.data"
#include "assets/inc/images/question.data"
#include "assets/inc/images/radio_checked.data"
#include "assets/inc/images/radio_unchecked.data"
#include "assets/inc/images/unchecked.data"
#endif/*WITH_STB_IMAGE*/
#ifdef WITH_VGCANVAS
#endif/*WITH_VGCANVAS*/
#if defined(WITH_STB_FONT) || defined(WITH_FT_FONT)
#include "assets/inc/fonts/default.res"
#else/*WITH_STB_FONT or WITH_FT_FONT*/
#endif/*WITH_STB_FONT or WITH_FT_FONT*/
#endif/*WITH_FS_RES*/

ret_t assets_init(void) {
  assets_manager_t* rm = assets_manager();

#ifdef WITH_FS_RES
  assets_manager_preload(rm, ASSET_TYPE_FONT, "default");
  assets_manager_preload(rm, ASSET_TYPE_STYLE, "default");
#else
  assets_manager_add(rm, image_arrow_down_n);
  assets_manager_add(rm, image_arrow_down_o);
  assets_manager_add(rm, image_arrow_down_p);
  assets_manager_add(rm, image_arrow_left_n);
  assets_manager_add(rm, image_arrow_left_o);
  assets_manager_add(rm, image_arrow_left_p);
  assets_manager_add(rm, image_arrow_right_n);
  assets_manager_add(rm, image_arrow_right_o);
  assets_manager_add(rm, image_arrow_right_p);
  assets_manager_add(rm, image_arrow_up_n);
  assets_manager_add(rm, image_arrow_up_o);
  assets_manager_add(rm, image_arrow_up_p);
  assets_manager_add(rm, image_check);
  assets_manager_add(rm, image_checked);
  assets_manager_add(rm, image_earth);
  assets_manager_add(rm, image_edit_clear_n);
  assets_manager_add(rm, image_edit_clear_o);
  assets_manager_add(rm, image_edit_clear_p);
  assets_manager_add(rm, image_empty);
  assets_manager_add(rm, image_info);
  assets_manager_add(rm, image_question);
  assets_manager_add(rm, image_radio_checked);
  assets_manager_add(rm, image_radio_unchecked);
  assets_manager_add(rm, image_unchecked);
  assets_manager_add(rm, strings_en_US);
  assets_manager_add(rm, strings_zh_CN);
  assets_manager_add(rm, style_default);
  assets_manager_add(rm, style_dialog);
  assets_manager_add(rm, style_dialog_confirm);
  assets_manager_add(rm, style_dialog_info);
  assets_manager_add(rm, style_dialog_toast);
  assets_manager_add(rm, style_dialog_warn);
  assets_manager_add(rm, style_main);
  assets_manager_add(rm, style_window1);
  assets_manager_add(rm, ui_books);
  assets_manager_add(rm, ui_calculator);
  assets_manager_add(rm, ui_custom_handlers);
  assets_manager_add(rm, ui_demo9_main);
  assets_manager_add(rm, ui_home);
  assets_manager_add(rm, ui_humidity);
  assets_manager_add(rm, ui_js_address);
  assets_manager_add(rm, ui_js_books);
  assets_manager_add(rm, ui_js_calculator);
  assets_manager_add(rm, ui_js_demo9_main);
  assets_manager_add(rm, ui_js_home);
  assets_manager_add(rm, ui_js_humidity);
  assets_manager_add(rm, ui_js_room_settings);
  assets_manager_add(rm, ui_js_shape);
  assets_manager_add(rm, ui_js_temperature1);
  assets_manager_add(rm, ui_js_temperature10);
  assets_manager_add(rm, ui_js_temperature14);
  assets_manager_add(rm, ui_js_temperature2);
  assets_manager_add(rm, ui_js_temperature3);
  assets_manager_add(rm, ui_js_temperature4);
  assets_manager_add(rm, ui_js_temperature5);
  assets_manager_add(rm, ui_js_temperature6);
  assets_manager_add(rm, ui_js_temperature9);
  assets_manager_add(rm, ui_js_temperature_humidity);
  assets_manager_add(rm, ui_room_settings);
  assets_manager_add(rm, ui_shape);
  assets_manager_add(rm, ui_temperature1);
  assets_manager_add(rm, ui_temperature10);
  assets_manager_add(rm, ui_temperature14);
  assets_manager_add(rm, ui_temperature2);
  assets_manager_add(rm, ui_temperature3);
  assets_manager_add(rm, ui_temperature4);
  assets_manager_add(rm, ui_temperature5);
  assets_manager_add(rm, ui_temperature6);
  assets_manager_add(rm, ui_temperature9);
  assets_manager_add(rm, ui_temperature_humidity);
#ifdef WITH_VGCANVAS
#endif/*WITH_VGCANVAS*/
#endif

  tk_init_assets();
  return RET_OK;
}
