#pragma once
#include "pch.h"

namespace dae
{
	class Camera final
	{
	public:
		Camera() = default;
		Camera(const Vector3& _origin, float _fovAngle):
			m_Origin{_origin}
		{
			ChangeFov(_fovAngle);
		}

		void Initialize(const float _aspectRatio, const float _fovAngle = 90.f, const Vector3& _origin = { 0.f,0.f,0.f })
		{
			m_AspectRatio = _aspectRatio;

			ChangeFov(_fovAngle);

			m_Origin = _origin;
		}

		void Update(const Timer* pTimer, bool isUsingDirectX)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//lock mouse position
			SDL_SetRelativeMouseMode((pKeyboardState[SDL_SCANCODE_LCTRL] || pKeyboardState[SDL_SCANCODE_RCTRL]) ? SDL_TRUE : SDL_FALSE);

			//set variables
			const float currentSpeedMultiplier{ (pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT]) ? m_SpeedMultiplier : 1.f };

			const float currentKeyboardMoveSpeed{ m_KeyboardMoveSpeed * currentSpeedMultiplier * deltaTime };
			const float currentMouseMoveSpeed{ ((isUsingDirectX) ? m_DirectXMouseMoveSpeed : m_SoftwareMouseMoveSpeed) * currentSpeedMultiplier * deltaTime };
			const float currentRotateSpeed{ ((isUsingDirectX) ? m_DirectXRotateSpeed : m_SoftwareRotateSpeed) * currentSpeedMultiplier * deltaTime };

			const bool lmb{ mouseState == SDL_BUTTON_LMASK }; //left mouse button
			const bool rmb{ mouseState == SDL_BUTTON_RMASK }; //right mouse button
			const bool lrmb{ mouseState == SDL_BUTTON_X2 }; //left & right mouse button

			//fov changes
			if (pKeyboardState[SDL_SCANCODE_PERIOD])
			{
				ChangeFov(m_FovAngle - currentKeyboardMoveSpeed);
			}
			if (pKeyboardState[SDL_SCANCODE_COMMA])
			{
				ChangeFov(m_FovAngle + currentKeyboardMoveSpeed);
			}

			//keyboard controls
			//forward movement
			m_Origin += m_Forward * (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP]) * currentKeyboardMoveSpeed;
			m_Origin -= m_Forward * (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN]) * currentKeyboardMoveSpeed;

			//right movement
			m_Origin += m_Right * (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT]) * currentKeyboardMoveSpeed;
			m_Origin -= m_Right * (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT]) * currentKeyboardMoveSpeed;

			//up movement
			m_Origin += m_Up * pKeyboardState[SDL_SCANCODE_E] * currentKeyboardMoveSpeed;
			m_Origin -= m_Up * pKeyboardState[SDL_SCANCODE_Q] * currentKeyboardMoveSpeed;

			//mouse controls
			//mouse movement
			m_Origin -= m_Forward * (lmb * mouseY * currentMouseMoveSpeed);
			m_Origin += m_Right * (lrmb * mouseX * currentMouseMoveSpeed);
			m_Origin -= m_Up * (lrmb * mouseY * currentMouseMoveSpeed);

			//mouse rotation
			m_TotalYaw += lmb * mouseX * currentRotateSpeed;
			m_TotalYaw += rmb * mouseX * currentRotateSpeed;
			m_TotalPitch -= rmb * mouseY * currentRotateSpeed;
			m_TotalPitch = std::clamp(m_TotalPitch, -89.99f * TO_RADIANS, 89.99f * TO_RADIANS);

			//update forward vector
			const Matrix finalRotation{ Matrix::CreateRotation(m_TotalPitch, m_TotalYaw, 0.f) };

			m_Forward = finalRotation.TransformVector(Vector3::UnitZ);

			//update view matrix
			CalculateViewMatrix();
		}

		Matrix& GetInvViewMatrix()
		{
			return m_InvViewMatrix;
		}
		Matrix& GetViewMatrix()
		{
			return m_ViewMatrix;
		}
		Matrix& GetProjectionMatrix()
		{
			return m_ProjectionMatrix;
		}

	private:
		Vector3 m_Origin{};
		float m_FovAngle{ 90.f };
		float m_FovTan{ 1.f };
		const float m_MinFov{ 30.f };
		const float m_MaxFov{ 120.f };

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };

		float m_TotalPitch{ 0.f };
		float m_TotalYaw{ 0.f };

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		const float m_NearPlane{ .1f };
		const float m_FarPlane{ 100.f };

		float m_AspectRatio{};

		const float m_KeyboardMoveSpeed{ 10.f };
		const float m_DirectXMouseMoveSpeed{ 60.f };
		const float m_SoftwareMouseMoveSpeed{ 5.f };
		const float m_DirectXRotateSpeed{ 360.f * TO_RADIANS };
		const float m_SoftwareRotateSpeed{ 20.f * TO_RADIANS };
		const int m_SpeedMultiplier{ 4 };

		void CalculateViewMatrix()
		{
			//calculate ONB => invViewMatrix
			m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
			m_Up = Vector3::Cross(m_Forward, m_Right);

			m_InvViewMatrix = Matrix{ m_Right, m_Up, m_Forward, m_Origin };

			//calculate Inverse(ONB) => ViewMatrix
			m_ViewMatrix = Matrix::Inverse(m_InvViewMatrix);
		}

		void CalculateProjectionMatrix()
		{
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
			m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_FovTan, m_AspectRatio, m_NearPlane, m_FarPlane);
		}

		void ChangeFov(float newAngle)
		{
			m_FovAngle = std::clamp(newAngle, m_MinFov, m_MaxFov);

			m_FovTan = tanf((m_FovAngle / 2) * TO_RADIANS);

			//update projection matrix
			CalculateProjectionMatrix();
		}
	};
}
