#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

// BlockingQueue 就是一个线程安全的消息队列，用来把行情生产者（FeedHandler）和策略消费者解耦，让数据在不同线程之间安全、稳定地传递。

namespace md {

template<class T>
class BlockingQueue {
 public:
  void push(T v) {
    { std::lock_guard<std::mutex> lk(m_); q_.push(std::move(v)); }
    cv_.notify_one();
  }
  std::optional<T> pop() {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [&]{ return stop_ || !q_.empty(); });
    if (stop_ && q_.empty()) return std::nullopt;
    T v = std::move(q_.front()); q_.pop();
    return v;
  }
  void stop() {
    { std::lock_guard<std::mutex> lk(m_); stop_ = true; }
    cv_.notify_all();
  }
 private:
  std::queue<T> q_;
  std::mutex m_;
  std::condition_variable cv_;
  bool stop_{false};
};

} // namespace md