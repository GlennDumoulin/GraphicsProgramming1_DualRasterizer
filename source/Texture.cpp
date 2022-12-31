#include "pch.h"
#include "Texture.h"

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice)
		: m_pSurface{ pSurface }
		, m_pSurfacePixels{ static_cast<uint32_t*>(pSurface->pixels) }
	{
		//Create Resource
		DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

		HRESULT hr{ pDevice->CreateTexture2D(&desc, &initData, &m_pResource) };
		if (FAILED(hr))
			return;

		//Create Shader Resource View (SRV)
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
		if (FAILED(hr))
			return;
	}

	Texture::~Texture()
	{
		if (m_pSRV)
			m_pSRV->Release();
		if (m_pResource)
			m_pResource->Release();

		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)
		Texture* temp{ new Texture{ IMG_Load(path.c_str()), pDevice } };

		return temp;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//Sample the correct texel for the given uv
		//calculate current pixel to sample
		const int x{ static_cast<int>(uv.x * m_pSurface->w) };
		const int y{ static_cast<int>(uv.y * m_pSurface->h) };

		const Uint32 pixelIdx{ m_pSurfacePixels[(y * m_pSurface->w) + x] };

		//get RGB-values from the texture for the current pixel
		Uint8 r{};
		Uint8 g{};
		Uint8 b{};

		SDL_GetRGB(pixelIdx, m_pSurface->format, &r, &g, &b);

		//get RGB-values in [0, 1] range instead of [0, 255]
		const float maxValue{ 255.f };
		return ColorRGB{ r / maxValue, g / maxValue, b / maxValue };
	}
}
