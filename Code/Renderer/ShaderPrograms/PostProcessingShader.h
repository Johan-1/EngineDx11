#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

class ScreenQuad;
class RenderToTexture;

class PostProcessingShader
{
public:
	PostProcessingShader();
	~PostProcessingShader();

	// renders all meshes to the depth map
	void Render(ScreenQuad* quad, ID3D11ShaderResourceView* SceneImage, ID3D11ShaderResourceView* sceneDepth);

	void CreateBloomBlurRenderTextures();
	void createDofRenderTextures();

	void ShowAllDebugQuads();
	void ShowBrightnessMapDebugQuad();
	void ShowBloomBlurP1DebugQuad();
	void ShowBloomBlurP2DebugQuad();
	void ShowDofMapDebugQuad();

	// the shader bytecode
	ID3D10Blob* vertexPostProcessingShaderByteCode;
	ID3D10Blob* pixelPostProcessingHDRShaderByteCode;
	ID3D10Blob* vertexBlurShaderByteCode;
	ID3D10Blob* pixelBlurShaderByteCode;
	ID3D10Blob* pixelPostProcessingSDRShaderByteCode;
	ID3D10Blob* computeBrightnessShaderByteCode;

	ID3D11ShaderResourceView* RenderBlurMap(ID3D11ShaderResourceView* imageToBlur, float scaleDown1, RenderToTexture* h1, RenderToTexture* v1);
private:

	struct ComputeResources
	{
		ID3D11Texture2D*           Tex;
		ID3D11ShaderResourceView*  SRV;
		ID3D11UnorderedAccessView* UAV;
	};

	void ComputeBrightnessMap(ID3D11ShaderResourceView* originalImage);
	void RenderFinalHDR(ID3D11ShaderResourceView* SceneImage, ID3D11ShaderResourceView* sceneDepth);
	void RenderFinalSDR(ID3D11ShaderResourceView* SceneImageSDR);

	// compiled shaders
	ID3D11VertexShader*  _vertexBlurShader;
	ID3D11PixelShader*   _pixelBlurShader;
	ID3D11VertexShader*  _vertexPostProcessingShader;
	ID3D11PixelShader*   _pixelPostProcessingHDRShader;
	ID3D11PixelShader*   _pixelPostProcessingSDRShader;
	ID3D11ComputeShader* _computeBrightnessShader;

	// constant buffers
	ID3D11Buffer* _blurVertexConstant;
	ID3D11Buffer* _finalPixelConstant;

	// render textures 
	RenderToTexture* _bloomHorizontalBlurPass1;
	RenderToTexture* _bloomVerticalBlurPass1;
	RenderToTexture* _bloomHorizontalBlurPass2;
	RenderToTexture* _bloomVerticalBlurPass2;
	RenderToTexture* _dofHorizontalBlurPass;
	RenderToTexture* _dofVerticalBlurPass;
	RenderToTexture* _sceneSDR;

	// compute resources
	ComputeResources _brightnessResources;

	// final input maps to post processing shader
	ID3D11ShaderResourceView* _bloomMap;
	ID3D11ShaderResourceView* _dofMap;

	struct ConstantBlurVertex
	{
		float screenWidth;
		float screenHeight;
		int   horizontalPass;
		int   pad; 
	};

	struct ConstantFinalHDRPixel
	{
		int      applyBloom;
		float    bloomIntensity;
		int      applyDof;
		int      applyTonemap;
		XMFLOAT2 startEndDofdst;
		int      tonemapType;
		float    tonemapExposure;
	};

	struct ConstantFinalSDRPixel
	{
		int   applyFXAA;
		float screenWidth;
		float screenHeight;
		float pad;
	};
};

