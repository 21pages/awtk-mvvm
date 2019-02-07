# 本示例展示了MVVM 事件处理的基本用法(JS)。

界面上有一个label控件、一个slider控件和一个button控件，前两者都绑定到**温度(temp)**变量上，当slider拖动时，其修改模型temperature对象的value属性，temperature对象的value属性自动更新到label上。在temperature对象的value属性值有变化时，button才启用，点击button执行save命令，button重新被禁用。

```
  <label text="1" x="center" y="middle:-40" w="80%" h="40" v-data:text="{temp}"/>
  <slider x="center" y="middle" w="90%" h="20" v-data:value="{temp}"/>
  <button text="Save" x="center" y="middle:40" w="40%" h="40" v-on:click="{save}"/>
```

* v-data:text="{temp}" 的意思是把控件的text属性与模型的temp变量关联起来。

* v-data:value="{temp}" 的意思是把控件的value属性与模型的temp变量关联起来。

* v-on:click="{save}" 的意思是click事件触发时，执行save命令。




