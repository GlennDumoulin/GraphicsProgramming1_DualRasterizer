#include "pch.h"
#include "Renderer.h"
#include "Camera.h"

namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow)
		: m_pWindow(pWindow)
		, m_hConsole(GetStdHandle(STD_OUTPUT_HANDLE))
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n\n";
		}

		//Initialize Camera
		m_pCamera = new Camera{};
		m_pCamera->Initialize(static_cast<float>(m_Width) / m_Height, 45.f);

		//Create Sampler State
		InitializeSamplerState();

		//Print settings to console
		PrintSettings();
	}
	Renderer::~Renderer()
	{
		//DirectX
		if (m_pSamplerState) m_pSamplerState->Release();
		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();

		//Software
		delete[] m_pDepthBufferPixels;

		//Shared
		delete m_pCamera;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		if (m_IsUsingDirectX)
		{
			RenderDirectX();
		}
		else
		{
			RenderSoftware();
		}
	}
	void Renderer::RenderDirectX() const
	{
		if (!m_IsInitialized)
			return;

		//1. Clear RTV & DSV
		ColorRGB clearColor{};
		if (m_IsUsingUniformClearColor)
		{
			clearColor = m_UniformClearColor;
		}
		else
		{
			clearColor = m_HardwareClearColor;
		}
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. Set Pipeline + Invoke DrawCalls (==Render)
		/*std::for_each(m_pMeshes.begin(), m_pMeshes.end(), [=](Mesh* pMesh)
			{
				pMesh->Render(m_pDeviceContext);
			}
		);*/

		//3. Present Backbuffer (Swap)
		m_pSwapChain->Present(0, 0);
	}
	void Renderer::RenderSoftware() const
	{
		//Reset Depth Buffer
		ResetDepthBuffer();

		//Clear Background
		ClearBackground();

		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		//Loop Over All Meshes
		/*for (Mesh& mesh : m_MeshesWorld)
		{
			//Calculate SCREEN Space Vertices
			VertexTransformationFunction(mesh);

			//Handle Primitive Topology Type
			switch (mesh.primitiveTopology)
			{
			case PrimitiveTopology::TriangleList:
			{
				//Loop Over All Triangles
				for (size_t verticeIdx{}; verticeIdx < mesh.indices.size(); verticeIdx += 3)
				{
					RenderTriangle(verticeIdx, mesh);
				}

				break;
			}

			case PrimitiveTopology::TriangleStrip:
			{
				//Loop Over All Triangles
				for (size_t verticeIdx{}; verticeIdx < mesh.indices.size() - 2; ++verticeIdx)
				{
					RenderTriangle(verticeIdx, mesh, verticeIdx % 2);
				}

				break;
			}
			}
		}*/

		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

#pragma region Software Functions
	inline void Renderer::ClearBackground() const
	{
		ColorRGB clearColor{};
		if (m_IsUsingUniformClearColor)
		{
			clearColor = m_UniformClearColor;
		}
		else
		{
			clearColor = m_SoftwareClearColor;
		}
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format,
			static_cast<uint8_t>(clearColor.r * 255),
			static_cast<uint8_t>(clearColor.g * 255),
			static_cast<uint8_t>(clearColor.b * 255))
		);
	}
	inline void Renderer::ResetDepthBuffer() const
	{
		const int nrPixels{ m_Width * m_Height };
		std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);
	}
#pragma endregion

#pragma region DirectX Functions
	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result{
			D3D11CreateDevice(
				nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
				1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext
			)
		};
		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};

		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		//2. Create SwapChain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version)
			SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain (fr this time)
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//Release DXGI Factory
		if (pDxgiFactory)
			pDxgiFactory->Release();

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		//5. Bind RTV & DSV to Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;
	}
	void Renderer::InitializeSamplerState()
	{
		m_SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		m_SamplerDesc.MipLODBias = 0;
		m_SamplerDesc.MinLOD = 0;
		m_SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		m_SamplerDesc.MaxAnisotropy = 16;

		SetSamplerState();
	}
	void Renderer::SetSamplerState()
	{
		switch (m_SamplerState)
		{
		case SamplerState::POINT:
			m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;

		case SamplerState::LINEAR:
			m_SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;

		case SamplerState::ANISOTROPIC:
			m_SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			break;
		}

		if (m_pSamplerState)
			m_pSamplerState->Release();

		HRESULT result{ m_pDevice->CreateSamplerState(&m_SamplerDesc, &m_pSamplerState) };
		if (FAILED(result))
			return;

		/*std::for_each(m_pMeshes.begin(), m_pMeshes.end(), [=](Mesh* pMesh)
			{
				pMesh->SetSampler(m_pSamplerState);
			}
		);*/
	}
#pragma endregion

#pragma region Toggle & Cycle Functions
	void Renderer::ToggleIsUsingDirectX()
	{
		m_IsUsingDirectX = !m_IsUsingDirectX;

		SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		std::cout << "**(SHARED) Rasterizer Mode = " << (m_IsUsingDirectX ? "HARDWARE" : "SOFTWARE") << "\n";
	}
	void Renderer::ToggleShouldRotate()
	{
		m_ShouldRotate = !m_ShouldRotate;

		SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		std::cout << "**(SHARED) Vehicle Rotation = " << (m_ShouldRotate ? "ON" : "OFF") << "\n";
	}
	void Renderer::ToggleShouldRenderFireFX()
	{
		m_ShouldRenderFireFX = !m_ShouldRenderFireFX;

		SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
		std::cout << "**(HARDWARE) FireFX = " << (m_ShouldRenderFireFX ? "ON" : "OFF") << "\n";
	}
	void Renderer::ToggleIsUsingNormalMap()
	{
		m_IsUsingNormalMap = !m_IsUsingNormalMap;

		SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		std::cout << "**(SOFTWARE) NormapMap = " << (m_IsUsingNormalMap ? "ON" : "OFF") << "\n";
	}
	void Renderer::ToggleShouldShowDepthBuffer()
	{
		m_ShouldShowDepthBuffer = !m_ShouldShowDepthBuffer;

		SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		std::cout << "**(SOFTWARE) DepthBuffer Visualization = " << (m_ShouldShowDepthBuffer ? "ON" : "OFF") << "\n";
	}
	void Renderer::ToggleShouldShowBoundingBox()
	{
		m_ShouldShowBoundingBox = !m_ShouldShowBoundingBox;

		SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		std::cout << "**(SOFTWARE) BoundingBox Visualization = " << (m_ShouldShowBoundingBox ? "ON" : "OFF") << "\n";
	}
	void Renderer::ToggleIsUsingUniformClearColor()
	{
		m_IsUsingUniformClearColor = !m_IsUsingUniformClearColor;

		SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		std::cout << "**(SHARED) Uniform ClearColor = " << (m_IsUsingUniformClearColor ? "ON" : "OFF") << "\n";
	}
	void Renderer::ToggleShouldPrintFPS()
	{
		m_ShouldPrintFPS = !m_ShouldPrintFPS;

		SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		std::cout << "**(SHARED) Print FPS = " << (m_ShouldPrintFPS ? "ON" : "OFF") << "\n";
	}

	void Renderer::CycleSamplerState()
	{
		m_SamplerState = static_cast<SamplerState>((static_cast<int>(m_SamplerState) + 1) % (static_cast<int>(SamplerState::ANISOTROPIC) + 1));

		SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
		std::cout << "**(HARDWARE) Sampler State = ";
		switch (m_SamplerState)
		{
		case SamplerState::POINT:
			std::cout << "Point\n";
			break;

		case SamplerState::LINEAR:
			std::cout << "Linear\n";
			break;

		case SamplerState::ANISOTROPIC:
			std::cout << "Anisotropic\n";
			break;
		}

		SetSamplerState();
	}
	void Renderer::CycleShadingMode()
	{
		m_ShadingMode = static_cast<ShadingMode>((static_cast<int>(m_ShadingMode) + 1) % (static_cast<int>(ShadingMode::Combined) + 1));
		
		SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		std::cout << "**(SOFTWARE) Shading Mode = ";
		switch (m_ShadingMode)
		{
		case ShadingMode::ObservedArea:
			std::cout << "Observed Area\n";
			break;

		case ShadingMode::Diffuse:
			std::cout << "Diffuse\n";
			break;

		case ShadingMode::Specular:
			std::cout << "Specular\n";
			break;

		case ShadingMode::Combined:
			std::cout << "Combined\n";
			break;
		}
	}
	void Renderer::CycleCullMode()
	{
		m_CullMode = static_cast<CullMode>((static_cast<int>(m_CullMode) + 1) % (static_cast<int>(CullMode::NONE) + 1));

		SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		std::cout << "**(SHARED) Cull Mode = ";
		switch (m_CullMode)
		{
		case CullMode::BACK:
			std::cout << "Back\n";
			break;

		case CullMode::FRONT:
			std::cout << "Front\n";
			break;

		case CullMode::NONE:
			std::cout << "None\n";
			break;
		}
	}
#pragma endregion

#pragma region Console Functions
	void Renderer::PrintFPS(float fps) const
	{
		SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
		std::cout << "dFPS: " << fps << "\n";
	}
	void Renderer::PrintControls() const
	{
		//Mouse controls
		SetConsoleTextAttribute(m_hConsole, m_ExtraColor);
		std::cout << "[CONTROLS - MOUSE]\n";

		std::cout << "  [RMB + X]\t\tRotate Yaw\n";
		std::cout << "  [RMB + Y]\t\tRotate Pitch\n";
		std::cout << "  [LMB + X]\t\tRotate Yaw\n";
		std::cout << "  [LMB + Y]\t\tMove (Local) Forward/Backward\n";
		std::cout << "  [LMB + RMB + Y]\tMove (World) Up/Down\n\n";

		//Keyboard controls
		std::cout << "[CONTROLS - KEYBOARD]\n";

		std::cout << "  [W|Arrow Up]\t\tMove (Local) Forward\n";
		std::cout << "  [S|Arrow Down]\tMove (Local) Backward\n";
		std::cout << "  [D|Arrow Right]\tMove (Local) Right\n";
		std::cout << "  [A|Arrow Left]\tMove (Local) Left\n";
		std::cout << "  [SHIFT]\t\tBoost Movement Speed\n\n";

		//Extra controls
		std::cout << "[EXTRA CONTROLS - KEYBOARD]\n";

		std::cout << "  [E]\t\t\tMove (Local) Up\n";
		std::cout << "  [Q]\t\t\tMove (Local) Down\n";
		std::cout << "  [CTRL]\t\tHide Cursor\n";
		std::cout << "  [,|.]\t\t\tChange FOV\n\n";
	}
	void Renderer::PrintSettings() const
	{
		//Shared settings
		SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		std::cout << "[Key Bindings - SHARED]\n";

		std::cout << "  [F1]\tToggle Rasterizer Mode (";
		if (m_IsUsingDirectX)
		{
			std::cout << "HARDWARE/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "SOFTWARE";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "HARDWARE";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/SOFTWARE";
		}
		std::cout << ")\n";

		std::cout << "  [F2]\tToggle Vehicle Rotation (";
		if (m_ShouldRotate)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n";

		std::cout << "  [F9]\tCycle CullMode (";
		switch (m_CullMode)
		{
		case CullMode::BACK:
			std::cout << "BACK/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "FRONT";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "NONE";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			break;

		case CullMode::FRONT:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "BACK";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/FRONT/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "NONE";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			break;

		case CullMode::NONE:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "BACK";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "FRONT";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/NONE";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			break;
		}
		std::cout << ")\n";

		std::cout << "  [F10]\tToggle Uniform ClearColor (";
		if (m_IsUsingUniformClearColor)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n";

		std::cout << "  [F11]\tToggle Print FPS (";
		if (m_ShouldPrintFPS)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_SharedColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n\n";

		//Hardware settings
		SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
		std::cout << "[Key Bindings - HARDWARE]\n";
		
		std::cout << "  [F3]\tToggle FireFX (";
		if (m_ShouldRenderFireFX)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n";

		std::cout << "  [F4]\tCycle Sampler State (";
		switch (m_SamplerState)
		{
		case SamplerState::POINT:
			std::cout << "POINT/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "LINEAR";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ANISOTROPIC";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			break;

		case SamplerState::LINEAR:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "POINT";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			std::cout << "/LINEAR/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ANISOTROPIC";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			break;

		case SamplerState::ANISOTROPIC:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "POINT/";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "LINEAR";
			SetConsoleTextAttribute(m_hConsole, m_HardwareColor);
			std::cout << "/ANISOTROPIC";
			break;
		}
		std::cout << ")\n\n";

		//Software settings
		SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		std::cout << "[Key Bindings - SOFTWARE]\n";
		
		std::cout << "  [F5]\tCycle Shading Mode (";
		switch (m_ShadingMode)
		{
		case ShadingMode::ObservedArea:
			std::cout << "OBSERVED AREA/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "DIFFUSE";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "SPECULAR";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "COMBINED";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			break;

		case ShadingMode::Diffuse:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OBSERVED AREA";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/DIFFUSE/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "SPECULAR";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "COMBINED";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			break;

		case ShadingMode::Specular:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OBSERVED AREA";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "DIFFUSE";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/SPECULAR/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "COMBINED";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			break;

		case ShadingMode::Combined:
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OBSERVED AREA";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "DIFFUSE";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "SPECULAR";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/COMBINED";
			break;
		}
		std::cout << ")\n";

		std::cout << "  [F6]\tToggle NormalMap (";
		if (m_IsUsingNormalMap)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n";

		std::cout << "  [F7]\tToggle DepthBuffer Visualization (";
		if (m_ShouldShowDepthBuffer)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n";

		std::cout << "  [F8]\tToggle BoundingBox Visualization (";
		if (m_ShouldShowBoundingBox)
		{
			std::cout << "ON/";
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "OFF";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
		}
		else
		{
			SetConsoleTextAttribute(m_hConsole, m_DefaultColor);
			std::cout << "ON";
			SetConsoleTextAttribute(m_hConsole, m_SoftwareColor);
			std::cout << "/OFF";
		}
		std::cout << ")\n\n";

		//Extra settings
		SetConsoleTextAttribute(m_hConsole, m_ExtraColor);
		std::cout << "[Key Bindings - EXTRA]\n";

		std::cout << "  [C]\tPrint (Extra) Controls\n";
		std::cout << "  [X]\tReset Console\n\n";
	}
	void Renderer::ClearConsole() const
	{
#if defined(_WIN32)
		system("cls");
#elif defined(__linux__)
		system("clear");
#endif
	}
	void Renderer::ResetConsole() const
	{
		ClearConsole();
		PrintSettings();
	}
#pragma endregion
}
