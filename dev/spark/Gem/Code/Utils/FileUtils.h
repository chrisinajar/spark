#pragma once


#include <AzFramework/FileFunc/FileFunc.h>
#include <AzCore/IO/FileIO.h>


namespace FileUtils {

	bool FileExists(const AZStd::string& filePath, AZ::IO::FileIOBase* overrideFileIO = nullptr); //only use that if you don't actually need to read the file

	AZ::Outcome<AZStd::string, AZStd::string> ReadTextFile(const AZStd::string& filePath, AZ::IO::FileIOBase* overrideFileIO = nullptr);
	AZ::Outcome<void, AZStd::string>	      WriteTextFile(const AZStd::string& filePath, const AZStd::string content);


	AZ::Outcome<rapidjson::Document, AZStd::string> ReadJsonFile(const AZStd::string& jsonFilePath, AZ::IO::FileIOBase* overrideFileIO = nullptr);

	AZ::Outcome<AZStd::string, AZStd::string> GetValueForKeyInCfgFile(const AZStd::string& filePath, const AZStd::string key);
	AZ::Outcome<void, AZStd::string>		  ReplaceInCfgFile(const AZStd::string& filePath, const AZStd::string key, const AZStd::string value);

}
