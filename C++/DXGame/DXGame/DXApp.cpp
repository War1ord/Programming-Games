#pragma region Includes
#include "DXApp.h"
#pragma endregion 

#pragma region Global
namespace
{
	//forward to msgs to user def
	DXApp* pApp = nullptr;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (pApp) return pApp->MsgProc(hwnd, msg, wParam, lParam);
	else return DefWindowProc(hwnd,msg,wParam,lParam);
}
#pragma endregion

#pragma region Constructors
DXApp::DXApp(HINSTANCE inst)
{
	AppInst = inst;
	AppWnd = NULL;
	ClientWidth = 1024;
	ClientHeight = 768;
	AppTitle = "DX Game";
	WndStyle = WS_OVERLAPPEDWINDOW;
	pApp = this;

	device = nullptr;
	context = nullptr;
	rendertargetview = nullptr;
	swapchain = nullptr;
}
DXApp::~DXApp(void)
{
	//clean up directx3d
	if (context) context->ClearState();
	Memory::SafeRelease(rendertargetview);
	Memory::SafeRelease(swapchain);
	Memory::SafeRelease(context);
	Memory::SafeRelease(device);
}
#pragma endregion

#pragma region main loop
int DXApp::Run()
{
	MSG msg = {0};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update(0.0f);
			Render(0.0f);
		}
	}
	return static_cast<int>(msg.wParam);
}
#pragma endregion

#pragma region framework methods
bool DXApp::Init()
{
	if (!InitWindow()) return false;
	if (!InitDirect3D()) return false;
	return true;
}
void DXApp::Update(float dt)
{
}
void DXApp::Render(float dt)
{
}
LRESULT DXApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY: 
		PostQuitMessage(0);
		return 0;
		break;
	default:
		return DefWindowProc(hwnd,msg,wParam,lParam);
		break;
	}
}
#pragma endregion

#pragma region Init
bool DXApp::InitWindow()
{
	WNDCLASSEX w;
	ZeroMemory(&w, sizeof(WNDCLASSEX));
	w.cbClsExtra = 0;
	w.cbWndExtra = 0;
	w.cbSize = sizeof(WNDCLASSEX);
	w.style = CS_HREDRAW | CS_VREDRAW;
	w.hInstance = AppInst;
	w.lpfnWndProc = MainWndProc;
	w.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	w.hCursor = LoadCursor(NULL,IDC_ARROW);
	w.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	w.lpszMenuName = NULL;
	w.lpszClassName = "DXAPPWNDCLASS";
	w.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	if (!RegisterClassEx(&w))
	{
		OutputDebugString("\nFAILED TO CREATE WINDOW\n");
		return false;
	}
	RECT r = {0,0,ClientWidth,ClientHeight};
	AdjustWindowRect(&r, WndStyle, false);
	UINT wigth = r.right - r.left;
	UINT height = r.bottom - r.top;
	UINT x = GetSystemMetrics(SM_CXSCREEN/2 - wigth/2);
	UINT y = GetSystemMetrics(SM_CYSCREEN/2 - height/2);
	AppWnd = CreateWindow("DXAPPWNDCLASS", AppTitle.c_str(), WndStyle,
		x,y,wigth,height,NULL,NULL,AppInst,NULL);
	if (!AppWnd)
	{
		OutputDebugString("\nFAILED TO CREATE WINDOW\n");
		return false;
	}
	ShowWindow(AppWnd, SW_SHOW);
	return true;
}
bool DXApp::InitDirect3D()
{
	UINT CreateDeviceFlags = 0;

#ifdef _DEBUG
	CreateDeviceFlags != D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	D3D_DRIVER_TYPE drivertypes[]=
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numdrivertypes = ARRAYSIZE(drivertypes);

	D3D_FEATURE_LEVEL featureleverls[]=
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3
	};
	UINT numfeatureleverls = ARRAYSIZE(featureleverls);

	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1; // double buffered
	swapDesc.BufferDesc.Width = ClientWidth;
	swapDesc.BufferDesc.Height = ClientHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = AppWnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //alt-enter fullscreen

	HRESULT result;
	for (int i = 0; i < numdrivertypes; ++i)
	{
		result = D3D11CreateDeviceAndSwapChain(NULL,drivertypes[i],NULL,CreateDeviceFlags,featureleverls,numfeatureleverls,D3D11_SDK_VERSION,&swapDesc,&swapchain,&device,&featurelevel,&context);
		if (SUCCEEDED(result))
		{
			drivertype = drivertypes[i];
			break;
		}
		if (FAILED(result))
		{
			OutputDebugString("FAILED TO CREATE DEVICE AND SWAP CHAIN");
			return false;
		}

		//create render target view
		ID3D11Texture2D* backbuff = 0;
		HR(swapchain->GetBuffer(NULL,__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuff)));
		HR(device->CreateRenderTargetView(backbuff, nullptr, &rendertargetview));
		Memory::SafeRelease(backbuff);

		//bind render target view
		context->OMSetRenderTargets(1,&rendertargetview, nullptr);

		//view port creation
		viewport.Width = static_cast<float>(ClientWidth);
		viewport.Height = static_cast<float>(ClientHeight);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// bind view port
		context->RSSetViewports(1,&viewport);


		return true;
	}
}
#pragma endregion
