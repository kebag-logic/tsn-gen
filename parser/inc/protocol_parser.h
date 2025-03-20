#pragma once

// TODO think of the builder design pattern, first we loop through everything

/**
 * This class is in charge of creating the database of protocol availble in the
 * 	base folder provide.
 *
 * It will first loop through the folder, looking for VALID yaml file:
 *
 *  1. Has to have a service: name_of_the_service
 *  2. A type: protocol
 *  2. interfaces and one
 * 		a. A least one protocol specific interface
 * 		b. Optionally An Introspection applicaiton interface.
 *  3. At least one var for each interfaces
 *  4. And a documentation reference.
 */

#include <map>
#include <string>
#include <vector>

#include <version.h>

#include <database.h>
#include <entity.h>
#include <protocol.h>
#include <protocol_interface.h>
#include <var.h>


#define MAX_CURRENT_DEPTH_PATH	(16)

/**
 * Error class, each class has to have its own error class
 */
class ProtocolParserErr: public Error {
public:
	enum {
		PROTOPARSER_SUCCESS,
		PROTOPARSERERR_UNKNOWN,
		PROTOPARSERERR_INVALID_INPUT,
		PROTOPARSERERR_UNEXPECTED,
		PROTOPARSERERR_UNDEFINED,
		PROTOPARSERERR_EXISTS,
		PROTOPARSERERR_ENUM_SENTINEL
	};

	ProtocolParserErr(void) = delete;
	ProtocolParserErr(int errorNum): Error(errorNum, mErrVector) {}

	ProtocolParserErr& operator=(const ProtocolParserErr other)
	{
		mErrorCode = other.mErrorCode;
		return *this;
	}

private:
	const std::vector<std::string> mErrVector = {
		"PPASER:ERR: SUCCESS",
		"PPASER:ERR: UNKNOWN",
		"PPASER:ERR: INVALID INPUT",
		"PPASER:ERR: UNEXPECTED"
		"PPASER:ERR: UNDEFINED"
		"PPASER:ERR: EXISTS"
       	};
};

/**
 * This class is in charge off retrieve the list of NODE availables
 * in the base ProtocolPath
 */
class ProtocolParser {
public:
	ProtocolParser(const std::string& baseProtoPath);
	/**
	 * Get rid of the default copy constructors
	 */
	ProtocolParser(const ProtocolParser &) = delete;
	const ProtocolParser operator=(const ProtocolParser &) = delete;

	/**
	 * @brief Provide the Version of the parser
	 * @return a reference on the string
	 */
	const std::string& getVersion(void) const;

	/**
	 * @brief Provide the base path (top level directory)
	 * 	Where are located al
	 * l the protocols
	 * @return a reference to the base path where the
	 * 	protocol are  specified
	 */
	const std::string& getBasePath(void) const;

	/**
	 * @brief This function will generate the list of protocols
	 *			available, yet there are no hierarchy. Steps are as following:
	 *
	 *			1. List the .yaml or .yml YAML files available from the base folder
	 *			2. For each YAML file:
	 *				a. Parse for the vars that are listed;
	 *					1. List the vars
	 *					2. Then the typdef, and link them with the vars.
	 *			3. The generate an interfaces for each entity.
	 *			4. Each entity will then compose the protocol.
	 *
	 * @return ProtocolParserErr, 0 upon success, or PROTOPARSERERR_UNEXPECTED if
	 * 		not good yaml file were provided.
	 */
	const ProtocolParserErr parse(void);

private:

	/**
	 * @brief This function will list the yaml file, and will create a list of
	 *		yaml file That are going to be processed.
	 *		Once a yaml file is found it will add to the lsit
	 * @return 0 upon success, PROTOPARSERERR_INVALID_INPUT if no yaml file are
	 *		found
	 * 		if folder depth < MAX_CURRENT_DEPTH_PATH, then
	 * 			PROTOPARSERERR_UNEXPECTED shall be return
	 *
	 * @todo create its own class
	 */
	const ProtocolParserErr listProtocolFiles(const std::string& path);

	/**
	 * @brief Will loop through the file list and then start parsing each file,
	 *		if a file is invalid, it willi remove the it from the listProtocol file.
	 *		and generate an error
	 *
	 * @return PROTOPARSERERR_INVALID_INPUT if the file does not contain the basis
	 * 			as defined.
	 *
	 */
	const ProtocolParserErr parseProtocol(const std::string& path);

	/**
	 * @brief Go through a protocol defined file, and looks for the information
	 *			It will call
	 */
	const ProtocolParserErr parseInterfaces(void);

	/**
	 * @brief Parse the var from the file, if it does not exist already,
	 * 			then it adds it to the mDbVars, with the full name, even if the
	 * 			this is a reference, without the value. So that later its value
	 * 			can be set correctly.
	 *
	 * @return If the value is set already, and is not a reference it will return
	 *			PROTOPARSERERR_EXISTS.
	 *		   If during the parsing, a beacon is missing, or not expected,
	 *				it will throw the PROTOPARSERERR_UNEXPECTED error.
	 */
	const ProtocolParserErr	parseVars(void);

	const std::string mProtocolParserVersion;
	const std::string mProtocolBasePath;

	std::vector<std::string> yamlPaths;
	/** The databased that are used by the parser */
	Database<Var> mProtocols;
	Database<Var> mDbVars;
	Database<ProtocolInterface> mDbIfproto;
	int mCurrentDepthPathListing ;
};
