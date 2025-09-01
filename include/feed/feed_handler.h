#pragma once
#include "core/types.h"
#include "feed/blocking_queue.h"
#include "order_book/order_book.h"
#include <atomic>
#include <thread>

namespace md {

// 简单的发布器接口（这里直接打印；以后可换 ZeroMQ/Redis/文件等）
class Publisher {
public:
  explicit Publisher(BlockingQueue<core::Event>& bus) : bus_(bus) {}
  void start();
  void stop() { running_ = false; }
  orderbook::OrderBook& book() { return book_; } // 方便 main 读取指标

private:
  BlockingQueue<core::Event>& bus_;
  std::atomic<bool> running_{true};
  std::thread th_;
  orderbook::OrderBook book_;
};

// 从 Parquet（经 Python）读取并往队列推事件
class ParquetSource {
public:
  ParquetSource(const std::string& parquet_path, BlockingQueue<core::Event>& bus)
    : path_(parquet_path), bus_(bus) {}
  void start();          // 启动子进程（python）并读取其 CSV 行
  void join();           // 等待结束
private:
  std::string path_;
  BlockingQueue<core::Event>& bus_;
  std::thread th_;
};

} // namespace md
