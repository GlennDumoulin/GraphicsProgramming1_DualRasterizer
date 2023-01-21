#include "pch.h"
#include "Mesh.h"
#include "Texture.h"
#include "Effect.h"
#include "EffectStandard.h"
#include "EffectTransparent.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, const EffectType effectType, const std::wstring& effectFilename, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices)
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
	void Mesh::RenderSoftware(SoftwareRenderingInfo& SRInfo) const
	{
		//Handle Primitive Topology Type
		switch (m_PrimitiveTopology)
		{
		case PrimitiveTopology::TRIANGLE_LIST:
		{
			//Loop Over All Triangles
			for (size_t verticeIdx{}; verticeIdx < m_Indices.size(); verticeIdx += 3)
			{
				RenderTriangle(verticeIdx, SRInfo);
			}

			break;
		}

		case PrimitiveTopology::TRIANGLE_STRIP:
		{
			//Loop Over All Triangles
			for (size_t verticeIdx{}; verticeIdx < m_Indices.size() - 2; ++verticeIdx)
			{
				RenderTriangle(verticeIdx, SRInfo, verticeIdx % 2);
			}

			break;
		}
		}
	}

	void Mesh::InitializeTransform(const Vector3& translation, const Vector3& rotation, const Vector3& scale)
	{
		SetTranslation(translation);
		SetRotation(rotation);
		SetScale(scale);
	}

	void Mesh::RotateY(const float angle)
	{
		m_RotationMatrix = Matrix::CreateRotationY(angle) * m_RotationMatrix;
	}

	void Mesh::UpdateMatrices(const Matrix& viewMatrix, const Matrix& projMatrix, const Matrix& viewInverseMatrix) const
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
		default:
			break;
		}
	}
	Matrix Mesh::GetWorldMatrix() const
	{
		return m_ScaleMatrix * m_RotationMatrix * m_TranslationMatrix;
	}

#pragma region Software Rendering
	void Mesh::RenderTriangle(const size_t idx, SoftwareRenderingInfo& SRInfo, const bool shouldSwapVertices) const
	{
		//store the indexes of current triangle vertices
		const size_t V0Idx{ m_Indices[idx] };
		const size_t V1Idx{ m_Indices[idx + 1 + shouldSwapVertices] };
		const size_t V2Idx{ m_Indices[idx + 1 + !shouldSwapVertices] };

		//check if the triangle has 3 different vertices
		if (V0Idx == V1Idx || V1Idx == V2Idx || V2Idx == V0Idx)
			return;

		//store triangle vertices in NDC space
		const VertexOut V0NDC{ m_VerticesOut[V0Idx] };
		const VertexOut V1NDC{ m_VerticesOut[V1Idx] };
		const VertexOut V2NDC{ m_VerticesOut[V2Idx] };

		//check if any vertice of the current triangle is outside the frustum
		if (!IsVerticeInFrustum(V0NDC))
			return;
		if (!IsVerticeInFrustum(V1NDC))
			return;
		if (!IsVerticeInFrustum(V2NDC))
			return;

		//store triangle vertices in SCREEN space
		const Vector2 V0Screen{ m_VerticesScreenSpace[V0Idx] };
		const Vector2 V1Screen{ m_VerticesScreenSpace[V1Idx] };
		const Vector2 V2Screen{ m_VerticesScreenSpace[V2Idx] };

		//calculate triangle edges
		const Vector2 edgeV0V1{ V1Screen - V0Screen };
		const Vector2 edgeV1V2{ V2Screen - V1Screen };
		const Vector2 edgeV2V0{ V0Screen - V2Screen };

		//calculate inverse of triangle area ( 1 / triangle area)
		const float invTriangleArea{ 1.f / Vector2::Cross(edgeV0V1, edgeV1V2) };

		//calculate triangle bounding box
		Vector2 minBoundingBox{ Vector2::Min(V0Screen, Vector2::Min(V1Screen, V2Screen)) };
		Vector2 maxBoundingBox{ Vector2::Max(V0Screen, Vector2::Max(V1Screen, V2Screen)) };
		minBoundingBox.Clamp(static_cast<float>(SRInfo.screenSize.x), static_cast<float>(SRInfo.screenSize.y));
		maxBoundingBox.Clamp(static_cast<float>(SRInfo.screenSize.x), static_cast<float>(SRInfo.screenSize.y));

		//add small margin to bounding box
		constexpr int boxMargin{ 1 };

		const int minX{ std::clamp(static_cast<int>(minBoundingBox.x) - boxMargin, 0, SRInfo.screenSize.x) };
		const int minY{ std::clamp(static_cast<int>(minBoundingBox.y) - boxMargin, 0, SRInfo.screenSize.y) };
		const int maxX{ std::clamp(static_cast<int>(maxBoundingBox.x) + boxMargin, 0, SRInfo.screenSize.x) };
		const int maxY{ std::clamp(static_cast<int>(maxBoundingBox.y) + boxMargin, 0, SRInfo.screenSize.y) };

		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				const int pixelIdx{ (py * SRInfo.screenSize.x) + px };

				//handle showing bounding boxes
				if (SRInfo.SRState == SoftwareRenderingState::BOUNDING_BOXES)
				{
					constexpr ColorRGB boundingBoxColor{ 1.f, 1.f, 1.f };

					SRInfo.pBackBufferPixels[pixelIdx] = SDL_MapRGB(SRInfo.pBackBuffer->format,
						static_cast<uint8_t>(boundingBoxColor.r * 255),
						static_cast<uint8_t>(boundingBoxColor.g * 255),
						static_cast<uint8_t>(boundingBoxColor.b * 255));

					//ignore any other calculations when showing bounding boxes
					continue;
				}

				const Vector2 currentPixel{ static_cast<float>(px), static_cast<float>(py) };

				//check if pixel is in triangle
				const Vector2 V0ToPixel{ currentPixel - V0Screen };
				const Vector2 V1ToPixel{ currentPixel - V1Screen };
				const Vector2 V2ToPixel{ currentPixel - V2Screen };

				//check if pixel is to the right side of all edges
				const float edge1Cross{ Vector2::Cross(edgeV0V1, V0ToPixel) };
				const float edge2Cross{ Vector2::Cross(edgeV1V2, V1ToPixel) };
				const float edge3Cross{ Vector2::Cross(edgeV2V0, V2ToPixel) };

				if (!IsCrossCheckValid(edge1Cross, edge2Cross, edge3Cross))
					continue;

				//calculate barycentric weights
				const float weightV0{ edge2Cross * invTriangleArea };
				const float weightV1{ edge3Cross * invTriangleArea };
				const float weightV2{ edge1Cross * invTriangleArea };

				//calculate depth for current pixel
				const float invDepthV0{ 1.f / V0NDC.position.z };
				const float invDepthV1{ 1.f / V1NDC.position.z };
				const float invDepthV2{ 1.f / V2NDC.position.z };

				const float pixelDepth
				{
					1.f /
					(
						weightV0 * invDepthV0 +
						weightV1 * invDepthV1 +
						weightV2 * invDepthV2
					)
				};

				//handle depth test
				if (SRInfo.pDepthBufferPixels[pixelIdx] < pixelDepth) continue;

				//store depth weight in depth buffer
				SRInfo.pDepthBufferPixels[pixelIdx] = pixelDepth;

				//initialize final color
				ColorRGB finalColor{};

				switch (SRInfo.SRState)
				{
				case SoftwareRenderingState::DEPTH_BUFFER:
				{
					//remap pixel depth to [0,1] range
					const float remappedDepth = Remap(pixelDepth, .997f, 1.f);

					finalColor = { remappedDepth, remappedDepth, remappedDepth };

					break;
				}

				case SoftwareRenderingState::DEFAULT:
				{
					//create combined vertex out with triangle info
					VertexOut combinedTriangleInfo{};

					//calculate interpolated depth for current triangle
					const float invInterpolatedDepthV0{ 1.f / V0NDC.position.w };
					const float invInterpolatedDepthV1{ 1.f / V1NDC.position.w };
					const float invInterpolatedDepthV2{ 1.f / V2NDC.position.w };

					//cache weight times depth for current triangle
					const float weightTimesDepthV0{ weightV0 * invInterpolatedDepthV0 };
					const float weightTimesDepthV1{ weightV1 * invInterpolatedDepthV1 };
					const float weightTimesDepthV2{ weightV2 * invInterpolatedDepthV2 };

					const float interpolatedPixelDepth
					{
						1.f /
						(
							weightTimesDepthV0 +
							weightTimesDepthV1 +
							weightTimesDepthV2
						)
					};

					//calculate pixel UV
					const Vector2 pixelUV
					{
						(
							weightTimesDepthV0 * V0NDC.uv +
							weightTimesDepthV1 * V1NDC.uv +
							weightTimesDepthV2 * V2NDC.uv
						)
						* interpolatedPixelDepth
					};

					//calculate pixel normal
					Vector3 pixelNormal
					{
						(
							weightTimesDepthV0 * V0NDC.normal +
							weightTimesDepthV1 * V1NDC.normal +
							weightTimesDepthV2 * V2NDC.normal
						)
						* interpolatedPixelDepth
					};
					pixelNormal.Normalize();

					//calculate pixel tangent
					Vector3 pixelTangent
					{
						(
							weightTimesDepthV0 * V0NDC.tangent +
							weightTimesDepthV1 * V1NDC.tangent +
							weightTimesDepthV2 * V2NDC.tangent
						)
						* interpolatedPixelDepth
					};
					pixelTangent.Normalize();

					//calculate pixel view direction
					Vector3 pixelViewDirection
					{
						(
							weightTimesDepthV0 * V0NDC.viewDirection +
							weightTimesDepthV1 * V1NDC.viewDirection +
							weightTimesDepthV2 * V2NDC.viewDirection
						)
						* interpolatedPixelDepth
					};
					pixelViewDirection.Normalize();

					//set combined triangle info
					combinedTriangleInfo.uv = pixelUV;
					combinedTriangleInfo.normal = pixelNormal;
					combinedTriangleInfo.tangent = pixelTangent;
					combinedTriangleInfo.viewDirection = pixelViewDirection;

					PixelShading(combinedTriangleInfo, finalColor, SRInfo.shadingMode, SRInfo.isUsingNormalMap);

					break;
				}
				
				default:
					break;
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				SRInfo.pBackBufferPixels[pixelIdx] = SDL_MapRGB(SRInfo.pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}

	void Mesh::VertexTransformationFunction(const int width, const int height, const Matrix& viewMatrix, const Matrix& projMatrix, const Vector3& cameraPos)
	{
		//clear out vertices vectors
		m_VerticesOut.clear();
		m_VerticesScreenSpace.clear();

		//reserve space for vertices vectors
		m_VerticesOut.reserve(m_Vertices.size());
		m_VerticesScreenSpace.reserve(m_Vertices.size());

		//Cache world matrix
		const Matrix worldMatrix{ GetWorldMatrix() };
		const Matrix worldViewProjMatrix{ worldMatrix * viewMatrix * projMatrix };

		//transform each vertex using camera view matrix and perspective info
		for (const Vertex& vertex : m_Vertices)
		{
			//Calculate view direction
			Vector3 viewDirection{ worldMatrix.TransformPoint(vertex.position) -  cameraPos};
			viewDirection.Normalize();

			VertexOut temp{
				Vector4{ worldViewProjMatrix.TransformPoint({ vertex.position, 1.f }) },
				vertex.uv,
				worldMatrix.TransformVector(vertex.normal).Normalized(),
				worldMatrix.TransformVector(vertex.tangent).Normalized(),
				viewDirection
			};

			temp.position.x /= temp.position.w;
			temp.position.y /= temp.position.w;
			temp.position.z /= temp.position.w;

			m_VerticesOut.emplace_back(temp);
		}

		//calculate vertices in SCREEN space
		for (const VertexOut& vertice : m_VerticesOut)
		{
			Vector2 temp{};
			temp.x = ((vertice.position.x + 1) / 2) * static_cast<float>(width);
			temp.y = ((1 - vertice.position.y) / 2) * static_cast<float>(height);

			m_VerticesScreenSpace.emplace_back(temp);
		}
	}

	bool Mesh::IsVerticeInFrustum(const VertexOut& vertice)
	{
		const Vector4 verticePos{ vertice.position };

		//check x-pos
		if (verticePos.x < -1.f || verticePos.x > 1.f)
			return false;

		//check y-pos
		if (verticePos.y < -1.f || verticePos.y > 1.f)
			return false;

		//check z-pos
		if (verticePos.z < 0.f || verticePos.z > 1.f)
			return false;

		//if all values are valid, return true
		return true;
	}
	bool Mesh::IsCrossCheckValid(const float edge1Cross, const float edge2Cross, const float edge3Cross) const
	{
		switch (m_CullMode)
		{
		case CullMode::BACK:
		{
			//Check if all results are positive
			return (edge1Cross > 0.f && edge2Cross > 0.f && edge3Cross > 0.f);
		}

		case CullMode::FRONT:
		{
			//Check if all results are negative
			return (edge1Cross < 0.f && edge2Cross < 0.f && edge3Cross < 0.f);
		}

		case CullMode::NONE:
		{
			//Check if all results are positive or all results are negative
			return (edge1Cross > 0.f && edge2Cross > 0.f && edge3Cross > 0.f) || (edge1Cross < 0.f && edge2Cross < 0.f && edge3Cross < 0.f);
		}
		}

		return false;
	}

	void Mesh::PixelShading(const VertexOut& vertice, ColorRGB& finalColor, const ShadingMode shadingMode, const bool isUsingNormalMap) const
	{
		//shading info
		const Vector3 lightDirection{ .577f, -.577f, .577f };
		constexpr float lightIntensity{ 7.f };

		constexpr float kd{ 1.f };
		constexpr float shininess{ 25.f };
		constexpr ColorRGB ambientColor{ .025f, .025f, .025f };

		//calculate sampled normal
		Vector3 sampledNormal{ vertice.normal };

		//handle showing normal map
		if (isUsingNormalMap)
		{
			const Vector3 binormal{ Vector3::Cross(vertice.normal, vertice.tangent) };
			const Matrix tangentSpaceAxis{ vertice.tangent, binormal.Normalized(), vertice.normal, { 0.f, 0.f ,0.f } };

			const ColorRGB normalColor{ m_pNormalTexture->Sample(vertice.uv) };
			sampledNormal = { normalColor.r, normalColor.g, normalColor.b };

			sampledNormal = 2 * sampledNormal - Vector3{ 1.f, 1.f, 1.f };
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
		}
		sampledNormal.Normalize();

		//calculate observed area
		const float observedArea{ Saturate(Vector3::Dot(sampledNormal, -lightDirection)) };

		//handle the different shading modes
		switch (shadingMode)
		{
		case ShadingMode::OBSERVED_AREA:
			finalColor += { observedArea, observedArea, observedArea };
			break;

		case ShadingMode::DIFFUSE:
			finalColor += (m_pDiffuseTexture->Sample(vertice.uv) * kd / PI) * lightIntensity * observedArea;
			break;

		case ShadingMode::SPECULAR:
			finalColor += CalculateSpecularColor(sampledNormal, lightDirection, vertice, shininess) * observedArea;
			break;

		case ShadingMode::COMBINED:
			const ColorRGB diffuseColor{ (m_pDiffuseTexture->Sample(vertice.uv) * kd / PI) * lightIntensity };
			const ColorRGB specularColor{ CalculateSpecularColor(sampledNormal, lightDirection, vertice, shininess) };

			finalColor += (diffuseColor * observedArea) + specularColor;
			break;
		}

		finalColor += ambientColor;
	}

	ColorRGB Mesh::CalculateSpecularColor(const Vector3& sampledNormal, const Vector3& lightDirection, const VertexOut& vertice, const float shininess) const
	{
		const Vector3 reflectVector{ Vector3::Reflect(lightDirection, sampledNormal) };
		const float reflectAngle{ Saturate(Vector3::Dot(reflectVector, -vertice.viewDirection)) };

		const ColorRGB glossinessColor{ m_pGlossinessTexture->Sample(vertice.uv) };
		const float glossinessExponent{ glossinessColor.r * shininess };

		const float phongValue{ powf(reflectAngle, glossinessExponent) };

		return m_pSpecularTexture->Sample(vertice.uv) * phongValue;
	}
#pragma endregion

#pragma region Setter Functions

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

	void Mesh::SetSampler(ID3D11SamplerState* pSampler) const
	{
		m_pEffect->SetSampler(pSampler);
	}
	void Mesh::SetRasterizerState(ID3D11RasterizerState* pRasterizerState, const CullMode cullMode)
	{
		m_CullMode = cullMode;

		m_pEffect->SetRasterizerState(pRasterizerState);
	}

	void Mesh::SetWorldViewProjMatrix(const Matrix& viewMatrix, const Matrix& projMatrix) const
	{
		m_pEffect->SetWorldViewProjMatrix(GetWorldMatrix() * viewMatrix * projMatrix);
	}
	void Mesh::SetWorldMatrix() const
	{
		const EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetWorldMatrix(GetWorldMatrix());
	}
	void Mesh::SetViewInverseMatrix(const Matrix& viewInverseMatrix) const
	{
		const EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetViewInverseMatrix(viewInverseMatrix);
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

		const EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetNormalMap(pNormalTexture);
	}
	void Mesh::SetSpecularMap(Texture* pSpecularTexture)
	{
		//Make sure the previous texture is deleted
		delete m_pSpecularTexture;
		m_pSpecularTexture = pSpecularTexture;

		const EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetSpecularMap(pSpecularTexture);
	}
	void Mesh::SetGlossinessMap(Texture* pGlossinessTexture)
	{
		//Make sure the previous texture is deleted
		delete m_pGlossinessTexture;
		m_pGlossinessTexture = pGlossinessTexture;

		const EffectStandard* pTempEffect{ dynamic_cast<EffectStandard*>(m_pEffect) };

		if (pTempEffect != nullptr)
			pTempEffect->SetGlossinessMap(pGlossinessTexture);
	}
#pragma endregion
}
