#pragma once
#include "CameraComponent.h"

class CameraManager
{
public:
	CameraManager();
	~CameraManager();

	// camera component pointer
	CameraComponent* currentCameraGame;
	CameraComponent* currentCameraUI;
	CameraComponent* currentCameraDepthMap;
};

