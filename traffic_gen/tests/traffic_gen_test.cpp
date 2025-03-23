#include <traffic_gen_common.h>
#include <traffic_gen.h>
#include <version.h>


TEST(TrafficGen, Version) {
	TrafficGen testTG;

	EXPECT_STREQ(TRAFFIC_GEN_VERSION,
				 testTG.getVersion().c_str());
}
