﻿/**
 * File:   binding_context_awtk.c
 * Author: AWTK Develop Team
 * Brief:  binding context awtk
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
 * 2019-01-30 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "tkc/str.h"
#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/int_str.h"
#include "tkc/darray.h"
#include "base/idle.h"
#include "base/widget.h"
#include "widgets/window.h"
#include "mvvm/base/data_binding.h"
#include "mvvm/base/model_factory.h"
#include "mvvm/base/binding_context.h"
#include "mvvm/base/command_binding.h"
#include "mvvm/base/view_model_normal.h"
#include "mvvm/base/binding_rule_parser.h"
#include "mvvm/awtk/binding_context_awtk.h"

static model_t* default_create_model(widget_t* widget, navigator_request_t* req);

static ret_t visit_data_binding_update_error_of(void* ctx, const void* data) {
  data_binding_t* rule = DATA_BINDING(data);
  data_binding_t* trigger_rule = DATA_BINDING(ctx);
  view_model_t* view_model = BINDING_RULE(trigger_rule)->view_model;

  if (tk_str_start_with(rule->path, DATA_BINDING_ERROR_OF)) {
    const char* path = rule->path + sizeof(DATA_BINDING_ERROR_OF) - 1;
    if (tk_str_eq(path, trigger_rule->path)) {
      widget_t* widget = WIDGET(BINDING_RULE(rule)->widget);
      widget_set_tr_text(widget, view_model->last_error.str);
    }
  }

  return RET_OK;
}

static ret_t binding_context_update_error_of(data_binding_t* rule) {
  binding_context_t* ctx = BINDING_RULE(rule)->binding_context;
  view_model_t* view_model = BINDING_RULE(rule)->view_model;
  return_value_if_fail(ctx != NULL && view_model != NULL, RET_BAD_PARAMS);

  if (view_model->last_error.size > 0) {
    darray_foreach(&(ctx->data_bindings), visit_data_binding_update_error_of, rule);
  }

  return RET_OK;
}

static ret_t on_widget_prop_change(void* ctx, event_t* e) {
  data_binding_t* rule = DATA_BINDING(ctx);
  prop_change_event_t* evt = prop_change_event_cast(e);

  data_binding_set_prop(rule, evt->value);
  binding_context_update_error_of(rule);

  return RET_OK;
}

static ret_t on_widget_value_change(void* ctx, event_t* e) {
  value_t v;
  widget_t* widget = WIDGET(e->target);
  data_binding_t* rule = DATA_BINDING(ctx);
  return_value_if_fail(widget_get_prop(widget, WIDGET_PROP_VALUE, &v) == RET_OK, RET_OK);

  data_binding_set_prop(rule, &v);
  binding_context_update_error_of(rule);

  return RET_OK;
}

static ret_t binding_context_bind_data(binding_context_t* ctx, const char* name,
                                       const char* value) {
  widget_t* widget = WIDGET(ctx->current_widget);
  data_binding_t* rule = (data_binding_t*)binding_rule_parse(name, value);
  return_value_if_fail(rule != NULL, RET_FAIL);

  BINDING_RULE(rule)->widget = widget;
  BINDING_RULE(rule)->binding_context = ctx;
  BINDING_RULE(rule)->view_model = ctx->current_view_model;
  goto_error_if_fail(darray_push(&(ctx->data_bindings), rule) == RET_OK);

  if (rule->trigger != UPDATE_WHEN_EXPLICIT) {
    if (rule->mode == BINDING_TWO_WAY || rule->mode == BINDING_ONE_WAY_TO_MODEL) {
      if (tk_str_eq(rule->prop, WIDGET_PROP_VALUE)) {
        if (rule->trigger == UPDATE_WHEN_CHANGING) {
          widget_on(widget, EVT_VALUE_CHANGING, on_widget_value_change, rule);
        }
        widget_on(widget, EVT_VALUE_CHANGED, on_widget_value_change, rule);
      } else {
        widget_on(widget, EVT_PROP_CHANGED, on_widget_prop_change, rule);
      }
    }
  }

  return RET_OK;
error:
  object_unref(OBJECT(rule));

  return RET_FAIL;
}

/*TODO: add more event*/
static int_str_t s_event_map[] = {{EVT_CLICK, "click"},
                                  {EVT_POINTER_DOWN, "pointer_down"},
                                  {EVT_POINTER_UP, "pointer_up"},
                                  {EVT_KEY_DOWN, "key_down"},
                                  {EVT_KEY_UP, "key_up"},
                                  {EVT_VALUE_CHANGED, "value_changed"},
                                  {EVT_NONE, NULL}};

static ret_t on_widget_event(void* ctx, event_t* e) {
  command_binding_t* rule = COMMAND_BINDING(ctx);

  if (command_binding_can_exec(rule)) {
    if (rule->update_model) {
      binding_context_update_to_model(BINDING_RULE(rule)->binding_context);
    }

    command_binding_exec(rule);

    if (rule->close_window) {
      widget_t* win = widget_get_window(BINDING_RULE(rule)->widget);
      window_close(win);
    }
  } else {
    log_debug("%s cannot exec\n", rule->command);
  }

  return RET_OK;
}

static ret_t binding_context_bind_command(binding_context_t* ctx, const char* name,
                                          const char* value) {
  int32_t event = 0;
  widget_t* widget = WIDGET(ctx->current_widget);
  command_binding_t* rule = (command_binding_t*)binding_rule_parse(name, value);
  return_value_if_fail(rule != NULL, RET_FAIL);

  BINDING_RULE(rule)->widget = widget;
  BINDING_RULE(rule)->binding_context = ctx;
  BINDING_RULE(rule)->view_model = ctx->current_view_model;
  goto_error_if_fail(darray_push(&(ctx->command_bindings), rule) == RET_OK);

  event = int_str_name(s_event_map, rule->event, EVT_NONE);
  if (event != EVT_NONE) {
    widget_on(widget, event, on_widget_event, rule);
  }

  return RET_OK;
error:
  object_unref(OBJECT(rule));

  return RET_FAIL;
}

static ret_t visit_bind_one_prop(void* ctx, const void* data) {
  binding_context_t* bctx = (binding_context_t*)(ctx);
  named_value_t* nv = (named_value_t*)data;

  if (tk_str_start_with(nv->name, BINDING_RULE_DATA_PREFIX)) {
    binding_context_bind_data(bctx, nv->name, value_str(&(nv->value)));
  } else if (tk_str_start_with(nv->name, BINDING_RULE_COMMAND_PREFIX)) {
    binding_context_bind_command(bctx, nv->name, value_str(&(nv->value)));
  }

  return RET_OK;
}

static ret_t on_view_model_prop_change(void* ctx, event_t* e) {
  binding_context_update_to_view((binding_context_t*)ctx);

  return RET_OK;
}

static view_model_t* binding_context_awtk_create_view_model(widget_t* widget,
                                                            navigator_request_t* req) {
  model_t* model = default_create_model(widget, req);
  view_model_t* view_model = view_model_normal_create(model);
  object_unref(OBJECT(model));

  return view_model;
}

static const char* widget_get_prop_vmodel(widget_t* widget) {
  value_t v;
  value_set_str(&v, NULL);

  return (widget_get_prop(widget, WIDGET_PROP_V_MODEL, &v) == RET_OK) ? value_str(&v) : NULL;
}

static ret_t binding_context_awtk_bind_widget(binding_context_t* ctx, widget_t* widget) {
  view_model_t* view_model = NULL;
  const char* vmodel = widget_get_prop_vmodel(widget);

  if (vmodel != NULL || ctx->current_view_model == NULL) {
    view_model = binding_context_awtk_create_view_model(widget, ctx->navigator_request);
    return_value_if_fail(view_model != NULL, RET_FAIL);

    model_on_will_mount(view_model->model, ctx->navigator_request);
    darray_push(&(ctx->view_models), view_model);
    darray_push(&(ctx->view_models_stack), view_model);
  } else if (ctx->view_models_stack.size > 0) {
    view_model = VIEW_MODEL(darray_tail(&(ctx->view_models_stack)));
  }

  ctx->current_view_model = view_model;
  if (view_model != NULL) {
    if (widget->custom_props != NULL) {
      ctx->current_widget = widget;
      object_foreach_prop(widget->custom_props, visit_bind_one_prop, ctx);
    }
  }

  WIDGET_FOR_EACH_CHILD_BEGIN(widget, iter, i)
  binding_context_awtk_bind_widget(ctx, iter);
  WIDGET_FOR_EACH_CHILD_END();

  if (vmodel != NULL) {
    model_on_mount(view_model->model);
    emitter_on(EMITTER(view_model), EVT_PROP_CHANGED, on_view_model_prop_change, ctx);
    emitter_on(EMITTER(view_model), EVT_PROPS_CHANGED, on_view_model_prop_change, ctx);
    darray_pop(&(ctx->view_models_stack));
  }

  return RET_OK;
}

static ret_t binding_context_awtk_bind(binding_context_t* ctx, void* widget) {
  return_value_if_fail(binding_context_awtk_bind_widget(ctx, WIDGET(widget)) == RET_OK, RET_FAIL);
  return_value_if_fail(binding_context_update_to_view(ctx) == RET_OK, RET_FAIL);
  ctx->bound = TRUE;

  return RET_OK;
}

static ret_t visit_data_binding_update_to_view(void* ctx, const void* data) {
  value_t v;
  data_binding_t* rule = DATA_BINDING(data);
  widget_t* widget = WIDGET(BINDING_RULE(rule)->widget);
  binding_context_t* bctx = BINDING_RULE(rule)->binding_context;

  if (tk_str_start_with(rule->path, DATA_BINDING_ERROR_OF)) {
    return RET_OK;
  }

  if ((rule->mode == BINDING_ONCE && !(bctx->bound)) || rule->mode == BINDING_ONE_WAY ||
      rule->mode == BINDING_TWO_WAY) {
    return_value_if_fail(data_binding_get_prop(rule, &v) == RET_OK, RET_OK);
    return_value_if_fail(widget_set_prop(widget, rule->prop, &v) == RET_OK, RET_OK);
  }

  return RET_OK;
}

static ret_t visit_command_binding(void* ctx, const void* data) {
  command_binding_t* rule = COMMAND_BINDING(data);
  widget_t* widget = WIDGET(BINDING_RULE(rule)->widget);
  bool_t can_exec = command_binding_can_exec(rule);

  widget_set_enable(widget, can_exec);

  return RET_OK;
}

static ret_t binding_context_awtk_update_to_view_sync(binding_context_t* ctx) {
  darray_foreach(&(ctx->data_bindings), visit_data_binding_update_to_view, ctx);
  darray_foreach(&(ctx->command_bindings), visit_command_binding, ctx);

  ctx->request_update_view = 0;

  return RET_OK;
}

static ret_t idle_update_to_view(const idle_info_t* info) {
  binding_context_t* ctx = BINDING_CONTEXT(info->ctx);
  return_value_if_fail(ctx != NULL, RET_BAD_PARAMS);

  binding_context_awtk_update_to_view_sync(ctx);

  return RET_OK;
}

static ret_t binding_context_awtk_update_to_view(binding_context_t* ctx) {
  return_value_if_fail(ctx != NULL, RET_BAD_PARAMS);

  if (ctx->bound) {
    if (!ctx->request_update_view) {
      idle_add(idle_update_to_view, ctx);
    }
    ctx->request_update_view++;
  } else {
    binding_context_awtk_update_to_view_sync(ctx);
  }

  return RET_OK;
}

static ret_t visit_data_binding_update_to_model(void* ctx, const void* data) {
  value_t v;
  data_binding_t* rule = DATA_BINDING(data);
  widget_t* widget = WIDGET(BINDING_RULE(rule)->widget);

  if (rule->trigger == UPDATE_WHEN_EXPLICIT) {
    if (rule->mode == BINDING_TWO_WAY || rule->mode == BINDING_ONE_WAY_TO_MODEL) {
      return_value_if_fail(widget_get_prop(widget, rule->prop, &v) == RET_OK, RET_OK);
      return_value_if_fail(data_binding_set_prop(rule, &v) == RET_OK, RET_OK);
    }
  }

  return RET_OK;
}

static ret_t binding_context_awtk_update_to_model(binding_context_t* ctx) {
  return_value_if_fail(ctx != NULL, RET_BAD_PARAMS);

  darray_foreach(&(ctx->data_bindings), visit_data_binding_update_to_model, ctx);

  return RET_OK;
}

static ret_t binding_context_awtk_destroy(binding_context_t* ctx) {
  TKMEM_FREE(ctx);

  return RET_OK;
}

static const binding_context_vtable_t s_binding_context_vtable = {
    .update_to_view = binding_context_awtk_update_to_view,
    .update_to_model = binding_context_awtk_update_to_model,
    .destroy = binding_context_awtk_destroy};

binding_context_t* binding_context_awtk_create(navigator_request_t* req) {
  binding_context_t* ctx = TKMEM_ZALLOC(binding_context_t);
  return_value_if_fail(ctx != NULL, NULL);

  ctx->vt = &s_binding_context_vtable;
  if (binding_context_init(ctx, req) != RET_OK) {
    binding_context_destroy(ctx);
    ctx = NULL;
  }

  return ctx;
}

static ret_t binding_context_on_widget_destroy(void* ctx, event_t* e) {
  binding_context_destroy((binding_context_t*)(ctx));

  return RET_REMOVE;
}

ret_t binding_context_bind_for_window(widget_t* widget, navigator_request_t* req) {
  binding_context_t* ctx = NULL;
  return_value_if_fail(widget != NULL, RET_BAD_PARAMS);

  if (req != NULL) {
    object_set_prop_pointer(OBJECT(req), NAVIGATOR_ARG_VIEW, widget);
  }

  ctx = binding_context_awtk_create(req);
  return_value_if_fail(ctx != NULL, RET_BAD_PARAMS);

  ctx->widget = widget;
  goto_error_if_fail(binding_context_awtk_bind(ctx, widget) == RET_OK);
  widget_on(widget, EVT_DESTROY, binding_context_on_widget_destroy, ctx);

  return RET_OK;
error:
  binding_context_destroy(ctx);

  return RET_FAIL;
}

static ret_t model_on_window_close(void* ctx, event_t* e) {
  model_on_will_unmount(MODEL(ctx));

  return RET_OK;
}

static ret_t model_on_window_destroy(void* ctx, event_t* e) {
  model_on_unmount(MODEL(ctx));

  return RET_OK;
}

static model_t* default_create_model(widget_t* widget, navigator_request_t* req) {
  model_t* model = NULL;
  widget_t* win = widget_get_window(widget);
  const char* vmodel = widget_get_prop_vmodel(widget);

  if (vmodel != NULL) {
    char name[TK_NAME_LEN + 1];
    char* ext_name = NULL;
    tk_strncpy(name, vmodel, TK_NAME_LEN);

    if (req != NULL) {
      object_set_prop_pointer(OBJECT(req), NAVIGATOR_ARG_VIEW, widget);
    }

    ext_name = strrchr(name, '.');
    if (ext_name != NULL) {
      *ext_name = '\0';
      model = model_factory_create_model(name, req);
      if (model == NULL) {
        *ext_name = '.';
        model = model_factory_create_model(ext_name, req);
      }
      return_value_if_fail(model != NULL, NULL);
    } else {
      model = model_factory_create_model(name, req);
      return_value_if_fail(model != NULL, NULL);
    }
  }

  if (model == NULL) {
    if (vmodel != NULL) {
      log_warn("%s not found model %s\n", __FUNCTION__, vmodel);
    }
    model = model_dummy_create(req);
  }

  widget_on(win, EVT_DESTROY, model_on_window_destroy, model);
  widget_on(win, EVT_WINDOW_CLOSE, model_on_window_close, model);

  return model;
}

ret_t awtk_open_window(navigator_request_t* req) {
  widget_t* win = NULL;
  return_value_if_fail(req != NULL && req->target != NULL, RET_BAD_PARAMS);

  win = window_open(req->target);
  return_value_if_fail(win != NULL, RET_NOT_FOUND);

  return binding_context_bind_for_window(win, req);
}
