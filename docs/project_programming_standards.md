# 工程命名规范
### 1. 文件夹
    
采用小驼峰命名法，third_party已经采用下划线了，就不再更改了。
### 1. 文件
- .h/.cpp文件
    文件名： 大驼峰命名法 like：CodeSample.cpp
- 脚本文件（shell、python等）
    文件名： 下划线命名法 like： build_helper.sh
- json等配置文件
  文件名： 下划线命名法 like： project_compile_options.json
- markdown文档
    文件名： 下划线命名法 like：markdown_sample.md，除了README.md
 
### 2. CPP编码规范（修改自 google C++ style）
类名： 大驼峰命名法
namespace： 小驼峰命名法

C++文件统一采用 .h/.cpp后缀，不使用.cc后缀与.hpp后缀
C文件统一采用 .h/.c后缀

brief写在头文件中，相关的internal技术细节可写在cpp文件中
