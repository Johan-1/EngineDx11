#include "SystemDefs.h"
#include "JsonHelpers.h"
#include <fstream>

/////////////////////////////////////////////////////////////////// SYSTEM SETTINGS
float SystemSettings::SCREEN_WIDTH  = 1920.0f;
float SystemSettings::SCREEN_HEIGHT = 1080.0f;
bool SystemSettings::V_SYNC         = false;
bool SystemSettings::FULLSCREEN     = false;
bool SystemSettings::USE_HDR        = true;

SystemSettings::SystemSettings(){}
SystemSettings::~SystemSettings(){}

void SystemSettings::ReadSettings(const char* file)
{
	FILE* fp; fopen_s(&fp, file, "rb");
	char readBuffer[65536];
	rapidjson::FileReadStream inStream(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document d;
	d.ParseStream(inStream);
	fclose(fp);

	assert(d.IsObject());

	SCREEN_WIDTH  = JSON::ReadFloat(d, "SCREEN_WIDTH");
	SCREEN_HEIGHT = JSON::ReadFloat(d, "SCREEN_HEIGHT");
	V_SYNC        = JSON::ReadBool(d,  "V_SYNC");
	FULLSCREEN    = JSON::ReadBool(d,  "FULLSCREEN");
	USE_HDR       = JSON::ReadBool(d,  "USE_HDR");	
}

void SystemSettings::WriteSettings(const char* file, SystemSettingsWindow::Settings settings)
{
	// create string buffer and json writer
	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

	// start the write
	writer.StartObject();

	JSON::WriteFloat(&writer, "SCREEN_WIDTH",  (float)settings.screenSize[0]);
	JSON::WriteFloat(&writer, "SCREEN_HEIGHT", (float)settings.screenSize[1]);
	JSON::WriteBool(&writer,  "V_SYNC",        settings.vSync);
	JSON::WriteBool(&writer,  "FULLSCREEN",    settings.fullScreen);
	JSON::WriteBool(&writer,  "USE_HDR",       settings.HDR);
	
	// end object
	writer.EndObject();

	// create output file stream and write
	// out the json stringBuilder object to the file
	std::ofstream of(file);
	of << sb.GetString();

	of.close();
}

/////////////////////////////////////////////////////////////////// POST PROCESSING
bool  PostProcessing::APPLY_BLOOM                  = true;
bool  PostProcessing::BLOOM_USE_TWO_PASS_BLUR      = true;
float PostProcessing::BLOOM_INTENSITY              = 1.0f;
float PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1 = 4.0f;
float PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2 = 8.0f;

bool  PostProcessing::APPLY_DEPTH_OF_FIELD = true;
float PostProcessing::START_END_DOF_DST[]  = { 20.0f, 40.0f};

bool PostProcessing::APPLY_FXAA = true;

bool  PostProcessing::APPLY_TONEMAPPING = true;
int   PostProcessing::TONEMAP_TYPE      = 0;
float PostProcessing::TONEMAP_EXPOSURE  = 0.9f;

PostProcessing::PostProcessing(){}
PostProcessing::~PostProcessing(){}

void PostProcessing::ReadSettings(const char* file)
{
	FILE* fp; fopen_s(&fp, file, "rb");
	char readBuffer[65536];
	rapidjson::FileReadStream inStream(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document d;
	d.ParseStream(inStream);
	fclose(fp);

	assert(d.IsObject());

	APPLY_BLOOM                  = JSON::ReadBool(d,   "APPLY_BLOOM");
	BLOOM_USE_TWO_PASS_BLUR      = JSON::ReadBool(d,   "BLOOM_USE_TWO_PASS_BLUR");
	BLOOM_INTENSITY              = JSON::ReadFloat(d,  "BLOOM_INTENSITY");
	BLOOM_BLUR_SCALE_DOWN_PASS_1 = JSON::ReadFloat(d,  "BLOOM_BLUR_SCALE_DOWN_PASS_1");
	BLOOM_BLUR_SCALE_DOWN_PASS_2 = JSON::ReadFloat(d,  "BLOOM_BLUR_SCALE_DOWN_PASS_2");
	APPLY_DEPTH_OF_FIELD         = JSON::ReadBool(d,   "APPLY_DEPTH_OF_FIELD");
	XMFLOAT2 dofDst              = JSON::ReadFloat2(d, "START_END_DOF_DST");
	START_END_DOF_DST[0]         = dofDst.x; START_END_DOF_DST[1] = dofDst.y;
	APPLY_FXAA                   = JSON::ReadBool(d,   "APPLY_FXAA");
	APPLY_TONEMAPPING            = JSON::ReadBool(d,   "APPLY_TONEMAPPING");
	TONEMAP_TYPE                 = JSON::ReadInt(d,    "TONEMAP_TYPE");
	TONEMAP_EXPOSURE             = JSON::ReadFloat(d,  "TONEMAP_EXPOSURE");
}

void PostProcessing::WriteSettings(const char* file)
{
	// create string buffer and json writer
	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

	// start the write
	writer.StartObject();

	JSON::WriteBool(&writer,   "APPLY_BLOOM",                  APPLY_BLOOM);
	JSON::WriteBool(&writer,   "BLOOM_USE_TWO_PASS_BLUR",      BLOOM_USE_TWO_PASS_BLUR);
	JSON::WriteFloat(&writer,  "BLOOM_INTENSITY",              BLOOM_INTENSITY);
	JSON::WriteFloat(&writer,  "BLOOM_BLUR_SCALE_DOWN_PASS_1", BLOOM_BLUR_SCALE_DOWN_PASS_1);
	JSON::WriteFloat(&writer,  "BLOOM_BLUR_SCALE_DOWN_PASS_2", BLOOM_BLUR_SCALE_DOWN_PASS_2);
	JSON::WriteBool(&writer,   "APPLY_DEPTH_OF_FIELD",         APPLY_DEPTH_OF_FIELD);
	JSON::WriteFloat2(&writer, "START_END_DOF_DST",            XMFLOAT2(START_END_DOF_DST[0], START_END_DOF_DST[1]));
	JSON::WriteBool(&writer,   "APPLY_TONEMAPPING",            APPLY_TONEMAPPING);
	JSON::WriteBool(&writer,   "APPLY_FXAA",                   APPLY_FXAA);
	JSON::WriteInt(&writer,    "TONEMAP_TYPE",                 TONEMAP_TYPE);
	JSON::WriteFloat(&writer,  "TONEMAP_EXPOSURE",             TONEMAP_EXPOSURE);
	
	// end object
	writer.EndObject();

	// create output file stream and write
	// out the json stringBuilder object to the file
	std::ofstream of(file);
	of << sb.GetString();

	of.close();
}