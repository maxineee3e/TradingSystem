cmake_minimum_required(VERSION 3.16)
project(TradingSystem CXX)

# ==== C++ 标准 & 编译告警 ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==== 头文件路径 ====
# 你的目录形如：
# include/
#   core/*.h
#   feed/*.h
#   order_book/*.h
# src/
#   feed/*.cpp
#   order_book/*.cpp
#   main.cpp
include_directories(${CMAKE_SOURCE_DIR}/include)

# ==== 源文件 ====
set(FEED_SOURCES
  src/feed/feed_handler.cpp
  src/feed/message_parser.cpp
  src/feed/socket_client.cpp
)

set(BOOK_SOURCES
  src/order_book/order_book.cpp
)

# ==== 可执行文件（主程序）====
add_executable(md_engine
  src/main.cpp
  ${FEED_SOURCES}
  ${BOOK_SOURCES}
)

# macOS/Linux 上用到 <thread> 时链接 pthread（有的系统会自动处理，这里显式一下更稳）
find_package(Threads REQUIRED)
target_link_libraries(md_engine PRIVATE Threads::Threads)

# ==== （可选）直接对接 Databento SDK 的示例可执行 ====
# 如果你以后要用 C++ SDK 直连 Databento，可以开启下面这段：
# ---------------------------------------------------------
# if(NOT EXISTS "${CMAKE_SOURCE_DIR}/databento/CMakeLists.txt")
#   message(STATUS "Databento SDK submodule not found; skipping md_databento target.")
# else()
#   add_subdirectory(databento)  # 生成目标 databento::databento
#   add_executable(md_databento
#     src/order_book/main_databento.cpp
#     src/order_book/order_book.cpp
#     src/feed/message_parser.cpp   # 如果还需要用到
#   )
#   target_link_libraries(md_databento PRIVATE databento::databento Threads::Threads)
# endif()
# ---------------------------------------------------------
