#include "core/types.h"
#include "feed/blocking_queue.h"
#include "feed/feed_handler.h"
#include "feed/publisher.h"
#include <thread>
#include <chrono>

int main() {
  md::BlockingQueue<md::Event> bus;   // ← 用 md::Event
  md::Publisher publisher(bus);
  publisher.start();

  md::FeedHandler fh(bus, publisher);
  fh.enableImmediateMode(true);       // 立即处理；若想走队列：false

  // 历史回放测试（需要 DATABENTO_API_KEY）
  fh.startHistoricalFeed(
      "GLBX.MDP3",    // dataset
      "ES.FUT",       // symbols
      "l3",           // schema
      "2023-08-01",   // start
      "2023-08-01"    // end
  );

  std::this_thread::sleep_for(std::chrono::seconds(10));
  fh.stop();            // 先停生产者
  publisher.stop();     // 再停消费者（让队列里的事件处理完）
  return 0;
}
