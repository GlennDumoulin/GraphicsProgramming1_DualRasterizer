#include "pch.h"
#include "EffectTransparent.h"

namespace dae
{
	EffectTransparent::EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect(pDevice, assetFile)
	{
		//Get Effect Variables
		LoadEffectVariables();
	}
	EffectTransparent::~EffectTransparent()
	{
	}

	void EffectTransparent::LoadEffectVariables()
	{
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
			std::wcout << L"m_pTechnique not valid!\n";

		m_pSamplerVariable = m_pEffect->GetVariableByName("gSampler")->AsSampler();
		if (!m_pSamplerVariable->IsValid())
		{
			std::wcout << L"m_pSamplerVariable not valid!";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid!";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid!";
		}
	}
	HRESULT EffectTransparent::LoadInputLayout(ID3D11Device* pDevice, ID3D11InputLayout** ppInputLayout)
	{
		//Create Vertex Layout
		static constexpr uint32_t numElements{ 2 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create Input Layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		HRESULT result{
			pDevice->CreateInputLayout(
				vertexDesc,
				numElements,
				passDesc.pIAInputSignature,
				passDesc.IAInputSignatureSize,
				ppInputLayout
			)
		};

		return result;
	}
}
