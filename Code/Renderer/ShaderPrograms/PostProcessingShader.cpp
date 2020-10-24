#include "PostProcessingShader.h"
#include "DXManager.h"
#include "Systems.h"
#include "ShaderHelpers.h"
#include "ScreenQuad.h"
#include "DXDepthStencilStates.h"
#include "DXBlendStates.h"
#include "RenderToTexture.h"
#include "SystemDefs.h"
#include "Entity.h"
#include "QuadComponent.h"
#include <math.h>
#include "DebugQuadHandler.h"

PostProcessingShader::PostProcessingShader()
{
	// create and compile shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/Shaders/PostProcess/vertexPostProcessing.shader",  _vertexPostProcessingShader,   vertexPostProcessingShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/Shaders/PostProcess/pixelPostProcessingHDR.shader", _pixelPostProcessingHDRShader, pixelPostProcessingHDRShaderByteCode);
	SHADER_HELPERS::CreateVertexShader(L"Data/Shaders/PostProcess/vertexBlur.shader",            _vertexBlurShader,             vertexBlurShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/Shaders/PostProcess/pixelBlur.shader",              _pixelBlurShader,              pixelBlurShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/Shaders/PostProcess/pixelPostProcessingSDR.shader", _pixelPostProcessingSDRShader, pixelPostProcessingSDRShaderByteCode);
	SHADER_HELPERS::CreateComputeShader(L"Data/Shaders/PostProcess/computeBrightness.shader",    _computeBrightnessShader,      computeBrightnessShaderByteCode);	

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_blurVertexConstant);
	SHADER_HELPERS::CreateConstantBuffer(_finalPixelConstant);

	// create render textures
	CreateBloomBlurRenderTextures();
	createDofRenderTextures();

	_sceneSDR = new RenderToTexture(SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT, false, false, false, false);

	// create compute shader resources for calculating the brigthness map
	SHADER_HELPERS::CreateTexture2DUAVSRV(SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT, _brightnessResources.Tex, _brightnessResources.SRV, _brightnessResources.UAV);
}

PostProcessingShader::~PostProcessingShader()
{
	_vertexPostProcessingShader->Release();
	_pixelPostProcessingHDRShader->Release();
	_vertexBlurShader->Release();
	_pixelBlurShader->Release();
	_computeBrightnessShader->Release();
	_pixelPostProcessingSDRShader->Release();

	vertexPostProcessingShaderByteCode->Release();
	pixelPostProcessingHDRShaderByteCode->Release();
	vertexBlurShaderByteCode->Release();
	pixelBlurShaderByteCode->Release();
	computeBrightnessShaderByteCode->Release();
	pixelPostProcessingSDRShaderByteCode->Release();

	_brightnessResources.SRV->Release();
	_brightnessResources.UAV->Release();
	_brightnessResources.Tex->Release();
}

void PostProcessingShader::CreateBloomBlurRenderTextures()
{
	ID3D11ShaderResourceView* oldp1 = nullptr;
	ID3D11ShaderResourceView* oldp2 = nullptr; 

	// the resolution of these maps can be updated from in game editor
	// delete old ones if exist and also get ptrs to the textures that 
	// could be used displaying debug quads
	if (_bloomHorizontalBlurPass1)   delete _bloomHorizontalBlurPass1;
	if (_bloomVerticalBlurPass1)   { oldp1 = _bloomVerticalBlurPass1->renderTargetSRV; delete _bloomVerticalBlurPass1;}
	if (_bloomHorizontalBlurPass2) { oldp2 = _bloomVerticalBlurPass2->renderTargetSRV; delete _bloomHorizontalBlurPass2;}
	if (_bloomVerticalBlurPass2)     delete _bloomVerticalBlurPass2;

	// create new render textures
	_bloomHorizontalBlurPass1 = new RenderToTexture(SystemSettings::SCREEN_WIDTH / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1, SystemSettings::SCREEN_HEIGHT / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1, false, SystemSettings::USE_HDR, false);
	_bloomVerticalBlurPass1   = new RenderToTexture(SystemSettings::SCREEN_WIDTH / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1, SystemSettings::SCREEN_HEIGHT / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1, false, SystemSettings::USE_HDR, false);
	_bloomHorizontalBlurPass2 = new RenderToTexture(SystemSettings::SCREEN_WIDTH / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2, SystemSettings::SCREEN_HEIGHT / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2, false, SystemSettings::USE_HDR, false);
	_bloomVerticalBlurPass2   = new RenderToTexture(SystemSettings::SCREEN_WIDTH / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2, SystemSettings::SCREEN_HEIGHT / PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2, false, SystemSettings::USE_HDR, false);

	// check if some of the old textures was used in the debug quads
	// if a match is found it will give it a ptr to the new texture instead
	Systems::renderer->debugQuadHandler->ReplaceTexture(oldp1, _bloomVerticalBlurPass1->renderTargetSRV);
	Systems::renderer->debugQuadHandler->ReplaceTexture(oldp2, _bloomVerticalBlurPass2->renderTargetSRV);
}

void PostProcessingShader::createDofRenderTextures()
{
	// render textures for the dof blur
	_dofHorizontalBlurPass = new RenderToTexture(SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT, false, SystemSettings::USE_HDR, false);
	_dofVerticalBlurPass   = new RenderToTexture(SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT, false, SystemSettings::USE_HDR, false);
}

void PostProcessingShader::Render(ScreenQuad* quad, ID3D11ShaderResourceView* SceneImage, ID3D11ShaderResourceView* sceneDepth)
{
	DXManager& DXM = *Systems::dxManager;
	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_OPAQUE);
	quad->UploadBuffers();

	// unbind our main rendertarget so we can use the SRV as input 
	// when generating our post process scene filters
	DXM.SetNullRenderTarget();

	if (PostProcessing::APPLY_BLOOM)
	{		
		// compute brightness map
		ComputeBrightnessMap(SceneImage);

		// blur the brightness map
		_bloomMap = RenderBlurMap(_brightnessResources.SRV, PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_1, _bloomHorizontalBlurPass1, _bloomVerticalBlurPass1);

		// if set to two pass blur we blur the texture from first pass
		if (PostProcessing::BLOOM_USE_TWO_PASS_BLUR)
			_bloomMap = RenderBlurMap(_bloomVerticalBlurPass1->renderTargetSRV, PostProcessing::BLOOM_BLUR_SCALE_DOWN_PASS_2, _bloomHorizontalBlurPass2, _bloomVerticalBlurPass2);
	}

	if (PostProcessing::APPLY_DEPTH_OF_FIELD)
		_dofMap = RenderBlurMap(SceneImage, 1, _dofHorizontalBlurPass, _dofVerticalBlurPass);

	// render the final post process effects and apply tonemapping
	RenderFinalHDR(SceneImage, sceneDepth);

	// render the final post processing effects that need to be done in LDR space
	RenderFinalSDR(_sceneSDR->renderTargetSRV);
}

void PostProcessingShader::ComputeBrightnessMap(ID3D11ShaderResourceView* originalImage)
{
	// get devicecontext
	ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;

	devCon->CSSetShader(_computeBrightnessShader, NULL, 0);
	devCon->CSSetShaderResources(0, 1, &originalImage);
	devCon->CSSetUnorderedAccessViews(0, 1, &_brightnessResources.UAV, 0);

	UINT threadsX = std::ceil(SystemSettings::SCREEN_WIDTH  / 32);
	UINT threadsY = std::ceil(SystemSettings::SCREEN_HEIGHT / 32);

	// shader is set to [32, 32, 1] threads per group meaning we have 32 * 32 = 1024 threads per group
	// if our screen is 1920 * 1080 we need 2073600 threads to cover all pixels
	// 1920 / 32 will result in 60 thread groups, 1080 / 32 will result in 33.75 thread groups, 60 * 33.75 = 2025 thread groups
	// 2025 groups * 1024 threads = 2073600 threads
	// in case of a 2d texture like this we should just think (width / declared therads.x, height / declared threads.y) and we will and up with the correct amount of threads to use	
	devCon->Dispatch(threadsX, threadsY, 1);

	// unbind so we can use resources as input in next stages
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	devCon->CSSetShaderResources(0, 1, nullSRV);

	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	devCon->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
}

ID3D11ShaderResourceView* PostProcessingShader::RenderBlurMap(ID3D11ShaderResourceView* imageToBlur, float scaleDown, RenderToTexture* horizontal, RenderToTexture* vertical)
{
	// get devicecontext
	ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;

	ID3D11ShaderResourceView* nullSRV[1] = { NULL };

	// set our shaders
	devCon->VSSetShader(_vertexBlurShader, NULL, 0);
	devCon->PSSetShader(_pixelBlurShader, NULL, 0);
	devCon->VSSetConstantBuffers(0, 1, &_blurVertexConstant);
	
	// constantbuffer for vertex shader
	ConstantBlurVertex constantVertex;
	constantVertex.screenWidth  = SystemSettings::SCREEN_WIDTH / scaleDown;
	constantVertex.screenHeight = SystemSettings::SCREEN_HEIGHT / scaleDown;

	//------------------------------------------------------ HORIZONTAL PASS
	// set the horizontal blur render target 1
	horizontal->ClearRenderTarget(0, 0, 0, 1, false);
	horizontal->SetRendertarget(false, false);

	constantVertex.horizontalPass = 1;
	SHADER_HELPERS::UpdateConstantBuffer((void*)&constantVertex, sizeof(ConstantBlurVertex), _blurVertexConstant);

	// set image to blur
	devCon->PSSetShaderResources(0, 1, &imageToBlur);

	// draw horizontal pass
	devCon->DrawIndexed(6, 0, 0);
	
	devCon->PSSetShaderResources(0, 1, nullSRV);

	//------------------------------------------------------ VERTICAL PASS
	// set vertical blur render target 1
	vertical->ClearRenderTarget(0, 0, 0, 1, false);
	vertical->SetRendertarget(false, false);

	// change constant input
	constantVertex.horizontalPass = 0;
	SHADER_HELPERS::UpdateConstantBuffer((void*)&constantVertex, sizeof(ConstantBlurVertex), _blurVertexConstant);

	// get and set the horizontaly blurred texture
	ID3D11ShaderResourceView* hBlur1SRV = horizontal->renderTargetSRV;
	devCon->PSSetShaderResources(0, 1, &hBlur1SRV);

	// draw vertical pass
	devCon->DrawIndexed(6, 0, 0);

	devCon->PSSetShaderResources(0, 1, nullSRV);

	// return blurred image
	return vertical->renderTargetSRV;
}

void PostProcessingShader::RenderFinalHDR(ID3D11ShaderResourceView* SceneImage, ID3D11ShaderResourceView* sceneDepth)
{
	// get dx manager
	DXManager& DXM = *Systems::dxManager;

	// get devicecontext
	ID3D11DeviceContext*& devCon = DXM.devCon;

	_sceneSDR->SetRendertarget(false, false);

	// set our shaders
	devCon->VSSetShader(_vertexPostProcessingShader, NULL, 0);
	devCon->PSSetShader(_pixelPostProcessingHDRShader, NULL, 0);

	devCon->PSSetConstantBuffers(0, 1, &_finalPixelConstant);

	// set pixel constants
	ConstantFinalHDRPixel pixelConstant;
	pixelConstant.applyBloom      = PostProcessing::APPLY_BLOOM;
	pixelConstant.bloomIntensity  = PostProcessing::BLOOM_INTENSITY;
	pixelConstant.applyDof        = PostProcessing::APPLY_DEPTH_OF_FIELD;
	pixelConstant.applyTonemap    = PostProcessing::APPLY_TONEMAPPING;
	pixelConstant.startEndDofdst  = XMFLOAT2(PostProcessing::START_END_DOF_DST[0], PostProcessing::START_END_DOF_DST[1]);
	pixelConstant.tonemapType     = PostProcessing::TONEMAP_TYPE;
	pixelConstant.tonemapExposure = PostProcessing::TONEMAP_EXPOSURE;

	SHADER_HELPERS::UpdateConstantBuffer((void*)&pixelConstant, sizeof(ConstantFinalHDRPixel), _finalPixelConstant);

	// set textures
	ID3D11ShaderResourceView* texArray[4] = { SceneImage, _bloomMap, _dofMap, sceneDepth };
	devCon->PSSetShaderResources(0, 4, texArray);

	// draw
	devCon->DrawIndexed(6, 0, 0);

	// unbind
	ID3D11ShaderResourceView* nullSRV[4] = { NULL, NULL, NULL, NULL };
	devCon->PSSetShaderResources(0, 4, nullSRV);
}

void PostProcessingShader::RenderFinalSDR(ID3D11ShaderResourceView* SceneImageSDR)
{
	// get dx manager
	DXManager& DXM = *Systems::dxManager;

	// get devicecontext
	ID3D11DeviceContext*& devCon = DXM.devCon;

	// set to defult backbuffer and render our final scene image
	DXM.SetBackBuffer();

	// set pixel constants
	ConstantFinalSDRPixel pixelConstant;
	pixelConstant.applyFXAA    = PostProcessing::APPLY_FXAA;	
	pixelConstant.screenWidth  = SystemSettings::SCREEN_WIDTH;
	pixelConstant.screenHeight = SystemSettings::SCREEN_HEIGHT;
	SHADER_HELPERS::UpdateConstantBuffer((void*)&pixelConstant, sizeof(ConstantFinalSDRPixel), _finalPixelConstant);

	devCon->PSSetShader(_pixelPostProcessingSDRShader, NULL, 0);

	devCon->PSSetShaderResources(0, 1, &SceneImageSDR);

	// draw
	devCon->DrawIndexed(6, 0, 0);

	// unbind
	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	devCon->PSSetShaderResources(0, 1, nullSRV);
}

// ENABLE DEBUG QUADS FOR VERIOUS MAPS
void PostProcessingShader::ShowAllDebugQuads()
{
	ShowBrightnessMapDebugQuad();
	ShowBloomBlurP1DebugQuad();
	ShowBloomBlurP2DebugQuad();
	ShowDofMapDebugQuad();
}

void PostProcessingShader::ShowBrightnessMapDebugQuad() { Systems::renderer->debugQuadHandler->AddDebugQuad(_brightnessResources.SRV); }
void PostProcessingShader::ShowBloomBlurP1DebugQuad()   { Systems::renderer->debugQuadHandler->AddDebugQuad(_bloomVerticalBlurPass1->renderTargetSRV); }
void PostProcessingShader::ShowBloomBlurP2DebugQuad()   { Systems::renderer->debugQuadHandler->AddDebugQuad(_bloomVerticalBlurPass2->renderTargetSRV); }
void PostProcessingShader::ShowDofMapDebugQuad()        { Systems::renderer->debugQuadHandler->AddDebugQuad(_dofVerticalBlurPass->renderTargetSRV); }
