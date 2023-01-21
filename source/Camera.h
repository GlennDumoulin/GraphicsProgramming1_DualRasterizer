#pragma once
#include "pch.h"

namespace dae
{
	class Camera final
	{
	public:
		Camera() = default;
		explicit Camera(const float aspectRatio, const float fovAngle = 90.f, const Vector3& origin = { 0.f,0.f,0.f }, const float nearPlane = .1f, const float farPlane = 100.f)
			: m_Origin{ origin }
			, m_NearPlane{ nearPlane }
			, m_FarPlane{ farPlane }
			, m_AspectRatio{ aspectRatio }
		{
			ChangeFov(fovAngle);
		}

		void Update(const Timer* pTimer, const bool isUsingDirectX)
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
			const float currentMouseMoveSpeed{ m_MouseMoveSpeed * currentSpeedMultiplier };
			const float currentRotateSpeed{ m_RotateSpeed * currentSpeedMultiplier };

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
			const float fltMouseX{ static_cast<float>(mouseX) };
			const float fltMouseY{ static_cast<float>(mouseY) };

			//mouse movement - local
			m_Origin -= m_Forward * (static_cast<float>(lmb) * fltMouseY * currentMouseMoveSpeed);

			//mouse movement - world
			m_Origin += Vector3::UnitX * (static_cast<float>(lrmb) * fltMouseX * currentMouseMoveSpeed);
			m_Origin -= Vector3::UnitY * (static_cast<float>(lrmb) * fltMouseY * currentMouseMoveSpeed);

			//mouse rotation
			m_TotalYaw += static_cast<float>(lmb) * fltMouseX * currentRotateSpeed;
			m_TotalYaw += static_cast<float>(rmb) * fltMouseX * currentRotateSpeed;
			m_TotalPitch -= static_cast<float>(rmb) * fltMouseY * currentRotateSpeed;
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

		Vector3& GetPosition()
		{
			return m_Origin;
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

		const float m_KeyboardMoveSpeed{ 20.f };
		const float m_MouseMoveSpeed{ .05f };
		const float m_RotateSpeed{ .2f * TO_RADIANS };
		const float m_SpeedMultiplier{ 4.f };

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

		void ChangeFov(const float newAngle)
		{
			m_FovAngle = std::clamp(newAngle, m_MinFov, m_MaxFov);

			m_FovTan = tanf((m_FovAngle / 2) * TO_RADIANS);

			//update projection matrix
			CalculateProjectionMatrix();
		}
	};
}
