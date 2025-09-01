#include "feed/feed_handler.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace md {

// ---------------- Publisher ----------------
void Publisher::start() {
  th_ = std::thread([this]{
    while (running_) {
      auto ev = bus_.pop();
      if (!ev.has_value()) break;
      book_.onEvent(*ev);

      // 简单演示：每处理一条就打印 TOP（真实系统可做定频/批量发布）
      if (auto t = book_.top(); t.best_bid && t.best_ask) {
        auto sp = book_.spread();
        auto qi = book_.queueImbalance();
        std::cout << "TOP  bid(" << t.best_bid->first << "," << t.best_bid->second
                  << ")  ask(" << t.best_ask->first << "," << t.best_ask->second << ")";
        if (sp) std::cout << "  spread=" << *sp;
        if (qi) std::cout << "  q_imb=" << *qi;
        std::cout << "\n";
      }
    }
  });
}

// ---------------- ParquetSource ----------------
// 通过 Python 脚本把 parquet 流为 CSV：type,symbol,side,id,price,size,fill_size
// type: ADD/MOD/CXL/FILL/HB   side: B/S
static md::Event parseCsvLine(const std::string& line) {
  // 极简 CSV 解析（假设没有逗号转义）
  std::vector<std::string> f; f.reserve(8);
  std::stringstream ss(line); std::string tok;
  while (std::getline(ss, tok, ',')) f.push_back(tok);
  if (f.size() < 7) return Heartbeat{};
  auto type = f[0], sidec = f[2], id = f[3];
  int64_t px = std::stoll(f[4]);
  int64_t sz = std::stoll(f[5]);
  int64_t fill = std::stoll(f[6]);

  auto side = Side::Unknown;
  if (sidec=="B") side = Side::Buy;
  else if (sidec=="S") side = Side::Sell;

  if (type=="ADD")   return Add{ OrderRef{id, side, px, sz} };
  if (type=="MOD")   return Modify{ id, sz };
  if (type=="CXL")   return Cancel{ id };
  if (type=="FILL")  return Fill{ id, fill };
  return Heartbeat{};
}

void ParquetSource::start() {
  th_ = std::thread([this]{
    // 启动 python 脚本：python python_scripts/parquet_stream.py <parquet_file>
    std::string cmd = "python3 python_scripts/parquet_stream.py \"" + path_ + "\"";
#if defined(_WIN32)
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) { std::perror("popen"); return; }

    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) {
      std::string line(buf);
      if (line.empty()) continue;
      auto ev = parseCsvLine(line);
      bus_.push(std::move(ev));
    }
#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    // 结束时停止队列
    bus_.stop();
  });
}

void ParquetSource::join() { if (th_.joinable()) th_.join(); }

} // namespace md
