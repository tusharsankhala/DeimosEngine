#pragma once

#include <vectormath/vectormath.hpp>

class Camera
{
public:
	Camera();
	void SetMatrix( const math::Matrix4& cameraMatrix );
	void LookAt( const math::Vector4& eyePos, const math::Vector4& lookAt );
	void LookAt( float yaw, float pitch, float distance, const math::Vector4& at );
	void SetFov( float fov, uint32_t width, uint32_t height, float nearPlane, float farPlane );
	void SetFov( float fov, float aspectRatio, float nearPlane, float farPlane );
	void UpdateCameraPolar( float yaw, float pitch, float x, float y, float distance );
	void UpdateCameraWASD( float yaw, float pitch, const bool keyDown[256], double deltaTime );


	math::Matrix4 GetView()			const { return m_view; }
	math::Matrix4 GetPrevView()		const { return m_prevView; }
	math::Vector4 GetPosition()		const { return m_eyePos; }

	math::Vector4 GetDirection()	const { return math::Vector4(( math::transpose( m_view ) * math::Vector4( 0.0f, 0.0f, 1.0f, 0.0 )).getXYZ(), 0 ); }
	math::Vector4 GetUp()			const { return math::Vector4(( math::transpose( m_view ) * math::Vector4( 0.0f, 1.0f, 0.0f, 0.0 )).getXYZ(), 0 ); }
	math::Vector4 GetSide()			const { return math::Vector4(( math::transpose( m_view ) * math::Vector4( 1.0f, 1.0f, 1.0f, 0.0 )).getXYZ(), 0 ); }
	math::Matrix4 GetProjection()	const { return m_proj; }

	float GetFovH()					const { return m_fovH; }
	float GetFovV()					const { return m_fovV; }

	float GetAspectRatio()			const { return m_aspectRatio; }

	float GetNearPlane()			const { return m_near; }
	float GetFarPlane()				const { return m_far; }

	float GetYaw()					const { return m_yaw; }
	float GetPitch()				const { return m_pitch; }
	float GetDistance()				const { return m_distance; }

	void SetSpeed( float speed ) { m_speed = speed; }
	void SetProjectionJitter( float jitterX, float jitterY );
	void SetProjectionJitter( uint32_t width, uint32_t height, uint32_t& sampleIndex );
	void UpdatePreviousmatrices() { m_prevView = m_view; }

private:
	math::Matrix4		m_view;
	math::Matrix4		m_proj;
	math::Matrix4		m_prevView;
	math::Vector4		m_eyePos;
	float				m_distance;
	float				m_fovV, m_fovH;
	float				m_near, m_far;
	float				m_aspectRatio;

	float				m_speed = 1.0f;
	float				m_yaw	= 0.0f;
	float				m_pitch = 0.0f;
	float				m_roll	= 0.0f;
};
