#include "feed/blocking_queue.h"
#include "feed/feed_handler.h"
#include "feed/publisher.h"
#include "core/types.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  // 简单读参：--py 与 --parquet
  std::string py_script;
  std::string parquet_path;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--py" && i + 1 < argc)        py_script    = argv[++i];
    else if (a == "--parquet" && i + 1 < argc) parquet_path = argv[++i];
  }
  if (py_script.empty() || parquet_path.empty()) {
    std::cerr << "Usage: md_engine --py <python_script> --parquet <file.parquet>\n";
    return 1;
  }

  // 事件总线
  md::BlockingQueue<core::Event> bus;

  // 发布器（消费者线程），内部维护 OrderBook 并输出指标
  md::Publisher publisher(bus);
  publisher.start();

  // 行情源（生产者线程）：用 SocketClient/FeedHandler 启动 python 子进程并读 stdout
  md::FeedHandler fh(bus);
  fh.startPythonPipe(py_script, parquet_path);  // 你在 FeedHandler 里实现的启动函数

  // 演示：运行一小段时间后退出（实际可按你的逻辑替换）
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // 收尾
  fh.stop();
  bus.stop();
  publisher.stop();

  return 0;
}
