#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>

TEST_CASE("./posix 1 0" * doctest::timeout(1)) {
  FILE* fp = freopen("tests/input.txt", "r", stdin);
  CHECK(550 == run_threads(1, 0));
  fclose(fp);
}

TEST_CASE("./posix 3 3" * doctest::timeout(1)) {
  FILE* fp = freopen("tests/input.txt", "r", stdin);
  CHECK(550 == run_threads(3, 3));
  fclose(fp);
}

TEST_CASE("./posix 30 30" * doctest::timeout(1)) {
  FILE* fp = freopen("tests/input.txt", "r", stdin);
  CHECK(550 == run_threads(30, 30));
  fclose(fp);
}