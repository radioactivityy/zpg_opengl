#pragma once
#include "glutils.h"
class Camera {
private:
	glm::vec3 position_;
	glm::vec3 target_;
	glm::vec3 up_;
	glm::mat4 view_matrix_;
	float farplane_{ 1000 };
	float nearplane_{ 0.1 };
	double width_;
	double height_;
	double fovy_;
	glm::mat4 projection_matrix;


	void UpdateViewMatrix();
	void UpdateProjectionMatrix();


public:
	//view matrix getters
	const glm::vec3 GetPosition();
	const glm::vec3 GetTarget();
	const glm::vec3 GetUp();
	const glm::mat4 GetViewMatrix() const;
	const glm::mat4 GetProjectionMatrix() const;

	//projectionMatrix getters
	const float GetFar();
	const float GetNear();
	const double GetHeight();
	const double GetWidth();
	
	const double GetFOV();


	void SetPosition(glm::vec3);
	void SetTarget(glm::vec3);
	void SetUp(glm::vec3);
	void SetFOV(double);
	void SetWidth(double width) { width_ = width; UpdateProjectionMatrix(); }  
	void SetHeight(double height) { height_ = height; UpdateProjectionMatrix(); }



};