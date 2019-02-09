﻿/**
 * File:   model_jerryscript.c
 * Author: AWTK Develop Team
 * Brief:  jerryscript implemented model
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

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "jerryscript-port.h"
#include "jerryscript-ext/handler.h"
#include "mvvm/jerryscript/jsobj.h"
#include "mvvm/jerryscript/model_jerryscript.h"
#include "mvvm/jerryscript/value_converter_jerryscript.h"
#include "mvvm/jerryscript/value_validator_jerryscript.h"

static ret_t model_jerryscript_on_destroy(object_t* obj) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);

  str_reset(&(modeljs->temp));
  jerry_release_value(modeljs->jscode);
  jerry_release_value(modeljs->jsobj);

  return RET_OK;
}

static int32_t model_jerryscript_compare(object_t* obj, object_t* other) {
  return tk_str_cmp(obj->name, other->name);
}

static ret_t model_jerryscript_set_prop(object_t* obj, const char* name, const value_t* v) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  return jsobj_set_prop(modeljs->jsobj, name, v, &(modeljs->temp));
}

static ret_t model_jerryscript_get_prop(object_t* obj, const char* name, value_t* v) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  value_set_int(v, 0);
  if (jsobj_has_prop(modeljs->jsobj, name)) {
    return jsobj_get_prop(modeljs->jsobj, name, v, &(modeljs->temp));
  }

  return RET_NOT_FOUND;
}

static bool_t model_jerryscript_can_exec(object_t* obj, const char* name, const char* args) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL, RET_BAD_PARAMS);

  return jsobj_can_exec(modeljs->jsobj, name, args);
}

static ret_t model_jerryscript_exec(object_t* obj, const char* name, const char* args) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL, RET_BAD_PARAMS);

  return jsobj_exec(modeljs->jsobj, name, args);
}

#define STR_NATIVE_MODEL "nativeModel"

jerry_value_t wrap_notify_props_changed(const jerry_value_t func_obj_val,
                                        const jerry_value_t this_p, const jerry_value_t args_p[],
                                        const jerry_length_t args_cnt) {
  object_t* obj = jsobj_get_prop_object(this_p, STR_NATIVE_MODEL);

  return jerry_create_number(object_notify_changed(obj));
}

static const object_vtable_t s_model_jerryscript_obj_vtable = {
    .type = "model_jerryscript",
    .desc = "model_jerryscript",
    .size = sizeof(model_jerryscript_t),
    .is_collection = FALSE,
    .on_destroy = model_jerryscript_on_destroy,

    .compare = model_jerryscript_compare,
    .get_prop = model_jerryscript_get_prop,
    .set_prop = model_jerryscript_set_prop,
    .can_exec = model_jerryscript_can_exec,
    .exec = model_jerryscript_exec};

ret_t model_jerryscript_init(void) {
  jerry_init(JERRY_INIT_EMPTY);

  jerryx_handler_register_global((const jerry_char_t*)"print", jerryx_handler_print);

  return RET_OK;
}

ret_t model_jerryscript_deinit(void) {
  jerry_cleanup();

  return RET_OK;
}

static ret_t model_jerryscript_on_will_mount(model_t* model, navigator_request_t* req) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(model);
  jerry_value_t jsargs = jerry_value_from_navigator_request(req);

  return jsobj_exec_ex(modeljs->jsobj, "onWillMount", jsargs);
}

static ret_t model_jerryscript_on_mount(model_t* model) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(model);

  return jsobj_exec(modeljs->jsobj, "onMount", NULL);
}

static ret_t model_jerryscript_on_will_unmount(model_t* model) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(model);

  return jsobj_exec(modeljs->jsobj, "onWillUnmount", NULL);
}

static ret_t model_jerryscript_on_unmount(model_t* model) {
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(model);

  return jsobj_exec(modeljs->jsobj, "onUnmount", NULL);
}

const static model_vtable_t s_model_jerryscript_model_vtable = {
    .on_will_mount = model_jerryscript_on_will_mount,
    .on_mount = model_jerryscript_on_mount,
    .on_will_unmount = model_jerryscript_on_will_unmount,
    .on_unmount = model_jerryscript_on_unmount};

model_t* model_jerryscript_create(const char* name, const char* code, uint32_t code_size) {
  jerry_value_t jsret = 0;
  object_t* obj = NULL;
  model_jerryscript_t* modeljs = NULL;
  return_value_if_fail(name != NULL && code != NULL && code_size > 0, NULL);

  obj = object_create(&s_model_jerryscript_obj_vtable);
  modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL, NULL);

  object_set_name(obj, name);
  modeljs->jscode = jerry_parse((const jerry_char_t*)name, strlen(name), (const jerry_char_t*)code,
                                code_size, JERRY_PARSE_NO_OPTS);

  goto_error_if_fail(jerry_value_check(modeljs->jscode) == RET_OK);

  jsret = jerry_run(modeljs->jscode);
  goto_error_if_fail(jerry_value_check(jsret) == RET_OK);

  modeljs->jsobj = jsobj_get_model(name);
  goto_error_if_fail(jerry_value_check(modeljs->jsobj) == RET_OK);

  str_init(&(modeljs->temp), 0);
  jsobj_set_prop_object(modeljs->jsobj, STR_NATIVE_MODEL, OBJECT(modeljs));
  jsobj_set_prop_func(modeljs->jsobj, "notifyPropsChanged", wrap_notify_props_changed);

  jerry_release_value(jsret);
  MODEL(obj)->vt = &s_model_jerryscript_model_vtable;

  return MODEL(obj);
error:
  object_unref(obj);

  return NULL;
}
