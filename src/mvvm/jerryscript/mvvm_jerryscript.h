﻿/**
 * File:   mvvm.h
 * Author: AWTK Develop Team
 * Brief:  mvvm global functions.
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
 * 2019-02-06 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_MVVM_JERRYSCRIPT_H
#define TK_MVVM_JERRYSCRIPT_H

#include "mvvm/jerryscript/model_jerryscript.h"
#include "mvvm/jerryscript/value_converter_jerryscript.h"

BEGIN_C_DECLS

/**
 * @method mvvm_jerryscript_init
 * 初始化MVVM jerryscript。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t mvvm_jerryscript_init(void);

/**
 * @method jerryscript_run
 * 执行js代码。
 * @param {const char*} code 代码。
 * @param {uint32_t} code_size 代码长度。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t jerryscript_run(const char* code, uint32_t code_size);

/**
 * @method mvvm_jerryscript_deinit
 * ~初始化MVVM jerryscript。
 *
 * @return {ret_t} 返回RET_OK表示成功，否则表示失败。
 */
ret_t mvvm_jerryscript_deinit(void);

END_C_DECLS

#endif /*TK_MVVM_JERRYSCRIPT_H*/