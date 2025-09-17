#pragma once
#include <string>
#include <atomic>
#include <thread>
#include "core/types.h"
#include "feed/blocking_queue.h"

namespace md {

class Publisher; // 前向声明

class FeedHandler {
public:
  FeedHandler(BlockingQueue<Event>& bus, Publisher& pub);

  // 历史数据回放（Databento）
  void startHistoricalFeed(const std::string& dataset,
                           const std::string& symbols,
                           const std::string& schema,
                           const std::string& start,
                           const std::string& end);
  void stop();

  // 开关：true = 直接更新 OrderBook；false = 推到队列由 Publisher 消费
  void enableImmediateMode(bool on) { immediate_mode_ = on; }

  // 供其它数据源复用的“单条记录”入口
  void onRecord(const std::string& action,
                const std::string& order_id,
                const std::string& side,
                int64_t price,
                int64_t size);
  void onRecordQueued(const std::string& action,
                      const std::string& order_id,
                      const std::string& side,
                      int64_t price,
                      int64_t size);

private:
  BlockingQueue<Event>& bus_;
  Publisher& publisher_;
  std::atomic<bool> running_{false};
  bool immediate_mode_{true};      // 默认立即处理
  std::thread th_;
};

} // namespace md
