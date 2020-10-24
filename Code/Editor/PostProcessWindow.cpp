#include "PostProcessWindow.h"
#include "MasterEditor.h"
#include "GUI.h"
#include "SystemDefs.h"
#include "Systems.h"
#include "PostProcessingShader.h"

PostProcessWindow::PostProcessWindow(MasterEditor* masterEditor) : IEditorWindow(masterEditor)
{
}

PostProcessWindow::~PostProcessWindow()
{
}

void PostProcessWindow::Render()
{
	GUI::BeginWindow("Post Process Settings", 0.6f, SystemSettings::SCREEN_WIDTH * 0.22f, SystemSettings::SCREEN_HEIGHT * 0.5f, 0, 0, 0, 0);

	GUI::Space(1);

	// BLOOM
	GUI::Text(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "BLOOM");
	GUI::CheckBox("1", "Apply Bloom",   "if bloom will be applied to final image",          &PostProcessing::APPLY_BLOOM);
	GUI::CheckBox("2", "Two Pass Blur", "if we will do a two pass blur for better quality", &PostProcessing::BLOOM_USE_TWO_PASS_BLUR);

	GUI::SetItemWidth(SystemSettings::SCREEN_WIDTH * 0.10f);
	GUI::FloatSlider("3", "Intensity", "How strong the bloom effect will be", 0.1f, 10.0f, &PostProcessing::BLOOM_INTENSITY);

	GUI::FloatSlider("4", "Res Scale Down P1", "How much we will lower the res output blurred image on first pass",  1.0f, 10.0f, &PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1);
	GUI::SameLine(); if (GUI::Button("Apply1")) Systems::renderer->postProcessingShader->CreateBloomBlurRenderTextures();
	GUI::FloatSlider("5", "Res Scale Down P2", "How much we will lower the res output blurred image on second pass", 1.0f, 10.0f, &PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2);
	GUI::SameLine(); if (GUI::Button("Apply2")) Systems::renderer->postProcessingShader->CreateBloomBlurRenderTextures();
	GUI::ClearItemWidth();

	GUI::Space(2);

	// DEPTH OF FIELD
	GUI::Text(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "DEPTH OF FIELD");
	GUI::CheckBox("6", "Apply Depth Of Field", "if Doof will be applied to final image", &PostProcessing::APPLY_DEPTH_OF_FIELD);
	GUI::FloatSlider2("7", "Start End Focus", "when the focus will start being blurred to when completely blurred", 0.0f, 1000.0f, &PostProcessing::START_END_DOF_DST[0]);

	GUI::Space(2);

	// TONEMAPPING
	GUI::Text(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "TONEMAPPING");
	GUI::CheckBox("7", "Apply Tonemap", "if tonemapping will be applied when going from HDR 16 bit per channel render targets to defualt 8 bit per channel backbuffer", &PostProcessing::APPLY_TONEMAPPING);

	if (PostProcessing::APPLY_TONEMAPPING)
	{
		GUI::ComboBox("8", "TONEMAP TYPE", "thye type of tonemapping algorithm to use", "NAUGHTY DOG\0REINHARD\0EXPOSURE BASED", &PostProcessing::TONEMAP_TYPE);
		if (PostProcessing::TONEMAP_TYPE == 2)
			GUI::FloatSlider("9", "EXPOSURE AMOUNT", "the exposure amount, lower == darker", 0.1f, 1.5f, &PostProcessing::TONEMAP_EXPOSURE);
	}

	GUI::Space(2);

	// FXAA
	GUI::Text(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "FXAA");
	GUI::CheckBox("10", "Apply FXAA", "if FXAA will be performed on the final scene image", &PostProcessing::APPLY_FXAA);

	GUI::Space(1);

	// serialize settings
	if (GUI::Button("Save Settings"))
	{
		std::string file = GUI::FindFileFromDirectory(".json", "save post processing settings");
		if (file != "")
			PostProcessing::WriteSettings(file.c_str());
	}

	// deserialize settings
	if (GUI::Button("Load Settings"))
	{
		std::string file = GUI::FindFileFromDirectory(".json", "save post processing settings");
		if (file != "")
			PostProcessing::ReadSettings(file.c_str());
	}

	GUI::Space(1);
	if (GUI::Button("Back"))
		GoToMain();

	GUI::EndWindow();
}