#include <sw/redis++/redis++.h>


int main() {

	auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");
	redis.set("raf", "340");
	return 0;
}
