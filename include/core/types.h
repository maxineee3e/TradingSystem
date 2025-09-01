#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <variant>

namespace core {

using Ts = std::chrono::system_clock::time_point;

// --- 逐笔订单事件（L3） ---
enum class Side { Buy, Sell, Unknown };

// 订单基础
struct OrderRef {
  std::string id;      // order_id（字符串更稳妥，交易所常带前缀）
  Side        side{Side::Unknown};
  int64_t     price{0}; // 用最小价位单位（tick/最小货币单位）
  int64_t     size{0};  // 剩余量（单位：合约/股/张）
};

// 事件类型
struct Add    { OrderRef o; };
struct Modify { std::string id; int64_t new_size{0}; };
struct Cancel { std::string id; };
struct Fill   { std::string id; int64_t fill_size{0}; }; // 成交导致数量减少（可能至 0）
struct Heartbeat {};

// 统一事件
using Event = std::variant<Add, Modify, Cancel, Fill, Heartbeat>;

inline const char* sideStr(Side s) {
  switch (s) { case Side::Buy: return "B"; case Side::Sell: return "S"; default: return "?"; }
}

}
