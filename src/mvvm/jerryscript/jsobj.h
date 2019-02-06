﻿/**
 * File:   jsobj.h
 * Author: AWTK Develop Team
 * Brief:  jerryscript wrapper
 *
 * Copyright (c) 2019 - 2019  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2019-02-05 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_JSOBJ_H
#define TK_JSOBJ_H

#include "tkc/str.h"
#include "jerryscript.h"

BEGIN_C_DECLS

jerry_value_t jsobj_get_model(const char* name);

bool_t jsobj_has_prop(jerry_value_t obj, const char* name);
bool_t jsobj_has_prop_func(jerry_value_t obj, const char* name);

ret_t jsobj_get_prop(jerry_value_t obj, const char* name, value_t* v, str_t* temp);
ret_t jsobj_set_prop(jerry_value_t obj, const char* name, const value_t* v, str_t* temp);
ret_t jsobj_set_prop_pointer(jerry_value_t obj, const char* name, void* p);

ret_t jsobj_exec(jerry_value_t obj, const char* name, const char* args);
bool_t jsobj_can_exec(jerry_value_t obj, const char* name, const char* args);

ret_t jerry_value_to_value(jerry_value_t value, value_t* v, str_t* temp);
jerry_value_t jerry_value_from_value(const value_t* v, str_t* temp);
void* jerry_value_to_pointer(jerry_value_t value);
jerry_value_t jerry_value_from_pointer(void* ptr);
ret_t jerry_value_check(jerry_value_t value);

bool_t jsvalue_converter_exist(const char* name);
ret_t jsvalue_converter_to_view(const char* name, const value_t* from, value_t* to, str_t* temp);
ret_t jsvalue_converter_to_model(const char* name, const value_t* from, value_t* to, str_t* temp);

END_C_DECLS

#endif /*TK_JSOBJ_H*/
