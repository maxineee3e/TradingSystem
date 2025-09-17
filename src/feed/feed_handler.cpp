#include "feed/feed_handler.h"
#include "order_book/order_book.h"
#include "core/types.h"
#include <type_traits>
#include <utility>
#include <variant>
#include <iostream>
#include "databento/historical.hpp"   // 你的子模块头

namespace md{ // 仅本文件可见的工具

// 你前面已添加过：单条记录 -> md::Event
md::Event makeEvent(const std::string& action,
                    const std::string& order_id,
                    const std::string& side,
                    int64_t price,
                    int64_t size);

void dispatchToBook(OrderBook& book, const md::Event& ev);

} // anonymous namespace

namespace md {

FeedHandler::FeedHandler(BlockingQueue<Event>& bus, Publisher& pub)
  : bus_(bus), publisher_(pub) {}

void FeedHandler::startHistoricalFeed(const std::string& dataset,
                                      const std::string& symbols,
                                      const std::string& schema,
                                      const std::string& start,
                                      const std::string& end) {
  running_ = true;

  th_ = std::thread([this, dataset, symbols, schema, start, end]{
    try {
      databento::Historical client{std::getenv("DATABENTO_API_KEY")};

      client.BatchDownload(
        databento::BatchDownloadParams{
          .dataset = dataset,
          .symbols = symbols,   // 例: "AAPL,MSFT" 或期货代码
          .schema  = schema,    // 例: "l3", "mbp-1", "trades" 等
          .start   = start,     // "YYYY-MM-DD"
          .end     = end
        },
        // 每条记录的回调
        [this](const auto& rec) {
          // ★ 把 rec 的字段映射到你需要的 5 个字段（按你选的 schema 改名）:
          // 下列名称只是“占位”，请用实际字段替换：
          const std::string action = rec.action;                         // e.g. "ADD"/"MOD"/"CXL"/"FILL"
          const std::string oid    = std::to_string(rec.order_id);       // 或直接 rec.order_id_str
          const std::string side   = std::string(1, rec.side);           // 'B' / 'A' or 'S'
          const int64_t price      = static_cast<int64_t>(rec.price);    // 注意价格单位
          const int64_t size       = static_cast<int64_t>(rec.size);

          if (immediate_mode_) {
            auto ev = makeEvent(action, oid, side, price, size);
            dispatchToBook(publisher_.book(), ev);   // 直接更新订单簿
          } else {
            bus_.push(makeEvent(action, oid, side, price, size)); // 入队
          }
        }
      );
    } catch (const std::exception& e) {
      std::cerr << "[FeedHandler] error: " << e.what() << std::endl;
    }
  });
}

void FeedHandler::stop() {
  running_ = false;
  if (th_.joinable()) th_.join();
}

// 便于其它数据源复用
void FeedHandler::onRecord(const std::string& action,
                           const std::string& order_id,
                           const std::string& side,
                           int64_t price,
                           int64_t size) {
  auto ev = makeEvent(action, order_id, side, price, size);
  dispatchToBook(publisher_.book(), ev);
}
void FeedHandler::onRecordQueued(const std::string& action,
                                 const std::string& order_id,
                                 const std::string& side,
                                 int64_t price,
                                 int64_t size) {
  bus_.push(makeEvent(action, order_id, side, price, size));
}

void Publisher::start() {
  running_ = true;
  th_ = std::thread([this]{
    while (running_) {
      md::Event ev;
      if (bus_.try_pop(ev, std::chrono::milliseconds(10))) {
        std::visit([&](auto&& e){
          using T = std::decay_t<decltype(e)>;
          if constexpr (std::is_same_v<T, md::Add>) {
            // Add 里是 OrderRef o
            book_.addOrder(e.o.id, e.o.side, e.o.price, e.o.size);
          } else if constexpr (std::is_same_v<T, md::Modify>) {
            book_.modifyOrder(e.id, e.new_size);
          } else if constexpr (std::is_same_v<T, md::Cancel>) {
            book_.cancelOrder(e.id);
          } else if constexpr (std::is_same_v<T, md::Fill>) {
            book_.fillOrder(e.id, e.fill_size);
          } else {
            // Heartbeat: no-op
          }
        }, ev);
      }
      }
  }
}
} // namespace md
