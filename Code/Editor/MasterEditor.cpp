#include "MasterEditor.h"
#include "Time.h"
#include "imgui.h"
#include "SystemDefs.h"
#include "CameraManager.h"
#include "TransformComponent.h"
#include "DXManager.h"
#include "HardwareProperties.h"
#include "Systems.h"
#include "FreeMoveComponent.h"

MasterEditor::MasterEditor() :
	_showEditorWindow(false),
	_showStatsWindow(true)
{
	mainWindow           = new MainEditorWindow(this);
	skyDomeWindow        = new SkyDomeWindow(this);
	postProcessWindow    = new PostProcessWindow(this);
	systemSettingsWindow = new SystemSettingsWindow(this);

	activeWindow = mainWindow;
}

MasterEditor::~MasterEditor()
{
}

void MasterEditor::Update()
{
	Input& input = *Systems::input;

	if (input.IskeyPressed(DIK_F9))
		_showEditorWindow = !_showEditorWindow;

	if (input.IskeyPressed(DIK_F8))
		_showStatsWindow = !_showStatsWindow;

	if (_showStatsWindow)  RenderStatsWindow();
	if (_showEditorWindow) RenderEditorWindow();
}

void MasterEditor::RenderStatsWindow()
{
	ImGuiIO& io       = ImGui::GetIO();
	CameraManager& CM = *Systems::cameraManager;
	DXManager& DXM    = *Systems::dxManager;
	Input& input      = *Systems::input;

	TransformComponent* gameCamTransform = CM.currentCameraGame->GetComponent<TransformComponent>();
	TransformComponent* shadowCamTransform = CM.currentCameraDepthMap->GetComponent<TransformComponent>();

	FreeMoveComponent* gameCameraMove = CM.currentCameraGame->GetComponent<FreeMoveComponent>();
	FreeMoveComponent* shadowCameraMove = CM.currentCameraDepthMap->GetComponent<FreeMoveComponent>();

	if (input.IskeyPressed(DIK_F1))
		gameCameraMove->SetActive(!gameCameraMove->IsActive());

	if (input.IskeyPressed(DIK_F2))
		shadowCameraMove->SetActive(!shadowCameraMove->IsActive());

	// get camera position
	const XMFLOAT3& gameCamPos = gameCamTransform->position;
	const XMFLOAT3& gameCamRot = gameCamTransform->rotation;

	const XMFLOAT3& shadowCamPos = shadowCamTransform->position;
	const XMFLOAT3& shadowCamRot = shadowCamTransform->rotation;

	// get hardware info
	const HardwareInfo& info = DXM.hardwareProperties->hardwareInfo;

	// set properties of next window
	ImGui::SetNextWindowBgAlpha(0.3f);
	ImGui::SetNextWindowSize(ImVec2(SystemSettings::SCREEN_WIDTH * 0.15f, SystemSettings::SCREEN_HEIGHT * 0.15f));
	ImGui::SetNextWindowPos(ImVec2(SystemSettings::SCREEN_WIDTH * 0.99f, SystemSettings::SCREEN_HEIGHT * 0.01f), 0, ImVec2(1, 0));

	// create window to display information
	ImGui::Begin("test", nullptr, ImGuiWindowFlags_NoTitleBar);

	// add text properties
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "FPS : %.2f", io.Framerate);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "FrameTime : %.3f", 1000.0f / io.Framerate);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "CAMERA POS : X %.2f Y %.2f Z %.2f", gameCamPos.x, gameCamPos.y, gameCamPos.z);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "CAMERA Rot : X %.2f Y %.2f Z %.2f", gameCamRot.x, gameCamRot.y, gameCamRot.z);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "GPU : %s", info.videoCardDescription);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "SHADOW POS : X %.2f Y %.2f Z %.2f", shadowCamPos.x, shadowCamPos.y, shadowCamPos.z);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "SHADOW Rot : X %.2f Y %.2f Z %.2f", shadowCamRot.x, shadowCamRot.y, shadowCamRot.z);
	ImGui::TextColored(ImVec4(1, 1, 1, 1), "Press F9 to toggle tool window");

	// end rendering of this window
	ImGui::End();
}

void MasterEditor::RenderEditorWindow()
{
	activeWindow->Render();	
}