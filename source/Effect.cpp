#include "pch.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		: m_pEffect{ LoadEffect(pDevice, assetFile) }
	{
	}
	Effect::~Effect()
	{
		if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->Release();
		if (m_pMatWorldViewProjVariable) m_pMatWorldViewProjVariable->Release();
		if (m_pRasterizerVariable) m_pRasterizerVariable->Release();
		if (m_pSamplerVariable) m_pSamplerVariable->Release();
		if (m_pTechnique) m_pTechnique->Release();
		if (m_pEffect) m_pEffect->Release();
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags{ 0 };
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(
			assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob
		);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << "\n";
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << "\n";
				return nullptr;
			}
		}

		return pEffect;
	}

	void Effect::SetSampler(ID3D11SamplerState* pSampler) const
	{
		m_pSamplerVariable->SetSampler(0, pSampler);
	}
	void Effect::SetRasterizerState(ID3D11RasterizerState* pRasterizerState) const
	{
		m_pRasterizerVariable->SetRasterizerState(0, pRasterizerState);
	}

	void Effect::SetWorldViewProjMatrix(const Matrix& matrix) const
	{
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}

	void Effect::SetDiffuseMap(const Texture* pDiffuseTexture) const
	{
		if (m_pDiffuseMapVariable)
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
	}
}
