Feature: Parser shall parse a protocol yaml file
	TODO: list of refs to the HL specification
	The tsn_gen can parse a folder containing the list of 
	protcol and extract the information of all protocol descriptions.

	Scenario: A folder not containing any YAML protocol
		Given: tsn_gen available in build
		When: tsn_gen folder argument is ./tests/dummy-proto-folder
		Then: tsn_gen should return an error code 254 
