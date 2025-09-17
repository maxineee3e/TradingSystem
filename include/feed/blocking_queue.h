#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <utility>

namespace md {

template <typename T>
class BlockingQueue {
public:
  // push: 线程安全入队
  void push(T value) {
    {
      std::lock_guard<std::mutex> lk(m_);
      q_.push(std::move(value));
    }
    cv_.notify_one();
  }

  // try_pop: 带超时的弹出；成功返回 true，并把元素写入 out
  bool try_pop(T& out, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lk(m_);
    if (!cv_.wait_for(lk, timeout, [&]{ return !q_.empty(); }))
      return false;
    out = std::move(q_.front());
    q_.pop();
    return true;
  }

  // （可选）阻塞式弹出
  void pop(T& out) {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [&]{ return !q_.empty(); });
    out = std::move(q_.front());
    q_.pop();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(m_);
    return q_.empty();
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lk(m_);
    return q_.size();
  }

private:
  mutable std::mutex m_;
  std::condition_variable cv_;
  std::queue<T> q_;
};

} // namespace md
