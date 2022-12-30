#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Camera;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void PrintFPS(float fps) const;
		void PrintControls() const;
		void ResetConsole() const;

#pragma region Toggle & Cycle Functions
		void ToggleIsUsingDirectX(); //F1
		void ToggleShouldRotate(); //F2
		void ToggleShouldRenderFireFX(); //F3
		void ToggleIsUsingNormalMap(); //F6
		void ToggleShouldShowDepthBuffer(); //F7
		void ToggleShouldShowBoundingBox(); //F8
		void ToggleIsUsingUniformClearColor(); //F10
		void ToggleShouldPrintFPS(); //F11

		void CycleSamplerState(); //F4
		void CycleShadingMode(); //F5
		void CycleCullMode(); //F9
#pragma endregion

#pragma region Getter Functions
		bool GetIsUsingDirectX() const { return m_IsUsingDirectX; }
		bool GetShouldPrintFPS() const { return m_ShouldPrintFPS; }
#pragma endregion

	private:
		SDL_Window* m_pWindow{};
		const HANDLE m_hConsole{};
		
		//Color variables
		const WORD m_DefaultColor{ FOREGROUND_INTENSITY };
		const WORD m_SharedColor{ FOREGROUND_RED | FOREGROUND_GREEN };
		const WORD m_HardwareColor{ FOREGROUND_GREEN };
		const WORD m_SoftwareColor{ FOREGROUND_BLUE | FOREGROUND_RED };
		const WORD m_ExtraColor{ FOREGROUND_BLUE | FOREGROUND_INTENSITY };

		const ColorRGB m_UniformClearColor{ .1f, .1f, .1f };
		const ColorRGB m_HardwareClearColor{ .39f, .59f, .93f };
		const ColorRGB m_SoftwareClearColor{ .39f, .39f, .39f };

		int m_Width{};
		int m_Height{};

		Camera* m_pCamera{};

		const float m_MeshRotateSpeed{ 45.f };

		bool m_IsInitialized{ false };

		//Toggle variables
		bool m_IsUsingDirectX{ true }; //F1
		bool m_ShouldRotate{ true }; //F2
		bool m_ShouldRenderFireFX{ true }; //F3
		bool m_IsUsingNormalMap{ true }; //F6
		bool m_ShouldShowDepthBuffer{ false }; //F7
		bool m_ShouldShowBoundingBox{ false }; //F8
		bool m_IsUsingUniformClearColor{ false }; //F10
		bool m_ShouldPrintFPS{ true }; //F11

		//Cycle variables
		SamplerState m_SamplerState{ SamplerState::POINT }; //F4
		ShadingMode m_ShadingMode{ ShadingMode::Combined }; //F5
		CullMode m_CullMode{ CullMode::BACK }; //F9

		void PrintSettings() const;
		void ClearConsole() const;

		void RenderDirectX() const;
		void RenderSoftware() const;

		//Software
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		void ClearBackground() const;
		void ResetDepthBuffer() const;

		//DIRECTX
		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		ID3D11SamplerState* m_pSamplerState{};

		D3D11_SAMPLER_DESC m_SamplerDesc{};

		HRESULT InitializeDirectX();
		void InitializeSamplerState();
		void SetSamplerState();
	};
}
