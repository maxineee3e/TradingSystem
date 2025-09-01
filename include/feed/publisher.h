#pragma once
#include "types.h"
#include "blocking_queue.h"

namespace md {

class Publisher {
public:
  explicit Publisher(BlockingQueue<core::Event>& out) : out_(out) {}
  void publish(const core::Event& e) { out_.push(e); }

private:
  BlockingQueue<core::Event>& out_;
};

} // namespace md
