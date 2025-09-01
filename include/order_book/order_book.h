#pragma once
#include "core/types.h"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <optional>
#include <algorithm>

namespace orderbook {

// 每个价位聚合信息
struct LevelData {
  int64_t total_size{0};
  int     order_count{0};
  std::unordered_set<std::string> order_ids; // 该价位的所有订单ID
};

// 全局侧（买/卖）统计
struct BookStats {
  int64_t total_size{0};
  int     total_count{0};
};

// 全深度 OrderBook
class OrderBook {
public:
  // 基本操作
  void addOrder(const std::string& id, core::Side side, int64_t px, int64_t sz);
  void modifyOrder(const std::string& id, int64_t new_size);
  void cancelOrder(const std::string& id);
  void fillOrder(const std::string& id, int64_t fill_size);

  // 事件入口（把 L3 事件派发到上面的方法）
  void onEvent(const core::Event& e);

  // Top-of-book
  struct Top {
    std::optional<std::pair<int64_t,int64_t>> best_bid; // (px, sz)
    std::optional<std::pair<int64_t,int64_t>> best_ask;
  };
  Top top() const;

  // 指标：点差/中价/队列不平衡/前N档深度聚合
  std::optional<int64_t> spread() const;           // ask - bid（价位单位）
  std::optional<double>  mid() const;              // (bid+ask)/2
  std::optional<double>  queueImbalance() const;   // bid_sz / (bid_sz + ask_sz)
  std::pair<int64_t,int64_t> depthN(int n) const;  // (sum_bid_sz, sum_ask_sz)

private:
  // 用 map 保持价格有序：bids 递增存，取 rbegin() 是最优买；asks 递增，取 begin() 是最优卖
  std::map<int64_t, LevelData> bids_;
  std::map<int64_t, LevelData> asks_;
  // 全局订单索引（id -> 侧/价/量）
  std::unordered_map<std::string, core::OrderRef> orders_;
  // 统计
  BookStats bid_stats_, ask_stats_;

  std::map<int64_t, LevelData>& sideMap(core::Side s)       { return (s==core::Side::Buy)? bids_ : asks_; }
  const std::map<int64_t, LevelData>& sideMap(core::Side s) const { return (s==core::Side::Buy)? bids_ : asks_; }
  BookStats& sideStats(core::Side s)       { return (s==core::Side::Buy)? bid_stats_ : ask_stats_; }
};

} // namespace md
