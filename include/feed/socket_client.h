#pragma once
#include "core/types.h"   // 注意路径：统一走 core/types.h
#include <functional>
#include <atomic>
#include <thread>

namespace md {

// 极简 Socket 客户端（模拟源）：内部线程周期性产出 L3 事件
class SocketClient {
public:
  using Callback = std::function<void(const core::Event&)>;

  explicit SocketClient(Callback cb) : cb_(std::move(cb)) {}
  ~SocketClient() { stop(); }

  void connect();   // 启动模拟事件流（在 .cpp 的 runLoop 里生成 Add/Modify/Fill/Cancel/Heartbeat）
  void stop();      // 停止并 join 线程

private:
  void runLoop();

  Callback cb_;
  std::atomic<bool> running_{false};
  std::thread th_;
  uint64_t seq_{1};
};

} // namespace md
