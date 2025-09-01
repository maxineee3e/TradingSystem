#include "order_book/order_book.h"
#include <cassert>

namespace orderbook {

static void touch_level(LevelData& lv, int64_t d_size, int d_cnt) {
  lv.total_size += d_size;
  lv.order_count += d_cnt;
}

void OrderBook::addOrder(const std::string& id, core::Side side, int64_t px, int64_t sz) {
  if (sz <= 0 || side==core::Side::Unknown) return;
  auto& m   = sideMap(side);
  auto& st  = sideStats(side);
  auto& lv  = m[px]; // 不存在则默认构造
  lv.order_ids.insert(id);
  touch_level(lv, sz, 1);
  orders_[id] = core::OrderRef{id, side, px, sz};
  st.total_size  += sz;
  st.total_count += 1;
}

void OrderBook::modifyOrder(const std::string& id, int64_t new_size) {
  auto it = orders_.find(id);
  if (it==orders_.end()) return;
  auto& o = it->second;
  if (new_size == o.size) return;
  int64_t delta = new_size - o.size;

  auto& m  = sideMap(o.side);
  auto  lv_it = m.find(o.price);
  if (lv_it==m.end()) return;
  auto& lv = lv_it->second;

  lv.total_size += delta;
  if (new_size==0) {
    lv.order_ids.erase(id);
    lv.order_count -= 1;
  }
  if (lv.order_count==0) m.erase(lv_it);

  auto& st = sideStats(o.side);
  st.total_size += delta;
  if (new_size==0) st.total_count -= 1;

  if (new_size==0) orders_.erase(it);
  else o.size = new_size;
}

void OrderBook::cancelOrder(const std::string& id) {
  modifyOrder(id, 0);
}

void OrderBook::fillOrder(const std::string& id, int64_t fill_size) {
  auto it = orders_.find(id);
  if (it==orders_.end() || fill_size<=0) return;
  auto new_sz = std::max<int64_t>(0, it->second.size - fill_size);
  modifyOrder(id, new_sz);
}

void OrderBook::onEvent(const core::Event& e) {
  std::visit([&](auto&& ev){
    using T = std::decay_t<decltype(ev)>;
    if constexpr (std::is_same_v<T, core::Add>)       addOrder(ev.o.id, ev.o.side, ev.o.price, ev.o.size);
    else if constexpr (std::is_same_v<T, core::Modify>) modifyOrder(ev.id, ev.new_size);
    else if constexpr (std::is_same_v<T, core::Cancel>) cancelOrder(ev.id);
    else if constexpr (std::is_same_v<T, core::Fill>)   fillOrder(ev.id, ev.fill_size);
    else { /* Heartbeat：忽略 */ }
  }, e);
}

OrderBook::Top OrderBook::top() const {
  Top t;
  if (!bids_.empty()) t.best_bid = std::make_pair(bids_.rbegin()->first, bids_.rbegin()->second.total_size);
  if (!asks_.empty()) t.best_ask = std::make_pair(asks_.begin()->first, asks_.begin()->second.total_size);
  return t;
}

std::optional<int64_t> OrderBook::spread() const {
  if (bids_.empty() || asks_.empty()) return std::nullopt;
  return asks_.begin()->first - bids_.rbegin()->first;
}

std::optional<double> OrderBook::mid() const {
  if (bids_.empty() || asks_.empty()) return std::nullopt;
  auto bp = static_cast<double>(bids_.rbegin()->first);
  auto ap = static_cast<double>(asks_.begin()->first);
  return 0.5*(bp+ap);
}

std::optional<double> OrderBook::queueImbalance() const {
  if (bids_.empty() || asks_.empty()) return std::nullopt;
  double b = static_cast<double>(bids_.rbegin()->second.total_size);
  double a = static_cast<double>(asks_.begin()->second.total_size);
  if (b+a == 0.0) return std::nullopt;
  return b / (a + b);
}

std::pair<int64_t,int64_t> OrderBook::depthN(int n) const {
  int64_t bsum=0, asum=0;
  // 前 N 档买
  for (auto it = bids_.rbegin(); it!=bids_.rend() && n>0; ++it, --n) bsum += it->second.total_size;
  // 前 N 档卖
  n = n < 0 ? 0 : n; // 防御
  int cnt = 0;
  for (auto it = asks_.begin(); it!=asks_.end() && cnt<n; ++it, ++cnt) asum += it->second.total_size;
  return {bsum, asum};
}

} // namespace md
