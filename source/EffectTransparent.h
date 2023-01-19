#pragma once
#include "Effect.h"

namespace dae
{
	class EffectTransparent final : public Effect
	{
	public:
		EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile);
		~EffectTransparent() = default;

		EffectTransparent(const EffectTransparent&) = delete;
		EffectTransparent(EffectTransparent&&) noexcept = delete;
		EffectTransparent& operator=(const EffectTransparent&) = delete;
		EffectTransparent& operator=(EffectTransparent&&) noexcept = delete;

		virtual HRESULT LoadInputLayout(ID3D11Device* pDevice, ID3D11InputLayout** ppInputLayout) override;

	private:
		virtual void LoadEffectVariables() override;
	};
}
