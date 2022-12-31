#pragma once

namespace dae
{
	class Texture final
	{
	public:
		~Texture();

		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept = delete;

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);
		ColorRGB Sample(const Vector2& uv) const;

		ID3D11ShaderResourceView* GetSRV() const
		{
			return m_pSRV;
		}

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		//DirectX
		ID3D11Texture2D* m_pResource{};
		ID3D11ShaderResourceView* m_pSRV{};

		//Software
		SDL_Surface* m_pSurface{};
		uint32_t* m_pSurfacePixels{};
	};
}
