const fs = require('fs')
const path = require('path')

class CodeGen {
  genToValue(type, name) {
    switch (type) {
      case 'int8_t':
      case 'int16_t':
      case 'int32_t':
      case 'int64_t':
      case 'uint8_t':
      case 'uint16_t':
      case 'uint32_t':
      case 'uint64_t':
      case 'float_t':
      case 'float':
      case 'double': {
        let typeName = type.replace(/_t$/, '');
        return `value_set_${typeName}(v, ${name});`;
      }
      case 'char*': {
        return `value_set_str(v, ${name});`;
      }
      default: {
        console.log(`not supported ${type} for ${name}`);
        process.exit(0);
      }
    }
  }

  genFromValue(clsName, type, name) {
    switch (type) {
      case 'int8_t':
      case 'int16_t':
      case 'int32_t':
      case 'int64_t':
      case 'uint8_t':
      case 'uint16_t':
      case 'uint32_t':
      case 'uint64_t':
      case 'float_t':
      case 'float':
      case 'double': {
        let typeName = type.replace(/_t$/, '');
        return `value_${typeName}(v)`;
      }
      case 'char*': {
        return `tk_str_copy(${clsName}->${name}, value_str(v))`;
      }
      default: {
        console.log(`not supported ${type} for ${name}`);
        process.exit(0);
      }
    }
  }

  genAssignValue(clsName, name, type) {
    if (type === 'char*') {
      return `tk_str_copy(${clsName}->${name}, ${name});`;
    } else {
      return `${clsName}->${name} = ${name};`;
    }
  }

  genHeader(json) {
    let propsDecl = '';
    let clsName = json.name;
    let clsNameUpper = clsName.toUpperCase();

    if (json.props && json.props.length) {
      propsDecl = json.props.map((prop) => {
        if (prop.synthesized) {
          return '';
        } else {
          return `  ${prop.type} ${prop.name};`;
        }
      }).join('\n');
    }
    let result =
      `
#ifndef TK_${clsNameUpper}_H
#define TK_${clsNameUpper}_H

#include "tkc/darray.h"
#include "mvvm/base/view_model_array.h"

BEGIN_C_DECLS

/**
 * @class ${clsName}_t
 *
 * ${json.desc}
 *
 */
typedef struct _${clsName}_t {
${propsDecl}
} ${clsName}_t;

/**
 * @class ${clsName}s_view_model_t
 *
 * view model of ${json.name}
 *
 */
typedef struct _${clsName}s_view_model_t {
  view_model_array_t view_model_array;

  /*model object*/
  darray_t ${clsName}s;
} ${clsName}s_view_model_t;

/**
 * @method ${clsName}s_view_model_create
 * 创建${clsName} view model对象。
 *
 * @annotation ["constructor"]
 * @param {navigator_request_t*} req 请求参数。
 *
 * @return {view_model_t} 返回view_model_t对象。
 */
view_model_t* ${clsName}s_view_model_create(navigator_request_t* req);

/*public for test*/

${clsName}_t* ${clsName}_create(void);

ret_t ${clsName}s_view_model_clear(view_model_t* view_model);
uint32_t ${clsName}s_view_model_size(view_model_t* view_model);
ret_t ${clsName}s_view_model_remove(view_model_t* view_model, uint32_t index);
ret_t ${clsName}s_view_model_add(view_model_t* view_model, ${clsName}_t* ${clsName});
${clsName}_t* ${clsName}s_view_model_get(view_model_t* view_model, uint32_t index);

END_C_DECLS

#endif /*TK_${clsNameUpper}_H*/
`
    return result;
  }

  genGetProps(json) {
    const clsName = json.name;
    const dispatch = json.props.map((prop, index) => {
      let str = '  ';
      const propName = prop.name;

      if (prop.private) {
        return str;
      }

      if (index === 0) {
        str += 'if (';
      } else {
        str += '} else if (';
      }

      str += `tk_str_eq("${propName}", name)) {\n`
      if (prop.getter || prop.synthesized) {
        str += `    ${prop.type} ${propName} = ${clsName}_get_${propName}(${clsName});\n`;
      } else {
        str += `    ${prop.type} ${propName} = ${clsName}->${propName};\n`;
      }
      str += '    ' + this.genToValue(prop.type, propName);
      return str;
    }).join('\n');

    const result =
      `
static ret_t ${clsName}s_view_model_get_prop(object_t* obj, const char* name, value_t* v) {
  uint32_t index = 0;
  ${clsName}_t* ${clsName} = NULL;
  view_model_t* vm = VIEW_MODEL(obj);

  if (tk_str_eq(VIEW_MODEL_PROP_ITEMS, name)) {
    value_set_int(v, ${clsName}s_view_model_size(VIEW_MODEL(obj)));

    return RET_OK;
  } else if (tk_str_eq(VIEW_MODEL_PROP_CURSOR, name)) {
    value_set_int(v, VIEW_MODEL_ARRAY(obj)->cursor);

    return RET_OK;
  }

  name = destruct_array_prop_name(name, &index);
  return_value_if_fail(name != NULL, RET_BAD_PARAMS);
  ${clsName} = ${clsName}s_view_model_get(vm, index);
  return_value_if_fail(${clsName} != NULL, RET_BAD_PARAMS);

${dispatch}
  } else if (tk_str_eq("style", name)) {
    value_set_str(v, index % 2 ? "odd" : "even");
  } else {
    return RET_NOT_FOUND;
  }
  
  return RET_OK;
}

`
    return result;
  }

  genPropFuncs(json) {
    const clsName = json.name;
    const result = json.props.map((prop, index) => {
      let str = '';
      const propName = prop.name;

      if (prop.private) {
        return str;
      }

      if (prop.synthesized) {
        let setter = typeof (prop.setter) === 'string' ? prop.setter : 'return RET_OK;';
        let getter = typeof (prop.getter) === 'string' ? prop.getter : 'return 0;';
        str =
          `
static ${prop.type} ${clsName}_get_${propName}(${clsName}_t* ${clsName}) {
  ${getter}
}

static ret_t ${clsName}_set_${propName}(${clsName}_t* ${clsName}, ${prop.type} value) {
  ${setter}
}

`;
      } else {
        if (prop.getter) {
          let defaultGetter = `return ${clsName}->${propName};`;
          let getter = typeof (prop.getter) === 'string' ? prop.getter : defaultGetter;
          str =
            `
static ${prop.type} ${clsName}_get_${propName}(${clsName}_t* ${clsName}) {
  ${getter}
}

`;
        }

        if (prop.setter) {
          let defaultSetter =
            `${this.genAssignValue(clsName, propName, prop.type)}

  return RET_OK;`

          let setter = typeof (prop.setter) === 'string' ? prop.setter : defaultSetter;
          str +=
            `
static ret_t ${clsName}_set_${propName}(${clsName}_t* ${clsName}, ${prop.type} ${prop.name}) {
  ${setter}
}

`;
        }
      }
      return str;
    }).join("");

    return result;
  }

  genSetProps(json) {
    const clsName = json.name;
    const dispatch = json.props.map((prop, index) => {
      let str = '  ';
      const propName = prop.name;
      if (prop.private) {
        return str;
      }

      if (index === 0) {
        str += 'if (';
      } else {
        str += '} else if (';
      }
      str += `tk_str_eq("${propName}", name)) {\n`
      if (prop.setter || prop.synthesized) {
        str += `    ${clsName}_set_${propName}(${clsName}, ${this.genFromValue(clsName, prop.type, prop.name)});`;
      } else {
        str += `    ${clsName}->${propName} = ${this.genFromValue(clsName, prop.type, prop.name)};`;
      }
      return str;
    }).join('\n');

    const result =
      `
static ret_t ${clsName}s_view_model_set_prop(object_t* obj, const char* name, const value_t* v) {
  uint32_t index = 0;
  ${clsName}_t* ${clsName} = NULL;
  view_model_t* vm = VIEW_MODEL(obj);

  if (tk_str_eq(VIEW_MODEL_PROP_CURSOR, name)) {
    view_model_array_set_cursor(vm, value_int(v));

    return RET_OK;
  }

  name = destruct_array_prop_name(name, &index);
  return_value_if_fail(name != NULL, RET_BAD_PARAMS);
  ${clsName} = ${clsName}s_view_model_get(vm, index);
  return_value_if_fail(${clsName} != NULL, RET_BAD_PARAMS);

${dispatch}
  } else {
    return RET_NOT_FOUND;
  }
  
  return RET_OK;
}

`
    return result;
  }

  genCmdFuncs(json) {
    const clsName = json.name;
    const result = json.cmds.map((cmd, index) => {
      let str = '';
      const cmdName = cmd.name;
      if (!cmd.canExec || typeof cmd.canExec === 'string') {
        let canExec = (typeof cmd.canExec === 'string') ? cmd.canExec : 'return TRUE;';
        str =
          `static bool_t ${clsName}_can_exec_${cmdName}(${clsName}_t* ${clsName}, const char* args) {
  ${canExec}
}\n\n`;
      }

      let impl = cmd.impl || '';
      str +=
        `static ret_t ${clsName}_${cmdName}(${clsName}_t* ${clsName}, const char* args) {
  ${impl}
  return RET_OBJECT_CHANGED;
}\n\n`;
      return str;
    }).join('\n');

    return result;
  }

  genExec(json) {
    const clsName = json.name;
    const dispatch = json.cmds.map((cmd, index) => {
      const cmdName = cmd.name;
      let str = '  } else if (';
      str += `tk_str_eq("${cmdName}", name)) {\n`
      str += `    return ${clsName}_${cmdName}(${clsName}, args);`;
      return str;
    }).join('\n');

    const result =
      `
static ret_t ${clsName}s_view_model_exec(object_t* obj, const char* name, const char* args) {
  uint32_t index = tk_atoi(args);
  view_model_t* vm = VIEW_MODEL(obj);
  ${clsName}_t* ${clsName} = NULL;

  if (tk_str_ieq(name, "add")) {
    ENSURE(${clsName}s_view_model_add(vm, ${clsName}_create()) == RET_OK);
    return RET_ITEMS_CHANGED;
  } else if (tk_str_ieq(name, "clear")) {
    ENSURE(${clsName}s_view_model_clear(vm) == RET_OK);
    return RET_ITEMS_CHANGED;
  }

  ${clsName} = ${clsName}s_view_model_get(vm, index);
  return_value_if_fail(${clsName} != NULL, RET_BAD_PARAMS);

  if (tk_str_ieq(name, "remove")) {
    ENSURE(${clsName}s_view_model_remove(vm, index) == RET_OK);
    return RET_ITEMS_CHANGED;
${dispatch}
  } else {
    return RET_NOT_FOUND;
  }
}
`
    return result;
  }

  genCanExec(json) {
    const clsName = json.name;
    const dispatch = json.cmds.map((cmd, index) => {
      const cmdName = cmd.name;
      let str = '  } else if (';
      str += `tk_str_eq("${cmdName}", name)) {\n`

      if (!cmd.canExec || typeof cmd.canExec === 'string') {
        str += `    return ${clsName}_can_exec_${cmdName}(${clsName}, args);`;
      } else {
        str += `    return TRUE;`;
      }
      return str;
    }).join('\n');

    const result =
      `
static bool_t ${clsName}s_view_model_can_exec(object_t* obj, const char* name, const char* args) {
  uint32_t index = tk_atoi(args);
  view_model_t* vm = VIEW_MODEL(obj);
  ${clsName}_t* ${clsName} = NULL;

  if (tk_str_ieq(name, "add")) {
    return TRUE;
  } else if (tk_str_ieq(name, "clear")) {
    return ${clsName}s_view_model_size(vm) > 0;
  }

  ${clsName} = ${clsName}s_view_model_get(vm, index);
  return_value_if_fail(${clsName} != NULL, FALSE);

  if (tk_str_ieq(name, "remove")) {
    return index < ${clsName}s_view_model_size(vm);
${dispatch}
  } else {
    return FALSE;
  }
}
`
    return result;
  }

  genVTable(json) {
    const clsName = json.name;
    const clsDesc = json.desc || clsName;
    let result =
      `
static ret_t ${clsName}s_view_model_on_destroy(object_t* obj) {
  ${clsName}s_view_model_t* vm = (${clsName}s_view_model_t*)(obj);
  return_value_if_fail(vm != NULL, RET_BAD_PARAMS);
  
  ${clsName}s_view_model_clear(VIEW_MODEL(obj));
  darray_deinit(&(vm->${clsName}s));

  return view_model_array_deinit(VIEW_MODEL(obj));
}

static const object_vtable_t s_${clsName}s_view_model_vtable = {
  .type = "${clsName}",
  .desc = "${clsDesc}",
  .is_collection = TRUE,
  .size = sizeof(${clsName}s_view_model_t),
  .exec = ${clsName}s_view_model_exec,
  .can_exec = ${clsName}s_view_model_can_exec,
  .get_prop = ${clsName}s_view_model_get_prop,
  .set_prop = ${clsName}s_view_model_set_prop,
  .on_destroy = ${clsName}s_view_model_on_destroy
};

view_model_t* ${clsName}s_view_model_create(navigator_request_t* req) {
  object_t* obj = object_create(&s_${clsName}s_view_model_vtable);
  view_model_t* vm = view_model_array_init(VIEW_MODEL(obj));
  ${clsName}s_view_model_t* ${clsName}_vm = (${clsName}s_view_model_t*)(vm);

  return_value_if_fail(vm != NULL, NULL);

  darray_init(&(${clsName}_vm->${clsName}s), 100, 
    (tk_destroy_t)${clsName}_destroy, (tk_compare_t)${clsName}_cmp);

  return vm;
}
`
    return result;
  }

  genContent(json) {
    const clsName = json.name;
    let result = `#include "tkc/mem.h"\n`;
    let cmp = json.cmp ? json.cmp : "/*TODO: */\n  return 0;"
    let init = json.init ? json.init : "/*TODO: */"
    let deinit = json.deinit ? json.deinit : "/*TODO: */"
    result += `#include "tkc/utils.h"\n`;
    result += `#include "mvvm/base/utils.h"\n`;
    result += `#include "${json.name}s.h"\n\n`;
    result +=
      `
/***************${clsName}***************/;

${clsName}_t* ${clsName}_create(void) {
  ${clsName}_t* ${clsName} = TKMEM_ZALLOC(${clsName}_t);
  return_value_if_fail(${clsName} != NULL, NULL);
${init}
  return ${clsName};
} 

int ${clsName}_cmp(${clsName}_t* a, ${clsName}_t* b) {
  return_value_if_fail(a != NULL && b != NULL, -1);
${cmp}
}

static ret_t ${clsName}_destroy(${clsName}_t* ${clsName}) {
${deinit}

  TKMEM_FREE(${clsName});

  return RET_OK;
}

`
    if (json.props && json.props.length) {
      result += this.genPropFuncs(json);
    }

    if (json.cmds && json.cmds.length) {
      result += this.genCmdFuncs(json);
    }

    result += `
/***************${clsName}s_view_model***************/

ret_t ${clsName}s_view_model_remove(view_model_t* view_model, uint32_t index) {
  ${clsName}s_view_model_t* ${clsName}_vm = (${clsName}s_view_model_t*)(view_model);
  return_value_if_fail(${clsName}_vm != NULL, RET_BAD_PARAMS);

  return darray_remove_index(&(${clsName}_vm->${clsName}s), index);
}

ret_t ${clsName}s_view_model_add(view_model_t* view_model, ${clsName}_t* ${clsName}) {
  ${clsName}s_view_model_t* ${clsName}_vm = (${clsName}s_view_model_t*)(view_model);
  return_value_if_fail(${clsName}_vm != NULL && ${clsName} != NULL, RET_BAD_PARAMS);

  return darray_push(&(${clsName}_vm->${clsName}s), ${clsName});
}

uint32_t ${clsName}s_view_model_size(view_model_t* view_model) {
  ${clsName}s_view_model_t* ${clsName}_vm = (${clsName}s_view_model_t*)(view_model);
  return_value_if_fail(${clsName}_vm != NULL, 0); 

  return ${clsName}_vm->${clsName}s.size;
}

ret_t ${clsName}s_view_model_clear(view_model_t* view_model) {
  ${clsName}s_view_model_t* ${clsName}_vm = (${clsName}s_view_model_t*)(view_model);
  return_value_if_fail(${clsName}_vm != NULL, 0); 

  return darray_clear(&(${clsName}_vm->${clsName}s));
}

${clsName}_t* ${clsName}s_view_model_get(view_model_t* view_model, uint32_t index) {
  ${clsName}s_view_model_t* ${clsName}_vm = (${clsName}s_view_model_t*)(view_model);
  return_value_if_fail(${clsName}_vm != NULL, 0); 

  return_value_if_fail(${clsName}_vm != NULL && index < ${clsName}s_view_model_size(view_model), NULL);

  return (${clsName}_t*)(${clsName}_vm->${clsName}s.elms[index]);
}
`
    result += this.genSetProps(json);
    result += this.genGetProps(json);

    result += this.genCanExec(json);
    result += this.genExec(json);

    result += this.genVTable(json);

    return result;
  }

  genJson(json) {
    let header = this.genHeader(json);
    let content = this.genContent(json);

    fs.writeFileSync(`${json.name}s.h`, header);
    fs.writeFileSync(`${json.name}s.c`, content);

    console.log(`output to ${json.name}s.h and ${json.name}s.c`);
  }

  genFile(filename) {
    this.genJson(JSON.parse(fs.readFileSync(filename).toString()));
  }

  static run(filename) {
    const gen = new CodeGen();
    gen.genFile(filename);
  }
}

if (process.argv.length < 3) {
  console.log(`Usage: node gen_vm.js idl.json`);
  process.exit(0);
}

CodeGen.run(process.argv[2]);
