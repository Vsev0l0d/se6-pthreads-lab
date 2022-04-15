#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>
#include <sys/resource.h>

TEST_CASE("./posix 3 3" * doctest::timeout(1)) {
  struct rlimit limit = {1, 1};
  setrlimit(RLIMIT_CPU, &limit);
  FILE* fp = freopen("tests/input.txt", "r", stdin);
  CHECK(55 == run_threads(3, 3));
  fclose(fp);
}