cmake_minimum_required(VERSION 3.24)
project(TradingSystem CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认构建类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# macOS: 自动设置 OpenSSL 路径（Homebrew）
if(APPLE AND NOT DEFINED OPENSSL_ROOT_DIR)
  execute_process(
    COMMAND brew --prefix openssl@3
    OUTPUT_VARIABLE _openssl_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(EXISTS "${_openssl_prefix}")
    set(OPENSSL_ROOT_DIR "${_openssl_prefix}" CACHE PATH "OpenSSL root (Homebrew)" FORCE)
  endif()
endif()

# 确认子模块存在
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/databento/CMakeLists.txt")
  message(FATAL_ERROR
    "Databento submodule is missing.\n"
    "Run:  git submodule update --init --recursive"
  )
endif()

# 引入 Databento C++ SDK（子模块）
add_subdirectory(databento)   # 定义目标：databento::databento

# 你的可执行文件
add_executable(download_example src/main.cpp)

# 连接 Databento 目标（自动带上 include 路径和依赖库）
target_link_libraries(download_example PRIVATE databento::databento)

# 可选：更友好点的编译告警
if(MSVC)
  target_compile_options(download_example PRIVATE /W4)
else()
  target_compile_options(download_example PRIVATE -Wall -Wextra -Wpedantic)
endif()
