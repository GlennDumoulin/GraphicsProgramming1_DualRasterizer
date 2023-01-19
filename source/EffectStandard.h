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

		void SetWorldMatrix(const Matrix& matrix) const;
		void SetViewInverseMatrix(const Matrix& matrix) const;

		void SetNormalMap(const Texture* pNormalTexture) const;
		void SetSpecularMap(const Texture* pSpecularTexture) const;
		void SetGlossinessMap(const Texture* pGlossinessTexture) const;

	private:
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable{};

		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

		virtual void LoadEffectVariables() override;
	};
}
