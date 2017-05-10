#include "benchmark/benchmark.h"
#include "time_zone.h"

namespace {

void BM_Zone_UTCTimeZone(benchmark::State& state) {
  cctz::time_zone tz;
  while (state.KeepRunning()) {
    tz = cctz::utc_time_zone();
    benchmark::DoNotOptimize(tz);
  }
}
BENCHMARK(BM_Zone_UTCTimeZone);

}

BENCHMARK_MAIN()
