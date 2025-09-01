#pragma once
#include <string>
#include <atomic>
#include <thread>
#include </Users/maxine/TradingSystem/databento/include/databento/historical.hpp>
#include <databento/symbol_map.hpp>
#include "feed/feed_handler.h"
#include "feed/message_parser.h"

namespace md {

class DatabentoSource : public IDataSource {
 public:
  DatabentoSource(FeedHandler& fh, Parser& parser,
                  std::string dataset, std::string start, std::string end)
    : fh_(fh), parser_(parser), dataset_(std::move(dataset)),
      start_(std::move(start)), end_(std::move(end)) {}

  void start() override;
  void stop() override { running_ = false; }

 private:
  FeedHandler& fh_;
  Parser& parser_;
  std::string dataset_, start_, end_;
  std::atomic<bool> running_{false};
};

} // namespace md

