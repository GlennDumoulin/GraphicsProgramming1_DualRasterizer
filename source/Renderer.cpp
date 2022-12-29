#include "pch.h"
#include "Renderer.h"

namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow)
		: m_pWindow(pWindow)
		, m_hConsole(GetStdHandle(STD_OUTPUT_HANDLE))
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

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

		//Print settings to console
		PrintSettings();
	}
	Renderer::~Renderer()
	{
		
	}

	void Renderer::Update(const Timer* pTimer)
	{

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
		//...
	}
	void Renderer::RenderSoftware() const
	{
		//...
	}

	HRESULT Renderer::InitializeDirectX()
	{
		return S_FALSE;
	}

#pragma region Toggle & Cycle functions
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

#pragma region Console functions
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
