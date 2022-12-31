#include "pch.h"
#include "EffectStandard.h"
#include "Texture.h"

namespace dae
{
	EffectStandard::EffectStandard(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect(pDevice, assetFile)
	{
		//Get Effect Variables
		LoadEffectVariables();
	}
	EffectStandard::~EffectStandard()
	{
		if (m_pGlossinessMapVariable) m_pGlossinessMapVariable->Release();
		if (m_pSpecularMapVariable) m_pSpecularMapVariable->Release();
		if (m_pNormalMapVariable) m_pNormalMapVariable->Release();
		if (m_pMatViewInverseVariable) m_pMatViewInverseVariable->Release();
		if (m_pMatWorldVariable) m_pMatWorldVariable->Release();
	}

	void EffectStandard::LoadEffectVariables()
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

		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldVariable not valid!";
		}

		m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pMatViewInverseVariable->IsValid())
		{
			std::wcout << L"m_pMatViewInverseVariable not valid!";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid!";
		}

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable not valid!";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable not valid!";
		}

		m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossinessMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable not valid!";
		}
	}
	HRESULT EffectStandard::LoadInputLayout(ID3D11Device* pDevice, ID3D11InputLayout** ppInputLayout)
	{
		//Create Vertex Layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 20;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 32;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

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

	void EffectStandard::SetWorldMatrix(const Matrix& matrix)
	{
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}
	void EffectStandard::SetViewInverseMatrix(const Matrix& matrix)
	{
		m_pMatViewInverseVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}

	void EffectStandard::SetNormalMap(Texture* pNormalTexture)
	{
		if (m_pNormalMapVariable)
			m_pNormalMapVariable->SetResource(pNormalTexture->GetSRV());
	}
	void EffectStandard::SetSpecularMap(Texture* pSpecularTexture)
	{
		if (m_pSpecularMapVariable)
			m_pSpecularMapVariable->SetResource(pSpecularTexture->GetSRV());
	}
	void EffectStandard::SetGlossinessMap(Texture* pGlossinessTexture)
	{
		if (m_pGlossinessMapVariable)
			m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetSRV());
	}
}
