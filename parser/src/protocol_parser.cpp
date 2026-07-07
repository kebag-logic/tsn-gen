/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 /*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 #include <algorithm>
#include <tsn/log.h>
#include <iostream>
#include <filesystem>

#include <tsn/protocol.h>
#include <tsn/protocol_parser.h>
#include <tsn/version.h>

namespace tsn {



ProtocolParser::ProtocolParser(const std::string& baseProtoPath):
	mProtocolParserVersion(PROTOCOL_PARSER_VERSION),
	mProtocolBasePath(baseProtoPath),
	mCurrentDepthPathListing(0)
{
}

const std::string& ProtocolParser::getVersion(void) const
{
	return mProtocolParserVersion;
}

const std::string& ProtocolParser::getBasePath(void) const
{
	return mProtocolBasePath;
}

const ProtocolParserErr ProtocolParser::parse(void)
{
	ProtocolParserErr err(ProtocolParserErr::PROTOPARSERERR_UNKNOWN);

	if (!std::filesystem::exists(mProtocolBasePath)) {
		log(LogLevel::error) << "ERROR: " << "Path Provided is incorrect " <<
					mProtocolBasePath << std::endl;
		err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);

		return err;
	}
	/** First list the folder content */
	err = listProtocolFiles(mProtocolBasePath);
	if (err.getErrorCode()) {
		return err;
	}

	if (!yamlPaths.size()) {
		log(LogLevel::error) << "ERROR: " << "No yaml files found in directory" <<
					mProtocolBasePath << std::endl;
		err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);

		return err;
	}

	/* Invalid files are skipped during listing; a directory that yields
	 * zero usable protocol definitions is still an input error. */
	if (mDbServices.size() == 0) {
		log(LogLevel::error) << "ERROR: " << "No valid protocol definition found in " <<
					mProtocolBasePath << std::endl;
		err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
	}

	return err;
}

// TODO create its own class
const ProtocolParserErr ProtocolParser::parseInterfaces(void)
{
	ProtocolParserErr err(ProtocolParserErr::PROTOPARSERERR_UNKNOWN);
	return err;
}

// TODO create its own classfg
const ProtocolParserErr ProtocolParser::parseVars(void)
{
	ProtocolParserErr err(ProtocolParserErr::PROTOPARSERERR_UNKNOWN);
	//std::string file (dir_entry.path());
	// file.erase(file.begin(), file.begin() + mProtocolBasePath.length());
	// std::replace(file.begin(), file.end(), '/', ':');

	return err;
}

const ProtocolParserErr ProtocolParser::parseProtocol(const std::string& path)
{
	ProtocolParserErr err(ProtocolParserErr::PROTOPARSERERR_UNKNOWN);
	Protocol proto (path);
	ProtocolErr errorProto = proto.parseProtocolFile(
		mDbVars, mDbIfproto, mDbServices);
	if (errorProto.getErrorCode()) {
		err.setErrorCode(errorProto.getErrorCode());
	} else {
		err.setErrorCode(ProtocolParserErr::PROTOPARSER_SUCCESS);
	}

	return err;
}

const ProtocolParserErr ProtocolParser::listProtocolFiles(const std::string& path)
{
	ProtocolParserErr err(ProtocolParserErr::PROTOPARSER_SUCCESS);

	log(LogLevel::debug) << "Entering directory: " << path << std::endl;
	for (const std::filesystem::directory_entry& dir_entry :
		std::filesystem::directory_iterator(path)) {

		/** Check if a file and parse inside of it */
		if (std::filesystem::is_regular_file(dir_entry.path())) {
			if (dir_entry.path().extension() != ".yaml") {
				log(LogLevel::debug) << "Skipping non-YAML file "
							<< dir_entry.path() << std::endl;
				continue;
			}

			log(LogLevel::debug) << "Parsing YAML file "
						<< dir_entry.path() << std::endl;
			yamlPaths.push_back(dir_entry.path());
			err = parseProtocol(dir_entry.path());

			/* Files without a 'service:' key (stubs, functional specs, etc.)
			   are not protocol definitions — skip them rather than aborting. */
			if (err.getErrorCode() == ProtocolErr::PROTOERR_INVALID_FILE) {
				err.setErrorCode(ProtocolParserErr::PROTOPARSER_SUCCESS);
			} else if (err.getErrorCode()) {
				break;
			}
		}
		else if (std::filesystem::is_directory(dir_entry.path())) {
			mCurrentDepthPathListing++;
			if (mCurrentDepthPathListing > MAX_CURRENT_DEPTH_PATH) {
				log(LogLevel::error) << "Error " << "File hierarchy too deep max Allowed "
							<< MAX_CURRENT_DEPTH_PATH << " at " << path
							<< std::endl;

				err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_UNEXPECTED);
			} else {
				err = listProtocolFiles(dir_entry.path());
				mCurrentDepthPathListing--;
			}

			// Exit the loop something went wrong.
			if (err.getErrorCode()) {
				break;
			}
		}
	}

	return err;
}

} /* namespace tsn */
