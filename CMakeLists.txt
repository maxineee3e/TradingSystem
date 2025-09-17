cmake_minimum_required(VERSION 3.16)
project(TradingSystem CXX)

# ---- C++ 标准 & 编译数据库 ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- 线程库 ----
find_package(Threads REQUIRED)

# ---- 源文件分组（按你仓库当前结构）----
set(FEED_SOURCES
    src/feed/feed_handler.cpp
    src/feed/message_parser.cpp
    src/feed/socket_client.cpp
)

set(ORDER_BOOK_SOURCES
    src/order_book/order_book.cpp
    src/databento_source.cpp   # 若暂时未实现，可先注释掉
)

# ---- 可执行程序 ----
add_executable(md_engine
    src/main.cpp
    ${FEED_SOURCES}
    ${ORDER_BOOK_SOURCES}
)

# ---- 头文件搜索路径 ----
target_include_directories(md_engine PRIVATE
    ${CMAKE_SOURCE_DIR}/include            # 你的工程头，如 core/, feed/, order_book/
    ${CMAKE_SOURCE_DIR}/databento/include  # databento-cpp 的头
    /opt/homebrew/include 
)

# ---- 链接库 ----
target_link_libraries(md_engine PRIVATE
    Threads::Threads
)

# ---- 推荐的告警（可选）----
if (MSVC)
    target_compile_options(md_engine PRIVATE /W4)
else()
    target_compile_options(md_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- 可选：再建一个独立的测试可执行（如果你将来要用 main_databento.cpp）----
# add_executable(player
#     src/order_book/main_databento.cpp
#     ${FEED_SOURCES}
#     ${ORDER_BOOK_SOURCES}
# )
# target_include_directories(player PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/databento/include
# )
# target_link_libraries(player PRIVATE Threads::Threads)
