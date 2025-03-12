#include "SkyDomeWindow.h"
#include "MasterEditor.h"
#include "imgui.h"
#include "Systemdefs.h"
#include "SkyDome.h"
#include "Systems.h"
#include "Renderer.h"
#include "TexturePool.h"
#include "GUI.h"
#include "JsonHelpers.h"
#include <fstream>

SkyDomeWindow::SkyDomeWindow(MasterEditor* masterEditor) : IEditorWindow(masterEditor)
{
}

SkyDomeWindow::~SkyDomeWindow()
{
}

void SkyDomeWindow::Render()
{
	SkyDome*& skyDome         = Systems::renderer->skyDome;
	SkySettings* skySettings = &skyDome->skySettings;
	SunMoon* sunMoon         = &skyDome->sunMoon;
	ID3D11ShaderResourceView* previewTex = Systems::texturePool->GetTexture(L"Data/Textures/domeMap.dds", false);

	GUI::BeginWindow("SkyDome Settings", 0.6f, SystemSettings::SCREEN_WIDTH * 0.20f, SystemSettings::SCREEN_HEIGHT * 0.9f , 0, 0, 0, 0);

	// decription image
	GUI::Image(previewTex, ImVec2(128, 128));

	// mode
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.10f);
	GUI::ComboBox("0", "SkyDome Mode", "the renderering mode for the skydome", (int*)&skySettings->RENDER_MODE, "THREE LAYER COLOR\0CUBE MAP");
	GUI::ClearItemWidth();

	// speed 
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.10f);
	GUI::FloatSlider("1", "Time Multiplier", "one cycle is set to 1 minute, alter the duration with this multiplier", 0.0f, 20.0f, &skySettings->speedMultiplier);

	 // cycle progress
	GUI::FloatSlider("2", "Cycle Progress", "Set the progress of the cycle, use this to controll where the cycle will start", 0.0f, 60.0f, &skySettings->cycleTimer);
	GUI::ClearItemWidth();

	GUI::Space(1);
	if (skySettings->RENDER_MODE == SKY_DOME_RENDER_MODE::THREE_LAYER_COLOR_BLEND) RenderColorBlend(skySettings);
	else                                                                           RenderCubeMap(skySettings);

	RenderLightning(skySettings);
	RenderSunMoon(sunMoon, skySettings);
	
	GUI::Space(1);
	if (GUI::Button("Save Settings"))
	{
		std::string file = GUI::FindFileFromDirectory(".json\0*.json", "save SkyDome Settings as .json");
		if (file != "")
			SaveSettings(file.c_str(), skySettings, sunMoon);
	}
		
	if (GUI::Button("Load Settings"))
	{
		std::string file = GUI::FindFileFromDirectory(".json\0*.json", "load SkyDome Settings from .json");
		if (file != "")
			skyDome->ReadSettings(file.c_str());
	}

	GUI::Space(1);
	if (GUI::Button("Back"))
		GoToMain();

	// end rendering of this window
	GUI::EndWindow();
}

void SkyDomeWindow::RenderColorBlend(SkySettings* skySettings)
{
	// sky color layers percentage
	GUI::Text(ImVec4(1, 1, 1, 1), "SKY LAYERS USAGE PERCENT");
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.05f);
	GUI::FloatSlider("3", "Bottom Layer Percent", "this color will be used from 0 to set value",      0.0f,                          skySettings->skyMidColor.w, &skySettings->skyBottomColor.w);
	GUI::FloatSlider("4", "Mid Layer Percent",    "this color will be used from bottom to set value", skySettings->skyBottomColor.w, skySettings->skyTopColor.w, &skySettings->skyMidColor.w);
	GUI::FloatSlider("5", "Top Layer Percent",    "this color will be used from mid to set value",    skySettings->skyMidColor.w,    100.0f,                     &skySettings->skyTopColor.w);
	GUI::ClearItemWidth();

	// sky colors
	GUI::Space(10);
	GUI::Text(ImVec4(1, 1, 1, 1), "SKY COLORS");
	GUI::Text(ImVec4(0.9f, 0.9f, 0.9f, 1), "Top Colors:");
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("6", "Day", "the color of the top part of skydome during day", "pick1", &skySettings->topSkyColorDay);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("7", "Sunset", "the color of the top part of skydome during Sunset", "pick2", &skySettings->topSkyColorSunSet);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("8", "Night", "the color of the top part of skydome during Night", "pick3", &skySettings->topSkyColorNight);

	GUI::Text(ImVec4(0.9f, 0.9f, 0.9f, 1), "Mid Colors:");
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("9", "Day", "the color of the mid part of skydome during day", "pick4", &skySettings->midSkyColorDay);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("10", "Sunset", "the color of the mid part of skydome during Sunset", "pick5", &skySettings->midSkyColorSunSet);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("11", "Night", "the color of the mid part of skydome during Night", "pick6", &skySettings->midSkyColorNight);

	GUI::Text(ImVec4(0.9f, 0.9f, 0.9f, 1), "Cur Colors:");
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("12", "Bottom", "the color of the bottom part of skydome", "pick7", &skySettings->skyBottomColor);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("13", "Mid", "the color of the middle part of skydome", "pick8", &skySettings->skyMidColor);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("14", "Top", "the color of the top part of skydome", "pick9", &skySettings->skyTopColor);

	// blend timings
	GUI::Space(10);
	GUI::Text(ImVec4(1, 1, 1, 1), "SKY COLORS BLEND TIMINGS");
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.06f);
	GUI::FloatSlider2("15",  "Blend Top Sunset", "when the top sky color will start blending from day to sunset color",   -1.0f, 1.0f, &skySettings->sunsetTopSkyColorStartEndBlend.x);
	GUI::FloatSlider2("16",  "Blend Top Night",  "when the top sky color will start blending from sunset to night color", -1.0f, 1.0f, &skySettings->nightTopSkyColorStartEndBlend.x);
	GUI::FloatSlider2("17",  "Blend Mid Sunset", "when the Mid sky color will start blending from day to sunset color",   -1.0f, 1.0f, &skySettings->sunsetMidSkyColorStartEndBlend.x);
	GUI::FloatSlider2("18",  "Blend Mid Night",  "when the Mid sky color will start blending from sunset to night color", -1.0f, 1.0f, &skySettings->nightMidSkyColorStartEndBlend.x);
	GUI::ClearItemWidth();
}

void SkyDomeWindow::RenderCubeMap(SkySettings* skySettings)
{
	if (ImGui::Button("Load CubeMap"))
	{
		std::string path = GUI::FindFileFromDirectory(".dds", "Load CubeMap");
		// if a file was selected change the texture
		if (path != "")
		{
			// add our relative path to our skyboxes
			skySettings->cubeMapName = GUI::ReplaceWithRelativePath("SkyBoxes/", path);
			
			// set the new cubemap
			Systems::renderer->skyDome->LoadCubemap();
		}		
	}
}

void SkyDomeWindow::RenderLightning(SkySettings* skySettings)
{
	// lightning settings
	GUI::Space(10);
	GUI::Text(ImVec4(1, 1, 1, 1), "LIGHTNING SETTINGS");
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.05f);
	GUI::FloatSlider("19", "Switch to moonLight threshold", "at this sun height we will change to cast light from the moon", -1.0f, 0.0f, &skySettings->switchToMoonLightThreshold);

	GUI::Space(3);
	GUI::FloatSlider2("20", "Blend Light Sunset", "when the dir light color will start to blend to sunset light color", -1.0f, 1.0f, &skySettings->sunsetLightColorStartEndBlend.x);
	GUI::FloatSlider2("21", "Blend Light Night",  "when the dir light color will start to blend to night light color",  -1.0f, 1.0f, &skySettings->nightLightColorStartEndBlend.x);
	GUI::FloatSlider2("22", "Day Light Fade",     "when the day light will start and finish to fadeout",                -1.0f, 1.0f, &skySettings->dayLightStartEndfade.x);
	GUI::FloatSlider2("23", "Night Light Fade",   "when the night light will start and finish to fadeout",              -1.0f, 1.0f, &skySettings->nightLightStartEndfade.x);

	GUI::Space(3);
	GUI::ColorPickerNoAlpha("24", "day dir light Color",    "the color of the directional light at day",    "pick10", &skySettings->normalDirLightColor);
	GUI::ColorPickerNoAlpha("25", "sunset dir light Color", "the color of the directional light at sunset", "pick11", &skySettings->sunsetDirLightColor);
	GUI::ColorPickerNoAlpha("26", "night dir light Color",  "the color of the directional light at night",  "pick12", &skySettings->nightDirLightColor);
	GUI::ClearItemWidth();	
}

void SkyDomeWindow::RenderSunMoon(SunMoon* sunMoon, SkySettings* skySettings)
{
	GUI::Space(10);
	GUI::Text(ImVec4(1, 1, 1, 1), "SUN/MOON SETTINGS");
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.08f);
	GUI::FloatSlider("27", "Y-Rotation", "the rotation of the sun around the y axis", 0.0f, 360.0f, &skySettings->startRotation.y);
	float shadowDst = skySettings->shadowMapDistance.x;
	GUI::FloatInput("28", "ShadowMap distance", "the distance from where the shadow camera is rendering", &shadowDst);
	skySettings->shadowMapDistance = XMFLOAT3(shadowDst, shadowDst, shadowDst);
	GUI::ClearItemWidth();

	GUI::Space(1);
	GUI::Text(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Sun Color Tints:");
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("29", "Day", "color tint during day", "pick13", &sunMoon->sun.dayColorTint);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("30", "SunSet", "color tint during SunSet", "pick14", &sunMoon->sun.sunsetColorTint);

	GUI::Text(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Moon Color Tints:");
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("31", "Night", "color tint during day", "pick15", &sunMoon->moon.dayColorTint);
	GUI::SameLine();
	GUI::ColorPickerNoAlpha("32", "SunSet", "color tint during SunSet", "pick16", &sunMoon->moon.sunsetColorTint);
	
	GUI::Space(1);
	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.12f);
	GUI::FloatSlider2("33", "Sun Color Blend",  "when the sun will start blending between day to sunset colors",     -1.0f, 1.0f, &sunMoon->sun.beginEndColorBlend.x);
	GUI::FloatSlider2("34", "Moon Color Blend", "when the moon will start blending between night to sunrise colors", -1.0f, 1.0f, &sunMoon->moon.beginEndColorBlend.x);
	GUI::FloatSlider2("35", "Sun Fade Out",     "when the sun will start and end in full transparancy",              -1.0f, 1.0f, &sunMoon->sun.beginEndFade.x);
	GUI::FloatSlider2("36", "Moon fade Out",    "when the moon will start and end in full transparancy",             -1.0f, 1.0f, &sunMoon->moon.beginEndFade.x);

	GUI::Space(1);
	GUI::FloatSlider2("37", "Sun Min/Max dst",  "min and max distances of the sun",                   0.0f, 20.0f, &sunMoon->sun.minMaxDst.x);
	GUI::FloatSlider2("38", "Moon Min/Max dst", "min and max distances of the moon",                  0.0f, 20.0f, &sunMoon->moon.minMaxDst.x);
	GUI::FloatSlider2("39", "Sun Dst Blend",    "when sun will start blending from max to min dst",  -1.0f, 1.0f,  &sunMoon->sun.beginEndDstLerp.x);
	GUI::FloatSlider2("40", "Moon Dst Blend",   "when moon will start blending from max to min dst", -1.0f, 1.0f,  &sunMoon->moon.beginEndDstLerp.x);

	GUI::ClearItemWidth();
}

void SkyDomeWindow::SaveSettings(const char* file, SkySettings* skySettings, SunMoon* sunMoon)
{
	// create string buffer and json writer
	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

	// start the write
	writer.StartObject();

	JSON::WriteFloat(&writer, "speedMultiplier",   skySettings->speedMultiplier);
	JSON::WriteFloat(&writer, "cycleTimer",        skySettings->cycleTimer);
	JSON::WriteFloat(&writer, "switchToMoonLightThreshold", skySettings->switchToMoonLightThreshold);

	JSON::WriteFloat2(&writer, "sunsetLightColorStartEndBlend",  skySettings->sunsetLightColorStartEndBlend);
	JSON::WriteFloat2(&writer, "nightLightColorStartEndBlend",   skySettings->nightLightColorStartEndBlend);
	JSON::WriteFloat2(&writer, "sunsetTopSkyColorStartEndBlend", skySettings->sunsetTopSkyColorStartEndBlend);
	JSON::WriteFloat2(&writer, "nightTopSkyColorStartEndBlend",  skySettings->nightTopSkyColorStartEndBlend);
	JSON::WriteFloat2(&writer, "sunsetMidSkyColorStartEndBlend", skySettings->sunsetMidSkyColorStartEndBlend);
	JSON::WriteFloat2(&writer, "nightMidSkyColorStartEndBlend",  skySettings->nightMidSkyColorStartEndBlend);

	JSON::WriteFloat2(&writer, "dayLightStartEndfade",   skySettings->dayLightStartEndfade);
	JSON::WriteFloat2(&writer, "nightLightStartEndfade", skySettings->nightLightStartEndfade);

	JSON::WriteFloat3(&writer, "shadowMapDistance", skySettings->shadowMapDistance);
	JSON::WriteFloat3(&writer, "startRotation",     skySettings->startRotation);
	JSON::WriteFloat3(&writer, "endRotation",       skySettings->endRotation);

	JSON::WriteFloat4(&writer, "normalDirLightColor", skySettings->normalDirLightColor);
	JSON::WriteFloat4(&writer, "sunsetDirLightColor", skySettings->sunsetDirLightColor);
	JSON::WriteFloat4(&writer, "nightDirLightColor",  skySettings->nightDirLightColor);
	JSON::WriteFloat4(&writer, "topSkyColorDay",      skySettings->topSkyColorDay);
	JSON::WriteFloat4(&writer, "topSkyColorSunSet",   skySettings->topSkyColorSunSet);
	JSON::WriteFloat4(&writer, "topSkyColorNight",    skySettings->topSkyColorNight);
	JSON::WriteFloat4(&writer, "midSkyColorDay",      skySettings->midSkyColorDay);
	JSON::WriteFloat4(&writer, "midSkyColorSunSet",   skySettings->midSkyColorSunSet);
	JSON::WriteFloat4(&writer, "midSkyColorNight",    skySettings->midSkyColorNight);
	JSON::WriteFloat4(&writer, "skyBottomColor",      skySettings->skyBottomColor);

	JSON::WriteInt(&writer,   "RENDER_MODE", skySettings->RENDER_MODE);
	JSON::WriteString(&writer, "cubeMap",    skySettings->cubeMapName.c_str());

	JSON::WriteFloat3(&writer, "sunDayColorTint",     sunMoon->sun.dayColorTint);
	JSON::WriteFloat3(&writer, "moonDayColorTint",    sunMoon->moon.dayColorTint);
	JSON::WriteFloat3(&writer, "sunSunsetColorTint",  sunMoon->sun.sunsetColorTint);
	JSON::WriteFloat3(&writer, "moonSunsetColorTint", sunMoon->moon.sunsetColorTint);

	JSON::WriteFloat2(&writer, "sunBeginEndfade",  sunMoon->sun.beginEndFade);
	JSON::WriteFloat2(&writer, "moonBeginEndfade", sunMoon->moon.beginEndFade);

	JSON::WriteFloat2(&writer, "sunMinMaxDst",  sunMoon->sun.minMaxDst);
	JSON::WriteFloat2(&writer, "moonMinMaxDst", sunMoon->moon.minMaxDst);

	JSON::WriteFloat2(&writer, "sunBeginEndDstLerp",  sunMoon->sun.beginEndDstLerp);
	JSON::WriteFloat2(&writer, "moonBeginEndDstLerp", sunMoon->moon.beginEndDstLerp);

	JSON::WriteFloat2(&writer, "sunBeginEndColorBlend",  sunMoon->sun.beginEndColorBlend);
	JSON::WriteFloat2(&writer, "moonBeginEndColorBlend", sunMoon->moon.beginEndColorBlend);

	// end object
	writer.EndObject();

	// create output file stream and write
	// out the json stringBuilder object to the file
	std::ofstream of(file);
	of << sb.GetString();

	of.close();
}

