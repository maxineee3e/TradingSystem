#pragma once
#include <optional>
#include <string>
#include <core/types.h>

namespace md {

// L3 解析器：把一行 CSV / 文本消息解析成 core::Event
// 输入格式（示例）：
//   "ADD,ES,B,ord123,502350,5,0"
//   "MOD,ES,B,ord123,502350,7,0"   // new_size=7
//   "CXL,ES,B,ord123,502350,0,0"
//   "FILL,ES,B,ord123,502350,0,3"  // fill_size=3
//   "HB,ES,?,-,0,0,0"
class MessageParser {
public:
  std::optional<Event> parse_line(const std::string& line) const;
};

} // namespace feed
