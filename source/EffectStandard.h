#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectStandard final : public Effect
	{
	public:
		EffectStandard(ID3D11Device* pDevice, const std::wstring& assetFile);
		~EffectStandard();

		EffectStandard(const EffectStandard&) = delete;
		EffectStandard(EffectStandard&&) noexcept = delete;
		EffectStandard& operator=(const EffectStandard&) = delete;
		EffectStandard& operator=(EffectStandard&&) noexcept = delete;

		virtual HRESULT LoadInputLayout(ID3D11Device* pDevice, ID3D11InputLayout** ppInputLayout) override;

		void SetRasterizerState(ID3D11RasterizerState* pRasterizerState);

		void SetWorldMatrix(const Matrix& matrix);
		void SetViewInverseMatrix(const Matrix& matrix);

		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);

	private:
		ID3DX11EffectRasterizerVariable* m_pRasterizerVariable{};

		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable{};

		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

		virtual void LoadEffectVariables() override;
	};
}
