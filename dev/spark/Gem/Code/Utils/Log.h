#pragma once
#include <AzCore/std/string/string.h>
#include <AzCore/std/string/conversions.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Component/EntityId.h>
#include <Cry_Color.h>


//just ERROR causes conflicts, and sparkERROR is too long
#if true
#define sDEBUG(msg)   spark::log::print(spark::log::DebugString()+"DEBUG: "+msg)
#define sLOG(msg)   spark::log::print(spark::log::DebugString()+"LOG: "+msg)
#define sERROR(msg)   spark::log::print(spark::log::DebugString()+"ERROR: "+msg)
#define sWARNING(msg)   spark::log::print(spark::log::DebugString()+"WARNING: "+msg)
#else
#define sDEBUG(msg)
#define sLOG(msg)
#define sERROR(msg)
#define sWARNING(msg)
#endif

namespace spark
{
	namespace log
	{



		struct DebugString
		{

			AZStd::string data;

			DebugString() {}
			DebugString(const char* c) {
				data = AZStd::string(c);
			}
			DebugString(AZStd::string s) : data(s){
			}
			template<class T>
			DebugString(T t) :data(t.ToString()){

			}

			DebugString(float f) {
				data = AZStd::to_string(f);
			}
			DebugString(double f) {
				data = AZStd::to_string(f);
			}
			DebugString(int i) {
				data = AZStd::to_string(i);
			}
			DebugString(unsigned int i) {
				data = AZStd::to_string(i);
			}
			DebugString(bool b) {
				data = b ? "true" : "false";
			}
			DebugString(char c) {
				data += c;
			}

			DebugString(AZ::Vector2 v) {
				data = "Vector2(" + AZStd::to_string((float)v.GetX()) + "," + AZStd::to_string((float)v.GetY()) + ")";
			}
			DebugString(AZ::Vector3 v) {
				data = "Vector3(" + AZStd::to_string((float)v.GetX()) + "," + AZStd::to_string((float)v.GetY()) + "," + AZStd::to_string((float)v.GetZ()) + ")";
			}
			DebugString(ColorF color) {
				data = "ColorF(" + AZStd::to_string((float)color.r) + "," + AZStd::to_string((float)color.g) + "," + AZStd::to_string((float)color.b) + "," + AZStd::to_string((float)color.a)+ ")";
			}
			DebugString(AZ::EntityId id) {
				data = AZStd::string::format("EntityId(%s)", id.IsValid() ? id.ToString().c_str() : "invalid");
			}

			DebugString& operator+(const DebugString &str) {
				DebugString *result = new DebugString(AZStd::string(toAZStd() + str.toAZStd()));
				return *result;
			}

			static void replaceAll(AZStd::string& str, const AZStd::string& from, const AZStd::string& to) {
				if (from.empty())
					return;
				size_t start_pos = 0;
				while ((start_pos = str.find(from, start_pos)) != AZStd::string::npos) {
					str.replace(start_pos, from.length(), to);
					start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}

			AZStd::string toAZStd() const {
				return data;
			}
			const char* c_str() const {
				return data.c_str();
			}


		};


		inline void print(DebugString str)
		{
			 AZ_Printf(0,"%s",str.c_str())
		}
	}
	
	

}
