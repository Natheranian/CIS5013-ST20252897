
#include "core.h"
#include "TextureLoader.h"
#include "shader_setup.h"
#include "ArcballCamera.h"
#include "GUClock.h"
#include "PrincipleAxes.h"
#include "AIMesh.h"
#include "Cube.h"
#include "Tetrahedron.h"
#include "Cylinder.h"


using namespace std;
using namespace glm;

struct DirectionalLight {

	vec3 direction;
	vec3 colour;
	
	DirectionalLight() {

		direction = vec3(0.0f, 1.0f, 0.0f); // default to point upwards
		colour = vec3(1.0f, 1.0f, 1.0f);
	}

	DirectionalLight(vec3 direction, vec3 colour = vec3(1.0f, 1.0f, 1.0f)) {

		this->direction = direction;
		this->colour = colour;
	}
};

struct PointLight {

	vec3 pos;
	vec3 colour;
	vec3 attenuation; // x=constant, y=linear, z=quadratic

	PointLight() {

		pos = vec3(0.0f, 0.0f, 0.0f);
		colour = vec3(1.0f, 1.0f, 1.0f);
		attenuation = vec3(1.0f, 1.0f, 1.0f);
	}

	PointLight(vec3 pos, vec3 colour = vec3(1.0f, 1.0f, 1.0f), vec3 attenuation = vec3(1.0f, 1.0f, 1.0f)) {

		this->pos = pos;
		this->colour = colour;
		this->attenuation = attenuation;
	}
};

#pragma region Global variables

// Main clock for tracking time (for animation / interaction)
GUClock* gameClock = nullptr;

// Main camera
ArcballCamera* mainCamera = nullptr;

// Mouse tracking
bool				mouseDown = false;
double				prevMouseX, prevMouseY;

// Keyboard tracking
bool				forwardPressed;
bool				backPressed;
bool				rotateLeftPressed;
bool				rotateRightPressed;

// Random number engine (if needed)
mt19937 rndEngine;
uniform_real_distribution<float> rndDist;

// Scene objects
AIMesh* groundMesh = nullptr;
AIMesh* creatureMesh = nullptr;
Cylinder* cylinderMesh = nullptr;


// Shaders

// Texture shader for ground
GLuint				textureShader;
GLuint				textureShader_transformMat;

// Shader for main beast character
GLuint				beastShader;
GLint				beastShader_mvpMatrix;

GLuint				beastHueShader;
GLint				beastHueShader_modelMatrix;
GLint				beastHueShader_viewMatrix;
GLint				beastHueShader_projectionMatrix;
GLint				beastHueShader_cylinderPos;


// Texture-directional light shader
GLuint				texDirLightShader;
GLint				texDirLightShader_modelMatrix;
GLint				texDirLightShader_viewMatrix;
GLint				texDirLightShader_projMatrix;
GLint				texDirLightShader_texture;
GLint				texDirLightShader_lightDirection;
GLint				texDirLightShader_lightColour;

// Texture-point light shader
GLuint				texPointLightShader;
GLint				texPointLightShader_modelMatrix;
GLint				texPointLightShader_viewMatrix;
GLint				texPointLightShader_projMatrix;
GLint				texPointLightShader_texture;
GLint				texPointLightShader_lightPos;
GLint				texPointLightShader_lightColour;
GLint				texPointLightShader_lightAttenuation;



// Window size
unsigned int windowWidth = 1024;
unsigned int windowHeight = 768;


// cylinder model
vec3 cylinderPos = vec3(-2.0f, 2.0f, 0.0f);

// beast model
vec3 beastPos = vec3(2.0f, 0.0f, 0.0f);
float beastRotation = 0.0f;


// Directional light example
float directLightTheta = 0.0f;
DirectionalLight directLight = DirectionalLight(vec3(cosf(directLightTheta), sinf(directLightTheta), 0.0f));

// Setup point light example light (use array to make adding other lights easier)
PointLight lights[1] = { PointLight(vec3(0.0f, 1.0f, 0.0), vec3(1.0f, 0.0f, 0.0f)) };


#pragma endregion


// Function prototypes
void renderScene();
void updateScene();
void resizeWindow(GLFWwindow* window, int width, int height);
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos);
void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
void mouseEnterHandler(GLFWwindow* window, int entered);


int main() {

	//
	// 1. Initialisation
	//
	
	gameClock = new GUClock();

#pragma region OpenGL and window setup

	// Initialise glfw and setup window
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "CIS5013", NULL, NULL);

	// Check window was created successfully
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	

	// Set callback functions to handle different events
	glfwSetFramebufferSizeCallback(window, resizeWindow); // resize window callback
	glfwSetKeyCallback(window, keyboardHandler); // Keyboard input callback
	glfwSetCursorPosCallback(window, mouseMoveHandler);
	glfwSetMouseButtonCallback(window, mouseButtonHandler);
	glfwSetScrollCallback(window, mouseScrollHandler);
	glfwSetCursorEnterCallback(window, mouseEnterHandler);

	// Initialise glew
	glewInit();

	
	// Setup window's initial size
	resizeWindow(window, windowWidth, windowHeight);

#pragma endregion


	// Initialise scene - geometry and shaders etc
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // setup background colour to be black
	glClearDepth(1.0f);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//
	// Setup Textures, VBOs and other scene objects
	//
	mainCamera = new ArcballCamera(-33.0f, 45.0f, 8.0f, 55.0f, (float)windowWidth/(float)windowHeight, 0.1f, 500.0f);
	
	groundMesh = new AIMesh(string("Assets\\ground-surface\\surface01.obj"));
	if (groundMesh) {
		groundMesh->addTexture("Assets\\ground-surface\\lunar-surface01.png", FIF_PNG);
	}

	creatureMesh = new AIMesh(string("Assets\\beast\\beast.obj"));
	if (creatureMesh) {
		creatureMesh->addTexture(string("Assets\\beast\\beast_texture.bmp"), FIF_BMP);
	}

	cylinderMesh = new Cylinder(string("Assets\\cylinder\\cylinderT.obj"));
	

	// Load shaders
	textureShader = setupShaders(string("Assets\\Shaders\\basic_texture.vert"), string("Assets\\Shaders\\basic_texture.frag"));
	beastShader = setupShaders(string("Assets\\beast\\beast_shader.vert"), string("Assets\\beast\\beast_shader.frag"));
	beastHueShader = setupShaders(string("Assets\\beast\\beast-hue-effect.vert"), string("Assets\\beast\\beast-hue-effect.frag"));
	texPointLightShader = setupShaders(string("Assets\\Shaders\\texture-point.vert"), string("Assets\\Shaders\\texture-point.frag"));
	texDirLightShader = setupShaders(string("Assets\\Shaders\\texture-directional.vert"), string("Assets\\Shaders\\texture-directional.frag"));

	// Get uniform variable locations for setting values later during rendering

	textureShader_transformMat = glGetUniformLocation(textureShader, "transformMat"); // sane varable but in different shader!

	beastShader_mvpMatrix = glGetUniformLocation(beastShader, "mvpMatrix");

	beastHueShader_modelMatrix = glGetUniformLocation(beastHueShader, "modelMatrix");
	beastHueShader_viewMatrix = glGetUniformLocation(beastHueShader, "viewMatrix");
	beastHueShader_projectionMatrix = glGetUniformLocation(beastHueShader, "projectionMatrix");
	beastHueShader_cylinderPos = glGetUniformLocation(beastHueShader, "cylinderPos");

	texDirLightShader_modelMatrix = glGetUniformLocation(texDirLightShader, "modelMatrix");
	texDirLightShader_viewMatrix = glGetUniformLocation(texDirLightShader, "viewMatrix");
	texDirLightShader_projMatrix = glGetUniformLocation(texDirLightShader, "projMatrix");
	texDirLightShader_texture = glGetUniformLocation(texDirLightShader, "texture");
	texDirLightShader_lightDirection = glGetUniformLocation(texDirLightShader, "lightDirection");
	texDirLightShader_lightColour = glGetUniformLocation(texDirLightShader, "lightColour");

	texPointLightShader_modelMatrix = glGetUniformLocation(texPointLightShader, "modelMatrix");
	texPointLightShader_viewMatrix = glGetUniformLocation(texPointLightShader, "viewMatrix");
	texPointLightShader_projMatrix = glGetUniformLocation(texPointLightShader, "projMatrix");
	texPointLightShader_texture = glGetUniformLocation(texPointLightShader, "texture");
	texPointLightShader_lightPos = glGetUniformLocation(texPointLightShader, "lightPos");
	texPointLightShader_lightColour = glGetUniformLocation(texPointLightShader, "lightColour");
	texPointLightShader_lightAttenuation = glGetUniformLocation(texPointLightShader, "lightAttenuation");




	// Setup random numbers for randomValue
	random_device rd;
	rndEngine = mt19937(rd());
	rndDist = uniform_real_distribution<float>(0.2f, 1.0f);


	//
	// 2. Main loop
	// 

	while (!glfwWindowShouldClose(window)) {

		updateScene();
		renderScene();						// Render into the current buffer
		glfwSwapBuffers(window);			// Displays what was just rendered (using double buffering).

		glfwPollEvents();					// Use this version when animating as fast as possible
	
		// update window title
		char timingString[256];
		sprintf_s(timingString, 256, "CIS5013: Average fps: %.0f; Average spf: %f", gameClock->averageFPS(), gameClock->averageSPF() / 1000.0f);
		glfwSetWindowTitle(window, timingString);
	}

	glfwTerminate();

	if (gameClock) {

		gameClock->stop();
		gameClock->reportTimingData();
	}

	return 0;
}


// renderScene - function to render the current scene
void renderScene()
{
	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get camera matrices
	mat4 cameraProjection = mainCamera->projectionTransform();
	mat4 cameraView = mainCamera->viewTransform() * translate(identity<mat4>(), -beastPos);

	// Render ground with light source!
	if (groundMesh) {

		glUseProgram(texDirLightShader);

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(10.0f, 1.0f, 10.0f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);
		glUniformMatrix4fv(texDirLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
		glUniformMatrix4fv(texDirLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);

		glUniform1i(texDirLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes

		glUniform3fv(texDirLightShader_lightDirection, 1, (GLfloat*)&(directLight.direction));
		glUniform3fv(texDirLightShader_lightColour, 1, (GLfloat*)&(directLight.colour));

		groundMesh->preRender();
		groundMesh->render();
		groundMesh->postRender();
	}


	if (creatureMesh) {
	
		//glUseProgram(beastHueShader);

		mat4 modelTransform = glm::translate(identity<mat4>(), beastPos) * eulerAngleY<float>(glm::radians<float>(beastRotation));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		// Setup uniforms
		//glUniformMatrix4fv(beastHueShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);
		//glUniformMatrix4fv(beastHueShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
		//glUniformMatrix4fv(beastHueShader_projectionMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
		//glUniform3fv(beastHueShader_cylinderPos, 1, (GLfloat*)&cylinderPos);

		creatureMesh->preRender();
		creatureMesh->render();
		creatureMesh->postRender();
	}

	if (cylinderMesh) {

		mat4 T = cameraProjection * cameraView * glm::translate(identity<mat4>(), cylinderPos);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		cylinderMesh->preRender();
		cylinderMesh->render(T);
		cylinderMesh->postRender();

		glDisable(GL_BLEND);

		glUseProgram(0);
	}

	// Render light source
	glUseProgram(0);
	mat4 cameraT = cameraProjection * cameraView;
	glLoadMatrixf((GLfloat*)&cameraT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glVertex3f(directLight.direction.x * 10.0f, directLight.direction.y * 10.0f, directLight.direction.z * 10.0f);
	glEnd();
}


// Function called to animate elements in the scene
void updateScene() {

	float tDelta = 0.0f;

	if (gameClock) {

		gameClock->tick();
		tDelta = (float)gameClock->gameTimeDelta();
	}

	cylinderMesh->update(tDelta);

	// update main light source
	directLightTheta += glm::radians(45.0f) * tDelta;
	directLight.direction = vec3(cosf(directLightTheta), sinf(directLightTheta), 0.0f);

	//
	// Handle movement based on user input
	//

	float moveSpeed = 3.0f; // movement displacement per second
	float rotateSpeed = 90.0f; // degrees rotation per second

	if (forwardPressed) {

		mat4 R = eulerAngleY<float>(glm::radians<float>(beastRotation)); // local coord space / basis vectors - move along z
		float dPos = moveSpeed * tDelta; // calc movement based on time elapsed
		beastPos += vec3(R[2].x * dPos, R[2].y * dPos, R[2].z * dPos); // add displacement to position vector
	}
	else if (backPressed) {

		mat4 R = eulerAngleY<float>(glm::radians<float>(beastRotation)); // local coord space / basis vectors - move along z
		float dPos = -moveSpeed * tDelta; // calc movement based on time elapsed
		beastPos += vec3(R[2].x * dPos, R[2].y * dPos, R[2].z * dPos); // add displacement to position vector
	}

	if (rotateLeftPressed) {

		beastRotation += rotateSpeed * tDelta;
	}
	else if (rotateRightPressed) {

		beastRotation -= rotateSpeed * tDelta;
	}

}


#pragma region Event handler functions

// Function to call when window resized
void resizeWindow(GLFWwindow* window, int width, int height)
{
	if (mainCamera) {

		mainCamera->setAspect((float)width / (float)height);
	}

	// Update viewport to cover the entire window
	glViewport(0, 0, width, height);

	windowWidth = width;
	windowHeight = height;
}


// Function to call to handle keyboard input
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {

		// check which key was pressed...
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			
			case GLFW_KEY_W:
				forwardPressed = true;
				break;

			case GLFW_KEY_S:
				backPressed = true;
				break;

			case GLFW_KEY_A:
				rotateLeftPressed = true;
				break;

			case GLFW_KEY_D:
				rotateRightPressed = true;
				break;

			default:
			{
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		// handle key release events
		switch (key)
		{
			case GLFW_KEY_W:
				forwardPressed = false;
				break;

			case GLFW_KEY_S:
				backPressed = false;
				break;

			case GLFW_KEY_A:
				rotateLeftPressed = false;
				break;

			case GLFW_KEY_D:
				rotateRightPressed = false;
				break;

			default:
			{
			}
		}
	}
}


void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos) {

	if (mouseDown) {

		float dx = float(xpos - prevMouseX);
		float dy = float(ypos - prevMouseY);

		if (mainCamera)
			mainCamera->rotateCamera(-dy, -dx);

		prevMouseX = xpos;
		prevMouseY = ypos;
	}

}

void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT) {

		if (action == GLFW_PRESS) {

			mouseDown = true;
			glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
		}
		else if (action == GLFW_RELEASE) {

			mouseDown = false;
		}
	}
}

void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset) {

	if (mainCamera) {

		if (yoffset < 0.0)
			mainCamera->scaleRadius(1.1f);
		else if (yoffset > 0.0)
			mainCamera->scaleRadius(0.9f);
	}
}

void mouseEnterHandler(GLFWwindow* window, int entered) {
}

#pragma endregion