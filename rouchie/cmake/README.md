# CMake 模块说明

本目录包含 Rouchie 项目的 CMake 配置模块，将主 CMakeLists.txt 拆分为多个功能模块以提高可维护性。

## 文件结构

- **imgui.cmake** - ImGui 图形库配置
  - 查找 glfw3 依赖
  - 配置 ImGui 静态库
  - 处理 assert 宏隔离

- **imgui_font.cmake** - ImGui 字体工具配置
  - 配置独立的字体压缩工具
  - 隔离项目自定义 assert.h

- **rouchie_lib.cmake** - Rouchie 核心库配置
  - 包含所有模块源码（Module、Base、server、Muxer、Msg）
  - 配置编译选项和链接库
  - 处理跨平台链接差异
  - 添加 Windows 资源文件

- **app.cmake** - 主应用程序配置
  - 配置 app 可执行文件
  - 链接 Rouchie 库

- **subdirs.cmake** - 子项目配置
  - 添加 Tests 测试目录
  - 添加 imguidemo 示例目录
  - 可选的 rtspclient 目录

## 优势

1. **清晰的职责分离** - 每个模块专注于单一功能
2. **易于维护** - 修改某个功能时只需关注对应文件
3. **可复用性** - 模块可以在其他项目中复用
4. **可读性** - 主 CMakeLists.txt 简洁明了
5. **团队协作** - 不同开发者可以并行修改不同模块

## 使用方法

在主 CMakeLists.txt 中通过 `include()` 命令引入各个模块：

```cmake
include(cmake/imgui.cmake)
include(cmake/imgui_font.cmake)
include(cmake/rouchie_lib.cmake)
include(cmake/app.cmake)
include(cmake/subdirs.cmake)
```

## 扩展建议

如果未来需要添加新功能，可以创建新的 .cmake 文件，例如：
- `cmake/tests.cmake` - 测试配置
- `cmake/deployment.cmake` - 部署配置
- `cmake/compiler_options.cmake` - 编译器选项
