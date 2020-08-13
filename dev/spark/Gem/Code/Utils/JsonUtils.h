#pragma once

#include <AzCore/JSON/stringbuffer.h>
#include <AzCore/JSON/writer.h>
#include <AzCore/JSON/document.h>

#include <AzCore/Math/Vector3.h>

namespace JsonUtils
{

	/**
    * Add to dstObjects all values of srcObject, preserving the hierarchy.
	*
	* NB: in case of conflicts the values already store in dstObject wins
    */
	static inline void MergeObjects(rapidjson::Value &dstObject,const rapidjson::Value &srcObject, rapidjson::Document::AllocatorType &allocator)
	{
		if (!dstObject.IsObject() || !srcObject.IsObject())return;

		for (auto srcIt = srcObject.MemberBegin(); srcIt != srcObject.MemberEnd(); ++srcIt)
		{
			if (!srcIt->name.IsString())continue;
			
			auto dstIt = dstObject.FindMember(srcIt->name);
			if (dstIt != dstObject.MemberEnd())
			{
				if (srcIt->value.IsObject())
				{
					MergeObjects(dstIt->value, srcIt->value, allocator);
				}
			}
			else
			{
				rapidjson::Value k;
				k.SetString(srcIt->name.GetString(),srcIt->name.GetStringLength(),allocator);

				rapidjson::Value v;
				v.CopyFrom(srcIt->value,allocator);
				dstObject.AddMember(k, v, allocator);
				//dstObject.AddMember(srcIt->name, srcIt->value, allocator);
			}
		}
	}

	static inline void ParseVector3(AZ::Vector3 &v, rapidjson::Value &value)
	{
		if (value.IsObject())
		{
			if (value.HasMember("x") && value["x"].IsNumber())v.SetX(value["x"].GetDouble());
			if (value.HasMember("y") && value["y"].IsNumber())v.SetY(value["y"].GetDouble());
			if (value.HasMember("z") && value["z"].IsNumber())v.SetZ(value["z"].GetDouble());
		}
		else if (value.IsArray())
		{
			for (int i = 0; i < value.Size() && i<3; ++i)
			{
				if (value[i].IsNumber())v.SetElement(i, value[i].GetDouble());
			}
		}

	}

	static inline AZStd::string ToString(const rapidjson::Value &json)
	{
		if (json.IsString())return json.GetString();

		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		json.Accept(writer);
		AZStd::string str(buffer.GetString());
		buffer.Clear();
		return str;
	}

	static inline rapidjson::Value* FindValue(AZStd::vector<AZStd::string> path, rapidjson::Value &srcObject)
	{
		if (path.empty())return &srcObject;

		auto it = srcObject.FindMember(path[0].c_str());
		if (it != srcObject.MemberEnd())
		{
			path.erase(path.begin());
			return FindValue(path, it->value);
		}
		return nullptr;
	}


	template<typename Function>
	void ForeachMember(rapidjson::Value &root, Function function)
	{
		if (!root.IsObject())return;

		for (auto member = root.MemberBegin(); member != root.MemberEnd(); ++member)
		{
			function(member->value);

			if (member->value.IsObject())
			{
				ForeachMember(member->value, function);
			}
		}
	}

	template<typename Function>
	void ForeachMemberWithParent(rapidjson::Value &root, Function function)
	{
		if (!root.IsObject())return;

		for (auto member = root.MemberBegin(); member != root.MemberEnd(); ++member)
		{
			function(member->value, root);

			if (member->value.IsObject())
			{
				ForeachMemberWithParent(member->value, function);
			}
		}
	}

}
