#include <feed/message_parser.h>
#include <sstream>
#include <vector>

namespace feed {

static inline std::vector<std::string> split_csv(const std::string& s) {
  std::vector<std::string> out; out.reserve(8);
  std::stringstream ss(s); std::string tok;
  while (std::getline(ss, tok, ',')) out.push_back(tok);
  return out;
}

std::optional<core::Event> MessageParser::parse_line(const std::string& line) const {
  auto f = split_csv(line);
  if (f.size() < 7) return core::Heartbeat{}; // 容错

  const auto& typ = f[0];
  // const auto& sym = f[1]; // 暂时不在 Event 里用到，如需多合约可加入到 OrderRef
  const auto& sdc = f[2];
  const auto& id  = f[3];

  auto side = core::Side::Unknown;
  if (sdc=="B") side = core::Side::Buy;
  else if (sdc=="S") side = core::Side::Sell;

  int64_t px   = 0, sz = 0, fill = 0;
  try {
    px   = std::stoll(f[4]);
    sz   = std::stoll(f[5]);
    fill = std::stoll(f[6]);
  } catch (...) { return std::nullopt; }

  if (typ=="ADD")   return core::Add{ core::OrderRef{id, side, px, sz} };
  if (typ=="MOD")   return core::Modify{ id, sz };
  if (typ=="CXL")   return core::Cancel{ id };
  if (typ=="FILL")  return core::Fill{ id, fill };
  if (typ=="HB")    return core::Heartbeat{};
  return std::nullopt;
}

} // namespace feed
