
#include "spark_precompiled.h"

#include "FileUtils.h"
#include "StringUtils.h"

#include <AzFramework/FileFunc/FileFunc.h>
#include "Utils/StringUtils.h"

#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/SystemFile.h>

#include <AzCore/JSON/stringbuffer.h>
#include <AzCore/JSON/prettywriter.h>

namespace FileUtils {

	bool FileExists(const AZStd::string& filePath, AZ::IO::FileIOBase* overrideFileIO)
	{
		AZ::IO::FileIOBase* fileIo = overrideFileIO != nullptr ? overrideFileIO : AZ::IO::FileIOBase::GetInstance();
		if (fileIo == nullptr)
		{
			return false;
		}

		AZ::IO::HandleType fileHandle;
		if (!fileIo->Open(filePath.c_str(), AZ::IO::OpenMode::ModeRead, fileHandle))
		{
			fileIo->Close(fileHandle);
			return false;
		}

		fileIo->Close(fileHandle);
		return true;
	}

	AZ::Outcome<AZStd::string, AZStd::string> ReadTextFile(const AZStd::string& filePath, AZ::IO::FileIOBase* overrideFileIO)
	{
		AZ::IO::FileIOBase* fileIo = overrideFileIO != nullptr ? overrideFileIO : AZ::IO::FileIOBase::GetInstance();
		if (fileIo == nullptr)
		{
			return AZ::Failure(AZStd::string("No FileIO instance present."));
		}

		AZ::IO::HandleType fileHandle;
		if (!fileIo->Open(filePath.c_str(), AZ::IO::OpenMode::ModeRead, fileHandle))
		{
			fileIo->Close(fileHandle);
			return AZ::Failure(AZStd::string("Failed to open."));
		}

		AZ::u64 fileSize = 0;
		if (!fileIo->Size(fileHandle, fileSize))
		{
			fileIo->Close(fileHandle);
			return AZ::Failure(AZStd::string::format("Failed to read size of file."));
		}

		AZStd::string text;
		text.resize(fileSize);
		if (!fileIo->Read(fileHandle, text.data(), fileSize, true))
		{
			fileIo->Close(fileHandle);
			return AZ::Failure(AZStd::string::format("Failed to read file."));
		}
		fileIo->Close(fileHandle);

		return AZ::Success(AZStd::move(text));
	}

	AZ::Outcome<rapidjson::Document, AZStd::string> ReadJsonFile(const AZStd::string& jsonFilePath, AZ::IO::FileIOBase* overrideFileIO)
	{
		auto result = ReadTextFile(jsonFilePath);
		if (result.IsSuccess())
		{
			auto str = result.GetValue();
			StringUtils::RemoveLineComments(str);

			rapidjson::Document document;
            if (document.Parse(str.c_str()).HasParseError())
            {
                return AZ::Failure(
                    AZStd::string::format("Error parsing json contents (offset %u): %d",
                        document.GetErrorOffset(),
                        document.GetParseError()));
            }

            return AZ::Success(AZStd::move(document));
		}
		else
		{
			return AZ::Failure(result.GetError());
		}
	}

	AZ::Outcome<void, AZStd::string> WriteTextFile(const AZStd::string& filePath, const AZStd::string content)
    {
        AZ::IO::SystemFile file;
        if (!file.Open(filePath.c_str(), AZ::IO::SystemFile::SF_OPEN_WRITE_ONLY | AZ::IO::SystemFile::SF_OPEN_CREATE | AZ::IO::SystemFile::SF_OPEN_CREATE_PATH))
        {
            return AZ::Failure(AZStd::string::format("Error opening file '%s'.", filePath.c_str()));
        }

    
        auto bytesWritten = file.Write(content.c_str(), content.size());
        file.Close();

        if (bytesWritten != content.size())
        {
            return AZ::Failure(AZStd::string::format("Failed to write to file %s.", filePath.c_str()));
        }

        return AZ::Success();
    }


	AZ::Outcome<AZStd::string, AZStd::string> GetValueForKeyInCfgFile(const AZStd::string& filePath, const AZStd::string key)
	{
		auto outcome = ReadJsonFile(filePath);
		if (outcome.IsSuccess())
		{
			rapidjson::Document &document=outcome.GetValue();
			if (document.HasMember(key.c_str()) && document[key.c_str()].IsString())
			{
				AZStd::string value = AZStd::string(document[key.c_str()].GetString());
				return AZ::Success( AZStd::move(value));
			}
			return AZ::Success( AZStd::move(AZStd::string()));
		}
		
		return AZ::Failure(AZStd::string::format("Failed to read from the file %s.", filePath.c_str()));
	}


	AZ::Outcome<void, AZStd::string> ReplaceInCfgFile(const AZStd::string& filePath, const AZStd::string key, const AZStd::string value)
    {
		rapidjson::Document document;

		auto outcome = ReadJsonFile(filePath);
		if (outcome.IsSuccess())
		{
			document.Swap(outcome.GetValue());
		}
		if (!document.IsObject())
		{
			document.SetObject();
		}


		if ( document.HasMember(key.c_str()))
		{
			document[key.c_str()].SetString(value.c_str(),value.length());
		}
		else
		{
			document.AddMember(rapidjson::Value(key.c_str(),key.length()),rapidjson::Value(value.c_str(),value.length()), document.GetAllocator());
		}


		AZStd::string content;
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		document.Accept(writer);
		content = buffer.GetString();
		buffer.Clear();



        AZ::IO::SystemFile file;
        if (!file.Open(filePath.c_str(), AZ::IO::SystemFile::SF_OPEN_WRITE_ONLY | AZ::IO::SystemFile::SF_OPEN_CREATE | AZ::IO::SystemFile::SF_OPEN_CREATE_PATH))
        {
            return AZ::Failure(AZStd::string::format("Error opening file '%s'.", filePath.c_str()));
        }

    
        auto bytesWritten = file.Write(content.c_str(), content.size());
        file.Close();

        if (bytesWritten != content.size())
        {
            return AZ::Failure(AZStd::string::format("Failed to write to file %s.", filePath.c_str()));
        }

        return AZ::Success();
    }

}
