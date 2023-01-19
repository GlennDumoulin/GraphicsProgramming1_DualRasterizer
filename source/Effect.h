#pragma once

namespace dae
{
	class Texture;

	class Effect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		virtual HRESULT LoadInputLayout(ID3D11Device* pDevice, ID3D11InputLayout** ppInputLayout) = 0;

		void SetSampler(ID3D11SamplerState* pSampler) const;
		void SetRasterizerState(ID3D11RasterizerState* pRasterizerState) const;

		void SetWorldViewProjMatrix(const Matrix& matrix) const;

		void SetDiffuseMap(const Texture* pDiffuseTexture) const;

		ID3DX11Effect* GetEffect() const
		{
			return m_pEffect;
		}
		ID3DX11EffectTechnique* GetTechnique() const
		{
			return m_pTechnique;
		}

	protected:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};
		ID3DX11EffectSamplerVariable* m_pSamplerVariable{};
		ID3DX11EffectRasterizerVariable* m_pRasterizerVariable{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

		virtual void LoadEffectVariables() = 0;
	};
}
