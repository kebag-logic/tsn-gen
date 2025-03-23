#include <iostream>
#include <version.h>

#include <traffic_gen.h>

TrafficGen::TrafficGen(void):
	mVersion(TRAFFIC_GEN_VERSION)
{
}

const std::string& TrafficGen::getVersion(void) const
{	
	return mVersion;
}
