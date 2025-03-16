#pragma once

#include <string>
#include <vector>

#include <version.h>

#include <protocol_node.h>


/** 
 * Error class, each class has to have its own error class
 */
class ProtocolParserErr {
public:
	enum {
		PROTOPARSER_SUCCESS,
		PROTOPARSERERR_UNKNOWN,
		PROTOPARSERERR_INVALID_PATH,

		ENUM_SENTINEL
	};

	ProtocolParserErr(void) {} ;
	ProtocolParserErr(const ProtocolParserErr & ) = delete;

	const std::string& getErrorMsg(void) const {
		return mErrorVector[mErrorCode];
	}

	int getErrorCode(void) const {
		return mErrorCode;
	}

	void setErrorCode(const int errorCode) {
		mErrorCode = errorCode;
	}
private:
	const std::vector<std::string> mErrorVector = { 
		"PROTOLPARSER SUCCESS",
		"PROTOLPARSER ERROR: UNKNOWN", 
		"PROTOLPARSER INVALID_PATH: UNKNOWN"
       	};

	/** 
	 * This is the erorr code, for internal retrievial 
	 * @fixme Make it trade safe,
	 */ 
	int mErrorCode;

};

/**
 * This class is in charge of retrieve the list of NODE availables
 * in the base ProtocolPath
 */
class ProtocolParser {
public:
	ProtocolParser(const std::string& baseProtoPath);
	/**
	 * Get rid of the default copy constructors
	 */
	ProtocolParser(const ProtocolParser &) = delete;
	ProtocolParser& operator=(const ProtocolParser &) = delete;

	/**
	 * @brief Provide the Version of the parser
	 * @return a reference on the string
	 */
	const std::string& getVersion(void) const;

	/**
	 * @brief Provide the base path (top level directory)
	 * 	Where are located all the protocols
	 * @return a reference to the base path where the
	 * 	protocol are  specified
	 */
	const std::string& getBasePath(void) const;

	/**
	 * @brief This function will generate the list of protocols
	 * 	available, yet there are no hierarchy.
	 * @param [in, out] nodes: The list of nodes to retrieve
	 */
	const ProtocolParserErr& parse(std::vector<ProtocolNode>& nodes);

private:
	const std::string mProtocolParserVersion;
	const std::string mProtocolBasePath;

	/** This is the shared accrose the whole object */
	ProtocolParserErr mErrorsMsg;
};
