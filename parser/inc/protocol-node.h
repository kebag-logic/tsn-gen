#pragma once

#include <string>

class ProtocolNodeErr {
public:
	enum {
		PROTOCOLERR_SUCCESS = 0
		PROTOCOLERR_UNKNOWN = 1
		PROTOCOLERR_INVALID_PATH = 2
	};

	ProtocolNodeErr(void) = delete
	ProtocolNodeErr(const ProtocolNodeErr & ) = delete;

	ProtocolNodeErr& operator=(const ProtocolNodeErr & ) = delete;

	static const std::string& operator=(const int errCode) const {
		return mErrorVector[-errCode];
	}

private:
	const std::vector<std::string> mErrorVector = { 
		"PROTOCOL SUCCESS",
		"PROTOCOL ERROR: UNKNOWN", 
		"PROTOCOL INVALID_PATH: UNKNOWN", 
       	};

};

class ProtocolNode {
public:
	ProtocolNode(const string& P)
private:
	const std::string mPath;
};
