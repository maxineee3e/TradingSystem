#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <variant>

namespace md {

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

// 原始字段（不一定要定义成结构体，也可函数参数直接传）
struct L3Message {
  std::string action;
  std::string order_id;
  std::string side;   // "B"/"A" 或 "B"/"S"
  int64_t     price{0};
  int64_t     size{0};
};

// 统一事件
using Event = std::variant<Add, Modify, Cancel, Fill, Heartbeat>;

inline const char* sideStr(Side s) {
  switch (s) { case Side::Buy: return "B"; case Side::Sell: return "S"; default: return "?"; }
}

inline Side parseSide(char c) {
  return (c == 'B' || c == 'b') ? Side::Buy :
         (c == 'A' || c == 'a' || c == 'S' || c == 's') ? Side::Sell :
         Side::Unknown;
}
inline Side parseSide(const std::string& s) { return s.empty() ? Side::Unknown : parseSide(s[0]); }

// 工厂：L3 → Event（用你的 Event 设计）
inline Event makeEvent(const L3Message& m) {
  if (m.action == "Add" || m.action == "ADD") {
    OrderRef o{ m.order_id, parseSide(m.side), m.price, m.size };
    return Add{ std::move(o) };
  }
  if (m.action == "Modify" || m.action == "MOD") {
    return Modify{ m.order_id, m.size /* new_size */ };
  }
  if (m.action == "Cancel" || m.action == "CXL") {
    return Cancel{ m.order_id };
  }
  if (m.action == "Fill" || m.action == "FILL") {
    return Fill{ m.order_id, m.size /* fill_size */ };
  }
  return Heartbeat{};
}
}
