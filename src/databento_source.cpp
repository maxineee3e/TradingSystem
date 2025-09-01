#include "md/databento_source.h"
#include <cstdlib>
#include <iostream>

using namespace databento;

namespace md {

void DatabentoSource::start() {
  const char* key = std::getenv("DATABENTO_API_KEY");
  if (!key) { std::cerr << "DATABENTO_API_KEY missing\n"; return; }

  running_ = true;
  auto client = Historical::Builder().SetKey(key).Build();

  TsSymbolMap sym_map;
  auto on_meta = [&sym_map](const Metadata& md) { sym_map = md.CreateSymbolMap(); };

  auto on_rec = [this, &sym_map](const Record& rec) {
    if (!running_) return KeepGoing::Stop;

    try {
      if (rec.Type() == Record::Type::kTrade) {
        const auto& t = rec.Get<TradeMsg>();
        std::string sym;
        try { sym = sym_map.At(t); } catch (...) { sym = "iid=" + std::to_string(t.instrument_id); }
        auto ev = parser_.on_trade(t.instrument_id, t.price, t.size, t.hd.sequence, Ts{}, sym);
        fh_.on_event(ev);
      }
    } catch (...) { /* swallow & count in prod */ }

    return KeepGoing::Continue;
  };

  client.TimeseriesGetRange(
      dataset_.c_str(),
      {start_.c_str(), end_.c_str()},
      kAllSymbols,
      Schema::Trades,
      SType::InstrumentId,
      {}, {}, on_meta, on_rec);
}

} // namespace md

