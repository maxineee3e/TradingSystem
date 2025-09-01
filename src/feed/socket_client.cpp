#include "feed/socket_client.h"
#include "core/types.h"
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

namespace md {

void SocketClient::connect() {
  if (running_) return;
  running_ = true;
  th_ = std::thread(&SocketClient::runLoop, this);
}

void SocketClient::stop() {
  if (!running_) return;
  running_ = false;
  if (th_.joinable()) th_.join();
}

// 一个极简的撮合前行情模拟器：周期性地发出
//  Heartbeat / Add / Modify / Fill / Cancel 五类事件。
void SocketClient::runLoop() {
  using namespace std::chrono_literals;

  std::vector<std::string> live_ids;   // 已发过且仍存活的订单
  int id_counter = 1;
  bool flip_side = false;              // 交替买/卖

  while (running_) {
    ++seq_;

    // 1) 心跳
    if (seq_ % 10 == 0) {
      if (cb_) cb_(core::Heartbeat{});
      std::this_thread::sleep_for(20ms);
      continue;
    }

    // 2) 新增订单
    if (seq_ % 5 == 1 || seq_ % 5 == 2) {
      std::string id = "ORD" + std::to_string(id_counter++);
      flip_side = !flip_side;
      auto side = flip_side ? core::Side::Buy : core::Side::Sell;

      // 简单生成一个价格带（以最小价位单位）
      int64_t px = side == core::Side::Buy ? 10000 + (id_counter % 5) * 2
                                           : 10010 + (id_counter % 5) * 2;
      int64_t sz = 5 + (id_counter % 5);  // 5~9

      live_ids.push_back(id);
      if (cb_) cb_(core::Add{ core::OrderRef{id, side, px, sz} });
      std::this_thread::sleep_for(20ms);
      continue;
    }

    // 3) 修改尺寸
    if (seq_ % 5 == 3 && !live_ids.empty()) {
      const std::string& id = live_ids.back();
      // 随机一点的新尺寸：8~14
      int64_t new_sz = 8 + (static_cast<int>(seq_) % 7);
      if (cb_) cb_(core::Modify{ id, new_sz });
      std::this_thread::sleep_for(20ms);
      continue;
    }

    // 4) 成交或撤单
    if (!live_ids.empty()) {
      const std::string& id = live_ids.front();
      if (seq_ % 2 == 0) {
        // 成交部分（2~5）
        int64_t fill_sz = 2 + (static_cast<int>(seq_) % 4);
        if (cb_) cb_(core::Fill{ id, fill_sz });
      } else {
        // 直接撤单
        if (cb_) cb_(core::Cancel{ id });
        live_ids.erase(live_ids.begin());
      }
      std::this_thread::sleep_for(20ms);
      continue;
    }

    // 回退：若没有可操作订单，就发心跳
    if (cb_) cb_(core::Heartbeat{});
    std::this_thread::sleep_for(20ms);
  }
}

} // namespace md
