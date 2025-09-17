#pragma once
#include <atomic>
#include <thread>
#include <variant>
#include <type_traits>
#include <chrono>
#include "core/types.h"
#include "feed/blocking_queue.h"
#include "order_book/order_book.h"

namespace md {

class Publisher {
public:
  explicit Publisher(BlockingQueue<Event>& bus) : bus_(bus) {}

  void start() {
    running_ = true;
    th_ = std::thread([this]{
      while (running_) {
        Event ev;
        if (bus_.try_pop(ev, std::chrono::milliseconds(10))) {
          std::visit([&](auto&& e){
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, Add>)
              book_.addOrder(e.o.id, e.o.side, e.o.price, e.o.size);
            else if constexpr (std::is_same_v<T, Modify>)
              book_.modifyOrder(e.id, e.new_size);
            else if constexpr (std::is_same_v<T, Cancel>)
              book_.cancelOrder(e.id);
            else if constexpr (std::is_same_v<T, Fill>)
              book_.fillOrder(e.id, e.fill_size);
          }, ev);
        }
      }
    });
  }

  void stop() {
    running_ = false;
    if (th_.joinable()) th_.join();
  }

  OrderBook& book() { return book_; }   // 注意命名空间，如你的 OrderBook 在 md::

private:
  BlockingQueue<Event>& bus_;
  std::atomic<bool> running_{false};
  std::thread th_;
  OrderBook book_; // 如果你的类在 md::orderbook::OrderBook，就改为那个完整名
};

} // namespace md
