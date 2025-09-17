#pragma once

// 编译器检测
#if defined(_MSC_VER)
  #define DATABENTO_CXX_COMPILER_MSVC 1
#elif defined(__clang__)
  #define DATABENTO_CXX_COMPILER_CLANG 1
#elif defined(__GNUC__)
  #define DATABENTO_CXX_COMPILER_GCC 1
#endif

// 系统检测
#if defined(__APPLE__)
  #define DATABENTO_SYSTEM_DARWIN 1
#elif defined(__linux__)
  #define DATABENTO_SYSTEM_LINUX 1
#endif
