#include "pch.h"
#include "Mesh.h"
#include "Texture.h"
#include "Effect.h"
#include "EffectStandard.h"
#include "EffectTransparent.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, EffectType effectType, const std::wstring& effectFilename, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_EffectType{ effectType }
		, m_Vertices{ vertices }
		, m_Indices{ indices }
	{
		//Create the Effect based on effect type
		switch (m_EffectType)
		{
		case EffectType::STANDARD:
		{
			m_pEffect = new EffectStandard{ pDevice, effectFilename };
			break;
		}

		case EffectType::TRANSPARENCY:
		{
			m_pEffect = new EffectTransparent{ pDevice, effectFilename };
			break;
		}
		}

		//Get Technique from Effect
		if (m_pEffect != nullptr)
			m_pTechnique = m_pEffect->GetTechnique();

		//Create layouts
		if (FAILED(CreateLayouts(pDevice)))
			return;

		//Create buffers
		if (FAILED(CreateBuffers(pDevice)))
			return;
	}
	Mesh::~Mesh()
	{
		if (m_pIndexBuffer)
			m_pIndexBuffer->Release();

		if (m_pInputLayout)
			m_pInputLayout->Release();

		if (m_pVertexBuffer)
			m_pVertexBuffer->Release();

		delete m_pEffect;

		delete m_pDiffuseTexture;
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;
	}

	HRESULT Mesh::CreateLayouts(ID3D11Device* pDevice)
	{
		return m_pEffect->LoadInputLayout(pDevice, &m_pInputLayout);
	}
	HRESULT Mesh::CreateBuffers(ID3D11Device* pDevice)
	{
		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = m_Vertices.data();

		HRESULT result{ pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer) };
		if (FAILED(result))
			return result;

		//Create index buffer
		m_NumIndices = static_cast<uint32_t>(m_Indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = m_Indices.data();

		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
		
		return result;
	}

	void Mesh::RenderDirectX(ID3D11DeviceContext* pDeviceContext) const
	{
		//1. Set Primitive Topology
		switch (m_PrimitiveTopology)
		{
		case PrimitiveTopology::TRIANGLE_LIST:
			pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;

		case PrimitiveTopology::TRIANGLE_STRIP:
			pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			break;
		}

		//2. Set Input Layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		//3. Set Vertex Buffer
		constexpr UINT stride{ sizeof(Vertex) };
		constexpr UINT offset{ 0 };
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		//4. Set Index Buffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5. Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pTechnique->GetDesc(&techDesc);
		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}
	void Mesh::RenderSoftware() const
	{
		//Calculate SCREEN Space Vertices
		//VertexTransformationFunction();

		//Handle Primitive Topology Type
		switch (m_PrimitiveTopology)
		{
		case PrimitiveTopology::TRIANGLE_LIST:
		{
			//Loop Over All Triangles
			for (size_t verticeIdx{}; verticeIdx < m_Indices.size(); verticeIdx += 3)
			{
				//RenderTriangle(verticeIdx);
			}

			break;
		}

		case PrimitiveTopology::TRIANGLE_STRIP:
		{
			//Loop Over All Triangles
			for (size_t verticeIdx{}; verticeIdx < m_Indices.size() - 2; ++verticeIdx)
			{
				//RenderTriangle(verticeIdx, verticeIdx % 2);
			}

			break;
		}
		}
	}

	void Mesh::SetTranslation(const Vector3& translation)
	{
		m_TranslationMatrix = Matrix::CreateTranslation(translation);
	}
	void Mesh::SetRotation(const Vector3& rotation)
	{
		m_RotationMatrix = Matrix::CreateRotation(rotation);
	}
	void Mesh::SetScale(const Vector3& scale)
	{
		m_ScaleMatrix = Matrix::CreateScale(scale);
	}

	void Mesh::RotateY(const float degAngle)
	{
		m_RotationMatrix = Matrix::CreateRotationY(degAngle * TO_RADIANS) * m_RotationMatrix;
	}

	void Mesh::SetSampler(ID3D11SamplerState* pSampler)
	{
		m_pEffect->SetSampler(pSampler);
	}

	void Mesh::SetWorldViewProjMatrix(const Matrix& viewMatrix, const Matrix& projMatrix)
	{
		m_pEffect->SetWorldViewProjMatrix(GetWorldMatrix() * viewMatrix * projMatrix);
	}
	void Mesh::SetWorldMatrix()
	{
		EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetWorldMatrix(GetWorldMatrix());
	}
	void Mesh::SetViewInverseMatrix(const Matrix& viewInverseMatrix)
	{
		EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetViewInverseMatrix(viewInverseMatrix);
	}
	void Mesh::UpdateMatrices(const Matrix& viewMatrix, const Matrix& projMatrix, const Matrix& viewInverseMatrix)
	{
		//Always update worldViewProj matrix
		SetWorldViewProjMatrix(viewMatrix, projMatrix);

		//Update specific matrices based on effect type
		switch (m_EffectType)
		{
		case EffectType::STANDARD:
		{
			SetWorldMatrix();
			SetViewInverseMatrix(viewInverseMatrix);

			break;
		}
		}
	}
	Matrix Mesh::GetWorldMatrix() const
	{
		return m_ScaleMatrix * m_RotationMatrix * m_TranslationMatrix;
	}

	void Mesh::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		//Make sure the previous texture is deleted
		delete m_pDiffuseTexture;
		m_pDiffuseTexture = pDiffuseTexture;

		m_pEffect->SetDiffuseMap(pDiffuseTexture);
	}
	void Mesh::SetNormalMap(Texture* pNormalTexture)
	{
		//Make sure the previous texture is deleted
		delete m_pNormalTexture;
		m_pNormalTexture = pNormalTexture;

		EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetNormalMap(pNormalTexture);
	}
	void Mesh::SetSpecularMap(Texture* pSpecularTexture)
	{
		//Make sure the previous texture is deleted
		delete m_pSpecularTexture;
		m_pSpecularTexture = pSpecularTexture;

		EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetSpecularMap(pSpecularTexture);
	}
	void Mesh::SetGlossinessMap(Texture* pGlossinessTexture)
	{
		//Make sure the previous texture is deleted
		delete m_pGlossinessTexture;
		m_pGlossinessTexture = pGlossinessTexture;

		EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetGlossinessMap(pGlossinessTexture);
	}
}
