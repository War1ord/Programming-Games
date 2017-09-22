#pragma region includes
#include <memory>
#include "DXApp.h"
#include "SpriteBatch.h"
#include "DDSTextureLoader.h"
#include "SimpleMath.h"
#pragma endregion 

#pragma region Classes
class GameApp : public DXApp
{
public:
	GameApp(HINSTANCE inst);
	~GameApp();

	bool Init() override;
	void Update(float dt) override;
	void Render(float dt) override;
protected:
private:
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	ID3D11ShaderResourceView* texture;
};
#pragma endregion

#pragma region Constructors
GameApp::GameApp( HINSTANCE inst ) : DXApp(inst){}
GameApp::~GameApp(){}
#pragma endregion 

#pragma region Implementations
bool GameApp::Init()
{
	if (DXApp::Init()) return false;

	//create sprite batch object
	spriteBatch.reset(new DirectX::SpriteBatch(context));

	//import texture for rendering 
	HR(DirectX::CreateDDSTextureFromFile(device,L"block.png",nullptr,&texture));

	spriteBatch->Begin();

	// draw sprites
	spriteBatch->Draw(texture, DirectX::SimpleMath::Vector2(100,100));

	spriteBatch->End();

	return true;
}
void GameApp::Update( float dt )
{

}
void GameApp::Render( float dt )
{
	context->ClearRenderTargetView(rendertargetview, DirectX::Colors::BlueViolet);

	HR(swapchain->Present(0,0));
}
#pragma endregion

#pragma region WinMain
int __stdcall WinMain(__in HINSTANCE inst, __in_opt HINSTANCE pinst, __in LPSTR cmdline, __in int showcmd)
{
	GameApp game(inst);
	if(!game.Init())return 1;
	return game.Run();
}
#pragma endregion