#include "../lib/ratelimiter.hpp"
#include<cassert>




void check_token_bucket_single() {

  FakeClock clk;
  TokenBucketSingle rl(5,1,clk);

  // check capacity
  for (size_t i = 0; i < 5; ++i) { assert(rl.request() == 0); }
  // check for rejection
  assert(rl.request() == -1);

  // check for early fill
  clk.advance(500ms);
  assert(rl.request() == -1);
  // check for fill
  clk.advance(500ms);
  assert(rl.request() == 0);
  // check correct fill rate
  assert(rl.request() == -1);

  // check fill caps at size
  clk.advance(100s);
  for (size_t i = 0; i < 5; ++i) { assert(rl.request() == 0); }
  assert(rl.request() == -1);
}



void check_token_bucket_multi() {

  FakeClock clk;
  TokenBucketMulti rl(5,1,clk);

  // check capacity
  for (size_t i = 0; i < 5; ++i) { assert(rl.request() == 0); }
  // check for rejection
  assert(rl.request() == -1);

  // check for early fill
  clk.advance(500ms);
  assert(rl.request() == -1);
  // check for fill
  clk.advance(500ms);
  assert(rl.request() == 0);
  // check correct fill rate
  assert(rl.request() == -1);

  // check fill caps at size
  clk.advance(100s);
  for (size_t i = 0; i < 5; ++i) { assert(rl.request() == 0); }
  assert(rl.request() == -1);
}


int main() {
  check_token_bucket_single();
  check_token_bucket_multi();

}
