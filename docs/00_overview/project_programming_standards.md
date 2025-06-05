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
### 3. 文件夹的文件存放
对于需要其他程序引用的库，拆分为 include与src文件夹
对于可执行程序，如守护进程，不拆分include与src，相关文件
全部放在src文件夹下

### 4. 注释
brief写在头文件中，相关的internal技术细节可写在cpp文件中
