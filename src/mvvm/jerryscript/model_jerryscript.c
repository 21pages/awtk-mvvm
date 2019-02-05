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
  jerry_value_t global_object;
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (jsobj_has_prop(modeljs->jsobj, name)) {
    return jsobj_set_prop(modeljs->jsobj, name, v);
  }

  global_object = jerry_get_global_object();
  if (jsobj_has_prop(global_object, name)) {
    ret_t ret = jsobj_set_prop(global_object, name, v);
    jerry_release_value(global_object);

    return ret;
  }

  return jsobj_set_prop(modeljs->jsobj, name, v);
}

static ret_t model_jerryscript_get_prop(object_t* obj, const char* name, value_t* v) {
  jerry_value_t global_object;
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL && v != NULL, RET_BAD_PARAMS);

  if (jsobj_has_prop(modeljs->jsobj, name)) {
    return jsobj_get_prop(modeljs->jsobj, name, v, &(modeljs->temp));
  }

  global_object = jerry_get_global_object();
  if (jsobj_has_prop(global_object, name)) {
    ret_t ret = jsobj_get_prop(global_object, name, v, &(modeljs->temp));
    jerry_release_value(global_object);

    return ret;
  }

  return RET_NOT_FOUND;
}

static bool_t model_jerryscript_can_exec(object_t* obj, const char* name, const char* args) {
  jerry_value_t global_object;
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL && args != NULL, RET_BAD_PARAMS);

  if (jsobj_can_exec(modeljs->jsobj, name, args)) {
    return TRUE;
  }

  global_object = jerry_get_global_object();
  if (jsobj_can_exec(global_object, name, args)) {
    jerry_release_value(global_object);
    return TRUE;
  }

  jerry_release_value(global_object);

  return FALSE;
}

static ret_t model_jerryscript_exec(object_t* obj, const char* name, const char* args) {
  ret_t ret = RET_OK;
  jerry_value_t global_object;
  model_jerryscript_t* modeljs = MODEL_JERRYSCRIPT(obj);
  return_value_if_fail(obj != NULL && name != NULL && args != NULL, RET_BAD_PARAMS);

  ret = jsobj_exec(modeljs->jsobj, name, args);
  if (ret != RET_NOT_IMPL) {
    return ret;
  }

  global_object = jerry_get_global_object();
  ret = jsobj_exec(global_object, name, args);
  if (ret != RET_NOT_IMPL) {
    return ret;
  }

  return RET_NOT_FOUND;
}

static const object_vtable_t s_model_jerryscript_vtable = {
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

model_t* model_jerryscript_create(const char* name, const char* code, uint32_t code_size) {
  jerry_value_t jsret;
  object_t* obj = NULL;
  model_jerryscript_t* modeljs = NULL;
  return_value_if_fail(name != NULL && code != NULL && code_size > 0, NULL);

  obj = object_create(&s_model_jerryscript_vtable);
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

  return MODEL(obj);
error:
  object_unref(obj);

  return NULL;
}
