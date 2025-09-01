#include <databento/constants.hpp>
#include <databento/historical.hpp>
#include <iostream>

using namespace databento;

int main() {
  auto client =
      Historical::Builder().SetKey("").Build();
  auto print_trades = [](const Record& record) {
    const auto& trade_msg = record.Get<TradeMsg>();
    std::cout << trade_msg << '\n';
    return KeepGoing::Continue;
  };
  client.TimeseriesGetRange(
      "GLBX.MDP3", {"2022-06-10T14:30", "2022-06-10T14:40"},
      kAllSymbols, Schema::Trades, SType::RawSymbol,
      SType::InstrumentId, {}, {}, print_trades);
}