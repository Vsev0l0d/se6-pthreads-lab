#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>

TEST_CASE("./posix 3 3" * doctest::timeout(1)) {
  FILE* fp = freopen("tests/input.txt", "r", stdin);
  CHECK(550 == run_threads(3, 3));
  fclose(fp);
}