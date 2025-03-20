#include <algorithm>
#include <iostream>
#include <filesystem>

#include <protocol_parser.h>
#include <version.h>



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
		std::cerr << "ERROR: " << "Path Provided is incorrect " <<
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
		std::cerr << "ERROR: " << "No yaml files found in directory" <<
					mProtocolBasePath << std::endl;
		err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);

		return err;
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

	if (proto.parseProtocolFile(mDbVars, mDbIfproto).getErrorCode()) {
		err.setErrorCode(ProtocolParserErr::PROTOPARSERERR_INVALID_INPUT);
	}

	return err;
}

const ProtocolParserErr ProtocolParser::listProtocolFiles(const std::string& path)
{
	ProtocolParserErr err(ProtocolParserErr::PROTOPARSER_SUCCESS);

	std::cout << "Entering directory: " << path.c_str() << std::endl;
	for (const std::filesystem::directory_entry& dir_entry :
		std::filesystem::recursive_directory_iterator(path)) {

		/** Check if a file and parse inside of it */
		if (std::filesystem::is_regular_file(dir_entry.path())) {
			std::cout << "  found file" << dir_entry.path() << std::endl;
			std::cout << "      YAML? ";
			if (dir_entry.path().extension() != ".yaml") {
				std::cout << " No" << std::endl;
				// The .yaml file is not found
				continue;
			}

			std::cout << " Yes- Checking Validity" << std::endl;
			yamlPaths.push_back(dir_entry.path());
			err = parseProtocol(dir_entry.path());

			// Exit the loop something went wrong.
			if (err.getErrorCode()) {
				break;
			}
		}
		else if (std::filesystem::is_directory(dir_entry.path())) {
			mCurrentDepthPathListing++;
			if (mCurrentDepthPathListing > MAX_CURRENT_DEPTH_PATH) {
				std::cerr << "Error " << "File hierarchy too deep max Allowed "
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