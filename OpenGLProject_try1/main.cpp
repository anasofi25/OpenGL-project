//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include <unordered_map>
#include <functional>

int start = 0.0f;
int duration = 20.0f;
int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


gps::Camera myCamera(
	glm::vec3(0.0f, 10.0f, 5.0f),
	glm::vec3(0.0f, 5.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
const float cameraSpeed = 0.1f;
const float sensivity = 0.1f;

std::vector<glm::vec3> cameraWaypoints = {
	glm::vec3(0.0f, 40.0f, 5.5f),   // Location 1
	glm::vec3(-44.0f, 6.0f, 28.0f),   // Location 2
	glm::vec3(-39.0f, 3.0f, -0.5f),  // Location 3
	glm::vec3(-2.9f, 6.0f, 14.5f),  // Location 4
	glm::vec3(11.0f, 3.0f, 50.0f), // Location 5
	glm::vec3(20.0f, 6.0f, 80.0f), // Location 6
	glm::vec3(20.0f, 6.0f, 130.0f)  // Location 7
};
bool isMousePressed = false;
bool isAnimationRunning = false; // Flag to control animation
int currentWaypointIndex = 0;    // Current point in the path
float transitionDuration = 4.0f; // Duration to move between two points
float elapsedSegmentTime = 0.0f;    // Timer to control the animation
bool isMovingToWaypoint = true;      // Flag to check if the camera is moving
float cameraSpeed1 = 1.0f;
float waypointPauseDuration = 2.0f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D lightCube;
gps::Model3D screenQuad;

//my objects
gps::Model3D scene;
gps::Model3D windmillBase;
gps::Model3D windmillBlades;
gps::Model3D bird;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

GLuint textureID;

float pitch = -10.0f;
float yaw = -90.0f;
float initialMouseX = glWindowWidth / 2.0f;
float initialMouseY = glWindowHeight / 2.0f;

int visualize = 0;
float movementSpeed = 2.0f;
float rotationSpeed = 500.0f;

float lastTimeStamp = 0.0f;

//boolean variables
bool fog = false; //fog
bool isFirstMouse = true; //move the scene according to the mouse movement
bool animation = true;  //start the animation automatically

glm::vec3 lanternPosition = glm::vec3(-14.0f, -0.8f, 2.0f); // Example position of the lantern
glm::vec3 lanternColor = glm::vec3(1.0f, 0.8f, 0.6f); // Warm light color (yellow/orange)
float lanternIntensity = 1.0f; // Light intensity (brightness)
float lightRadius = 5.0f; // Radius of the light falloff

// Store default camera state
glm::vec3 defaultPosition(0.0f, 10.0f, 5.0f);
glm::vec3 defaultTarget(0.0f, 5.0f, 0.0f);
glm::vec3 defaultUp(0.0f, 1.0f, 0.0f);

bool hasAnimationStarted = false;
// Each bird starts at different X and Z coordinates.
glm::vec3 birdPositions[] = {
	glm::vec3(5.0f, 0.1f, -5.0f),  // Bird 1 near the lake at (5, 0.1, -5)
	glm::vec3(8.0f, 0.1f, -5.0f),  // Bird 2 near the lake at (0, 0.1, -5)
	glm::vec3(-5.0f, 0.1f, -4.0f)  // Bird 3 near the lake at (-5, 0.1, -5)
};
// Speed settings for floating motion
float movementSpeeds[] = { 0.5f, 1.0f, 0.5f };  // Different speeds for each bird to float differently
// Maximum distance the birds float forward and backward
float movementDistance = 3.0f;  // Max distance for the movement

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			isMousePressed = true;
			isFirstMouse = true; // Reset mouse tracking when pressing
		}
		else if (action == GLFW_RELEASE) {
			isMousePressed = false;
		}
	}
}
void mouseCallback(GLFWwindow* window, double currentX, double currentY) {
	static const float dampeningFactor = 0.1f; // Adjust for smoother or faster transitions

	// Skip processing if the mouse is not pressed
	if (!isMousePressed) {
		return;
	}

	if (isFirstMouse) {
		initialMouseX = static_cast<float>(currentX);
		initialMouseY = static_cast<float>(currentY);
		isFirstMouse = false;
	}

	// Calculate mouse offset
	float xOffset = static_cast<float>(currentX) - initialMouseX;
	float yOffset = initialMouseY - static_cast<float>(currentY); // Inverted Y-axis
	initialMouseX = static_cast<float>(currentX);
	initialMouseY = static_cast<float>(currentY);

	// Apply sensitivity and acceleration
	xOffset *= sensivity;
	yOffset *= sensivity;

	// Mouse acceleration: Scale offsets exponentially
	xOffset *= (1.0f + glm::abs(xOffset) * 0.1f); // Small adjustments scale exponentially
	yOffset *= (1.0f + glm::abs(yOffset) * 0.1f);

	// Smooth dampening: Gradually apply offsets
	static float targetYaw = yaw;
	static float targetPitch = pitch;

	targetYaw += xOffset;
	targetPitch += yOffset;

	// Constrain pitch to avoid gimbal lock
	if (targetPitch > 89.0f) {
		targetPitch = 89.0f;
	}
	else if (targetPitch < -89.0f) {
		targetPitch = -89.0f;
	}

	// Interpolate towards the target values for smooth dampening
	yaw += (targetYaw - yaw) * dampeningFactor;
	pitch += (targetPitch - pitch) * dampeningFactor;

	// Update the camera's orientation
	myCamera.rotate(pitch, yaw);
}

// Function to reset the camera
void resetCameraView() {
	myCamera.cameraPosition = defaultPosition;
	myCamera.cameraTarget = defaultTarget;
	myCamera.cameraUpDirection = defaultUp;

	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void executeCameraTour(float deltaTime) {
	if (!isAnimationRunning) return; // Exit early if the animation isn't active

	if (!hasAnimationStarted) {
		resetCameraView(); // Reset camera to the default position
		hasAnimationStarted = true;    // Prevent multiple resets
	}

	// Stop the animation if we've reached the last waypoint
	if (currentWaypointIndex >= cameraWaypoints.size() - 1) {
		isAnimationRunning = false;
		return;
	}

	elapsedSegmentTime += deltaTime; // Accumulate time for the current segment

	if (isMovingToWaypoint) {
		// Calculate the normalized progress (0 to 1) for the current segment
		float progress = elapsedSegmentTime / transitionDuration;

		if (progress >= 1.0f) {
			progress = 1.0f; // Clamp progress to avoid overshooting
			myCamera.setPosition(cameraWaypoints[currentWaypointIndex + 1]); // Snap to the next waypoint
			isMovingToWaypoint = false; // Pause movement
			elapsedSegmentTime = 0.0f;  // Reset the segment timer
		}
		else {
			// Interpolate between the current waypoint and the next
			glm::vec3 start = cameraWaypoints[currentWaypointIndex];
			glm::vec3 destination = cameraWaypoints[currentWaypointIndex + 1];
			glm::vec3 interpolatedPosition = glm::mix(start, destination, progress);
			myCamera.setPosition(interpolatedPosition); // Update the camera's position
		}
	}
	else {
		// Pause at the current waypoint before transitioning to the next
		if (elapsedSegmentTime >= waypointPauseDuration) {
			elapsedSegmentTime = 0.0f; // Reset the timer for the pause
			isMovingToWaypoint = true; // Start moving to the next waypoint
			currentWaypointIndex++;
		}

		// Stop the animation if we reached the last waypoint
		if (currentWaypointIndex >= static_cast<int>(cameraWaypoints.size()) - 1) {
			isAnimationRunning = false;
		}
	}
}

void updateCameraMatrices() {
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}
using KeyAction = std::function<void()>;

std::unordered_map<int, KeyAction> keyPressActions; // Actions for key press
std::unordered_map<int, KeyAction> keyReleaseActions; // Actions for key release

void processMovement()
{
	// go forward
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for object
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	// go to the left
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for object
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	// go behind
	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for object
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	//go to the right
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for object
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	//update light angle
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 3.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 3.0f;
	}

	// visualize the scene in three different modes 
	if (pressedKeys[GLFW_KEY_V]) {
		visualize++;
		if (visualize == 0) {
			// solid
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else if (visualize == 1) {
			// wireframe 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else if (visualize == 2) {
			// polygonal
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

			//reset the visualization mode
			visualize = -1;
		}
		pressedKeys[GLFW_KEY_V] = false;
	}

	//enable or disable fog
	if (pressedKeys[GLFW_KEY_F]) {
		fog = not fog;
		myCustomShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "fog"), fog);
	}
	// move up
	if (pressedKeys[GLFW_KEY_U]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed); // You might need to define MOVE_UP in your Camera class
		// Update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// Compute normal matrix for object
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	// move down
	if (pressedKeys[GLFW_KEY_I]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed); // You might need to define MOVE_DOWN in your Camera class
		// Update view matrix
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// Compute normal matrix for object
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	if (pressedKeys[GLFW_KEY_R]) {
		resetCameraView();
	}

	if (pressedKeys[GLFW_KEY_T]) {
		isAnimationRunning = true; // Start the animation
		hasAnimationStarted = 0;   // Start from the first point
		elapsedSegmentTime = 0.0f;
	}

}
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);



	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
	if (key == GLFW_KEY_U && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
		pressedKeys[GLFW_KEY_U] = (action == GLFW_PRESS);
	}

	if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
		pressedKeys[GLFW_KEY_I] = (action == GLFW_PRESS);
	}
	if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
		pressedKeys[GLFW_KEY_R] = (action == GLFW_PRESS);
	}
	if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
		pressedKeys[GLFW_KEY_T] = (action == GLFW_PRESS);
	}

}
// Position settings for birds: 
void animateBirdMovement(gps::Shader& shader) {
	shader.useShaderProgram();

	glm::mat4 model = glm::mat4(1.0f);  // Start with identity matrix for the bird

	// Get the current time
	float time = glfwGetTime();

	// Loop through each bird to animate them
	for (int i = 0; i < 3; ++i) {
		// Bird floating back and forth (slightly forward and backward)
		float zPos = movementDistance * sin(time * movementSpeeds[i]);  // Sinusoidal movement for floating effect

		// Get the current bird's starting position
		glm::vec3 birdPosition = birdPositions[i];  // Base position of the bird

		// Update the bird's Z position (the floating motion)
		birdPosition.z += zPos;  // Apply the floating motion on the Z-axis

		// Apply the transformation
		model = glm::translate(model, birdPosition);

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		bird.Draw(shader);
	}
}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "A day at the Farm", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetMouseButtonCallback(glWindow, mouseButtonCallback);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);


	return true;
}



void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	scene.LoadModel("objects/farm/farm_try.mtl.obj");
	bird.LoadModel("objects/farm/bird.obj");
	windmillBlades.LoadModel("objects/windmill/windBlades.obj");
}


void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(60.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lanternPosition"), 1, glm::value_ptr(lanternPosition));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lanternColor"), 1, glm::value_ptr(lanternColor));
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "lanternIntensity"), lanternIntensity);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "lightRadius"), lightRadius);

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	myCamera.rotate(pitch, yaw);
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));


}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO

	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix\

	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 12.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);  // modified the parameters do the shadow is more proeminent
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}


void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scene.Draw(shader);
	windmillBlades.Draw(shader);
	windmillBase.Draw(shader);
	bird.Draw(shader);
}

void renderScene() {
	// Depth map creation pass
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true); // true for depthPass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render depth map on screen if toggled
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT);
		screenQuadShader.useShaderProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);
		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		// Final scene rendering pass
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false); // false for depthPass

		animateBirdMovement(myCustomShader);
	}
}


void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {
	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}



	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();

	initFBO();

	glCheckError();

	start = glfwGetTime();

	while (!glfwWindowShouldClose(glWindow)) {
		// Update deltaTime
	
			float deltaTime = glfwGetTime() - lastTimeStamp; // Time difference for smooth animation
			executeCameraTour(deltaTime); // Update camera movement
			lastTimeStamp = glfwGetTime(); // Update last time
		
		// Process inputs and render the scene
			processMovement();
		renderScene();
		
		// Swap buffers and poll events
		glfwSwapBuffers(glWindow);
		glfwPollEvents();
	}

	cleanup();

	return 0;
}