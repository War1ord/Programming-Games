#pragma region Includes and other
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include "DXUtil.h"
#pragma endregion 

#pragma region Classes
class DXApp
{
public:
	DXApp(HINSTANCE inst);
	virtual ~DXApp(void);

	//main loop
	int Run();
	//framework methods
	virtual bool Init();
	virtual void Update(float dt) = 0;
	virtual void Render(float dt) = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	//win32
	HWND AppWnd;
	HINSTANCE AppInst;
	UINT ClientWidth;
	UINT ClientHeight;
	std::string AppTitle;
	DWORD WndStyle;
	//DirectX
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapchain;
	ID3D11RenderTargetView* rendertargetview;
	D3D_DRIVER_TYPE drivertype;
	D3D_FEATURE_LEVEL featurelevel;
	D3D11_VIEWPORT viewport;
	
protected:
	//init win32
	bool InitWindow();
	//init Direct 3D
	bool InitDirect3D();
};

#pragma endregion 
