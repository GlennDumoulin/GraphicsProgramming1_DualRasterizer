#pragma once

namespace dae
{
	//class Effect;
	//class Texture;

	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, EffectType effectType, const std::wstring& effectFilename, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void RenderDirectX(ID3D11DeviceContext* pDeviceContext) const;
		void RenderSoftware() const;

		void SetTranslation(const Vector3& translation);
		void SetRotation(const Vector3& rotation);
		void SetScale(const Vector3& scale);
		
		void RotateY(const float degAngle);

		void SetSampler(ID3D11SamplerState* pSampler);

		void UpdateMatrices(const Matrix& viewMatrix, const Matrix& projMatrix, const Matrix& viewInverseMatrix);
		
		/*void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);*/

		PrimitiveTopology GetPrimitiveTopology() const
		{
			return m_PrimitiveTopology;
		}

	private:
		Matrix m_TranslationMatrix{ Matrix::CreateTranslation(Vector3::Zero) };
		Matrix m_RotationMatrix{ Matrix::CreateRotation(Vector3::Zero) };
		Matrix m_ScaleMatrix{ Matrix::CreateScale(Vector3::One) };

		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TRIANGLE_LIST };

		void SetWorldViewProjMatrix(const Matrix& viewMatrix, const Matrix& projMatrix);
		void SetWorldMatrix();
		void SetViewInverseMatrix(const Matrix& viewInverseMatrix);

		Matrix GetWorldMatrix() const;

		//DirectX
		EffectType m_EffectType{};
		//Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_NumIndices{};

		HRESULT CreateLayouts(ID3D11Device* pDevice);
		HRESULT CreateBuffers(ID3D11Device* pDevice);

		//Software
		std::vector<Vertex> m_Vertices{};
		std::vector<VertexOut> m_VerticesOut{};
		std::vector<uint32_t> m_Indices{};
	};
}
