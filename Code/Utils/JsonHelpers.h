#pragma once
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

namespace JSON
{
	static void WriteFloat(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, float value)
	{
		writer->Key(key);
		writer->Double(value);
	}

	static void WriteBool(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, bool value)
	{
		writer->Key(key);
		writer->Bool(value);
	}

	static void WriteFloat2(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, XMFLOAT2 value)
	{
		writer->Key(key);
		writer->StartArray();
		writer->Double(value.x);
		writer->Double(value.y);
		writer->EndArray();
	}

	static void WriteFloat3(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, XMFLOAT3 value)
	{
		writer->Key(key);
		writer->StartArray();
		writer->Double(value.x);
		writer->Double(value.y);
		writer->Double(value.z);
		writer->EndArray();
	}

	static void WriteFloat4(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, XMFLOAT4 value)
	{
		writer->Key(key);
		writer->StartArray();
		writer->Double(value.x);
		writer->Double(value.y);
		writer->Double(value.z);
		writer->Double(value.w);
		writer->EndArray();
	}

	static void WriteInt(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, int value)
	{
		writer->Key(key);
		writer->Int(value);
	}

	static void WriteString(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const char* key, const char* value)
	{
		writer->Key(key);
		writer->String(value);
	}

	static float ReadFloat(rapidjson::Document& document, const char* key)
	{		
		assert(document[key].IsFloat());
		return document[key].GetFloat();
	}

	static bool ReadBool(rapidjson::Document& document, const char* key)
	{
		assert(document[key].IsBool());
		return document[key].GetBool();
	}

	static int ReadInt(rapidjson::Document& document, const char* key)
	{
		assert(document[key].IsInt());
		return document[key].GetInt();
	}

	static const char* ReadString(rapidjson::Document& document, const char* key)
	{
		assert(document[key].IsString());
		return document[key].GetString();
	}

	static XMFLOAT2 ReadFloat2(rapidjson::Document& document, const char* key)
	{
		assert(document[key].IsArray());
		rapidjson::Value& a = document[key];
		return XMFLOAT2( a[0].GetFloat(), a[1].GetFloat());
	}

	static XMFLOAT3 ReadFloat3(rapidjson::Document& document, const char* key)
	{
		assert(document[key].IsArray());
		rapidjson::Value& a = document[key];
		return XMFLOAT3(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
	}

	static XMFLOAT4 ReadFloat4(rapidjson::Document& document, const char* key)
	{
		assert(document[key].IsArray());
		rapidjson::Value& a = document[key];
		return XMFLOAT4(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat(), a[3].GetFloat());
	}

}