#pragma once

namespace dae
{
	class Texture;
	class Effect;

	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, const EffectType effectType, const std::wstring& effectFilename, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void RenderDirectX(ID3D11DeviceContext* pDeviceContext) const;
		void RenderSoftware(SoftwareRenderingInfo& SRInfo) const;

		void InitializeTransform(const Vector3& translation = Vector3::Zero, const Vector3& rotation = Vector3::Zero, const Vector3& scale = Vector3::One);
		void SetTranslation(const Vector3& translation);
		void SetRotation(const Vector3& rotation);
		void SetScale(const Vector3& scale);
		
		void RotateY(const float angle);

		void SetSampler(ID3D11SamplerState* pSampler) const;
		void SetRasterizerState(ID3D11RasterizerState* pRasterizerState, const CullMode cullMode);

		void UpdateMatrices(const Matrix& viewMatrix, const Matrix& projMatrix, const Matrix& viewInverseMatrix) const;
		void VertexTransformationFunction(const int width, const int height, const Matrix& viewMatrix, const Matrix& projMatrix, const Vector3& cameraPos);
		
		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);

		PrimitiveTopology GetPrimitiveTopology() const
		{
			return m_PrimitiveTopology;
		}

	private:
		Matrix m_TranslationMatrix{ Matrix::CreateTranslation(Vector3::Zero) };
		Matrix m_RotationMatrix{ Matrix::CreateRotation(Vector3::Zero) };
		Matrix m_ScaleMatrix{ Matrix::CreateScale(Vector3::One) };

		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TRIANGLE_LIST };

		void SetWorldViewProjMatrix(const Matrix& viewMatrix, const Matrix& projMatrix) const;
		void SetWorldMatrix() const;
		void SetViewInverseMatrix(const Matrix& viewInverseMatrix) const;

		Matrix GetWorldMatrix() const;

		//DirectX
		EffectType m_EffectType{};
		Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};

		uint32_t m_NumIndices{};

		HRESULT CreateLayouts(ID3D11Device* pDevice);
		HRESULT CreateBuffers(ID3D11Device* pDevice);

		//Software
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		
		std::vector<Vector2> m_VerticesScreenSpace{};
		std::vector<VertexOut> m_VerticesOut{};

		Texture* m_pDiffuseTexture{};
		Texture* m_pNormalTexture{};
		Texture* m_pSpecularTexture{};
		Texture* m_pGlossinessTexture{};

		CullMode m_CullMode{};

		void RenderTriangle(const size_t idx, SoftwareRenderingInfo& SRInfo, const bool shouldSwapVertices = false) const;
		static bool IsVerticeInFrustum(const VertexOut& vertice);
		bool IsCrossCheckValid(const float edge1Cross, const float edge2Cross, const float edge3Cross) const;
		void PixelShading(const VertexOut& vertice, ColorRGB& finalColor, const ShadingMode shadingMode, const bool isUsingNormalMap) const;
		ColorRGB CalculateSpecularColor(const Vector3& sampledNormal, const Vector3& lightDirection, const VertexOut& vertice, const float shininess) const;
	};
}
