/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> skySphere;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, q, e, up, down, shift, timeAdd, timeSub;
	int speedConst;
	camera()
	{
		w = a = s = d = q = e = up = down = shift = timeAdd = timeSub = 0;
		speedConst = 5;
		pos = rot = glm::vec3(0, 0, 0);
		pos = glm::vec3(0, -3, 0);
	}
	glm::mat4 process(double ftime)
	{
		if (shift == 1) {
			speedConst = 50;
		}
		else {
			speedConst = 5;
		}

		float speed = 0, elevation = 0, angle = 0, vJump = 0;
		if (w == 1)
		{
			speed = speedConst * ftime;
		}
		else if (s == 1)
		{
			speed = -speedConst * ftime;
		}
		float yangle = 0;
		if (a == 1)
			yangle = -3 * ftime;
		else if (d == 1)
			yangle = 3 * ftime;

		if (q == 1) {
			elevation = speedConst * ftime;
		}
		else if (e == 1) {
			elevation = -speedConst * ftime;
		}

		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, elevation, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);

		



		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		glm::mat4 LD = glm::rotate(glm::mat4(1), angle, glm::vec3(cos(rot.x), 0, sin(rot.z)));
		return LD * R * T;
	}
};
camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> heightshader, progSky, progWater;

	// Contains vertex information for OpenGL
	GLuint TerrainVertexArrayID;
	GLuint WaterVertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint TerrainPosID, TerrainTexID, IndexBufferIDBox;
	GLuint WaterPosID, WaterTexID, WaterIndexBufferIDBox;

	//texture data
	GLuint GrassTexture, SnowTexture, SandTexture, CliffTexture;
	GLuint SkyTexture, NightTexture;
	GLuint GrassNormal, SnowNormal, SandNormal, CliffNormal;
	float time = 1.0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			mycam.q = 1;
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		{
			mycam.q = 0;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		{
			mycam.shift = 1;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		{
			mycam.shift = 0;
		}

		if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
		{
			mycam.timeAdd = 1;
		}
		if (key == GLFW_KEY_EQUAL && action == GLFW_RELEASE)
		{
			mycam.timeAdd = 0;
		}

		if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
		{
			mycam.timeSub = 1;
		}
		if (key == GLFW_KEY_MINUS && action == GLFW_RELEASE)
		{
			mycam.timeSub = 0;
		}

		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			mycam.up = 1;
		}
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
		{
			mycam.up = 0;
		}

		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			mycam.down = 1;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		{
			mycam.down = 0;
		}

	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{

	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
#define DRAW_LINES false
#define DRAW_GREY false
#define MESHSIZE 300
#define RESOLUTION 2.0f // Higher value = less verticies per unit of measurement
	void init_mesh()
	{
		//generate the VAO
		glGenVertexArrays(1, &TerrainVertexArrayID);
		glBindVertexArray(TerrainVertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &TerrainPosID);
		glBindBuffer(GL_ARRAY_BUFFER, TerrainPosID);

		// Size of the net mesh squared (grid) times 4 (verticies per rectangle)
		vec3 *vertices = new vec3[MESHSIZE * MESHSIZE * 4];

		for (int x = 0; x < MESHSIZE; x++)
			for (int z = 0; z < MESHSIZE; z++)
			{
				vertices[x * 4 + z * MESHSIZE * 4 + 0] = vec3(0.0, 0.0, 0.0) * RESOLUTION + vec3(x, 0, z) * RESOLUTION;
				vertices[x * 4 + z * MESHSIZE * 4 + 1] = vec3(1.0, 0.0, 0.0) * RESOLUTION + vec3(x, 0, z) * RESOLUTION;
				vertices[x * 4 + z * MESHSIZE * 4 + 2] = vec3(1.0, 0.0, 1.0) * RESOLUTION + vec3(x, 0, z) * RESOLUTION;
				vertices[x * 4 + z * MESHSIZE * 4 + 3] = vec3(0.0, 0.0, 1.0) * RESOLUTION + vec3(x, 0, z) * RESOLUTION;
			}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 4, vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//tex coords
		float t = RESOLUTION / 100;
		//float t = RESOLUTION / 100;

		vec2 *tex = new vec2[MESHSIZE * MESHSIZE * 4];
		for (int x = 0; x < MESHSIZE; x++)
			for (int y = 0; y < MESHSIZE; y++)
			{
				tex[x * 4 + y * MESHSIZE * 4 + 0] = vec2(0.0, 0.0) + vec2(x, y) * t;
				tex[x * 4 + y * MESHSIZE * 4 + 1] = vec2(t, 0.0) + vec2(x, y) * t;
				tex[x * 4 + y * MESHSIZE * 4 + 2] = vec2(t, t) + vec2(x, y) * t;
				tex[x * 4 + y * MESHSIZE * 4 + 3] = vec2(0.0, t) + vec2(x, y) * t;
			}
		glGenBuffers(1, &TerrainTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, TerrainTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 4, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//free(tex);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);

		GLuint *elements = new GLuint[MESHSIZE * MESHSIZE * 6];
		int ind = 0;
		for (int i = 0; i < MESHSIZE * MESHSIZE * 6; i += 6, ind += 4)
		{
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 2;
			elements[i + 3] = ind + 0;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 3;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * MESHSIZE * MESHSIZE * 6, elements, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}


#define WATERSIZE 1
#define W_RESOLUTION 600.0f
	void init_water()
	{
		//generate the VAO
		glGenVertexArrays(1, &WaterVertexArrayID);
		glBindVertexArray(WaterVertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &WaterPosID);
		glBindBuffer(GL_ARRAY_BUFFER, WaterPosID);

		// Size of the net mesh squared (grid) times 4 (verticies per rectangle)
		vec3 *vertices = new vec3[WATERSIZE * WATERSIZE * 4];

		for (int x = 0; x < WATERSIZE; x++)
			for (int z = 0; z < WATERSIZE; z++)
			{
				vertices[x * 4 + z * WATERSIZE * 4 + 0] = vec3(0.0, 0.0, 0.0) * W_RESOLUTION + vec3(x, 0, z) * W_RESOLUTION;
				vertices[x * 4 + z * WATERSIZE * 4 + 1] = vec3(1.0, 0.0, 0.0) * W_RESOLUTION + vec3(x, 0, z) * W_RESOLUTION;
				vertices[x * 4 + z * WATERSIZE * 4 + 2] = vec3(1.0, 0.0, 1.0) * W_RESOLUTION + vec3(x, 0, z) * W_RESOLUTION;
				vertices[x * 4 + z * WATERSIZE * 4 + 3] = vec3(0.0, 0.0, 1.0) * W_RESOLUTION + vec3(x, 0, z) * W_RESOLUTION;
			}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * WATERSIZE * WATERSIZE * 4, vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//tex coords
		float t = W_RESOLUTION / 100;
		//float t = W_RESOLUTION / 100;

		vec2 *tex = new vec2[WATERSIZE * WATERSIZE * 4];
		for (int x = 0; x < WATERSIZE; x++)
			for (int y = 0; y < WATERSIZE; y++)
			{
				tex[x * 4 + y * WATERSIZE * 4 + 0] = vec2(0.0, 0.0) + vec2(x, y) * t;
				tex[x * 4 + y * WATERSIZE * 4 + 1] = vec2(t, 0.0) + vec2(x, y) * t;
				tex[x * 4 + y * WATERSIZE * 4 + 2] = vec2(t, t) + vec2(x, y) * t;
				tex[x * 4 + y * WATERSIZE * 4 + 3] = vec2(0.0, t) + vec2(x, y) * t;
			}
		glGenBuffers(1, &WaterTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, WaterTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * WATERSIZE * WATERSIZE * 4, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//free(tex);

		glGenBuffers(1, &WaterIndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WaterIndexBufferIDBox);

		GLuint *elements = new GLuint[WATERSIZE * WATERSIZE * 6];
		int ind = 0;
		for (int i = 0; i < WATERSIZE * WATERSIZE * 6; i += 6, ind += 4)
		{
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 2;
			elements[i + 3] = ind + 0;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 3;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * WATERSIZE * WATERSIZE * 6, elements, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{

		//initialize the net mesh
		init_mesh();
		init_water();

		string resourceDirectory = "../resources";
		// Initialize mesh.
		skySphere = make_shared<Shape>();
		skySphere->loadMesh(resourceDirectory + "/sphere.obj");
		skySphere->resize();
		skySphere->init();

		int width, height, channels;
		char filepath[1000];

		string str;
		unsigned char* data;

		// Grass texture
		str = resourceDirectory + "/grass.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &GrassTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Grass normal map
		str = resourceDirectory + "/grass_normal.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &GrassNormal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);



		// Snow texture
		str = resourceDirectory + "/snow.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SnowTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SnowTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Snow normal map
		str = resourceDirectory + "/snow_normal.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SnowNormal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SnowNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);



		// Sand texture
		str = resourceDirectory + "/sand.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SandTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Sand normal map
		str = resourceDirectory + "/sand_normal.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SandNormal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Cliff texture
		str = resourceDirectory + "/cliff.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &CliffTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CliffTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Cliff normal map
		str = resourceDirectory + "/cliff_normal.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &CliffNormal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CliffNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Skybox Texture
		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SkyTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SkyTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Skybox Texture
		str = resourceDirectory + "/sky2.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &NightTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NightTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);



		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint TextureLocation, GrassTextureLocation, SnowTextureLocation, SandTextureLocation, CliffTextureLocation, SkyTextureLocation, NightTextureLocation;
		GLuint GrassNormalLocation, SnowNormalLocation, SandNormalLocation, CliffNormalLocation;

		GrassTextureLocation = glGetUniformLocation(heightshader->pid, "grassSampler");
		GrassNormalLocation = glGetUniformLocation(heightshader->pid, "grassNormal");
		SnowTextureLocation = glGetUniformLocation(heightshader->pid, "snowSampler");
		SnowNormalLocation = glGetUniformLocation(heightshader->pid, "snowNormal");
		SandTextureLocation = glGetUniformLocation(heightshader->pid, "sandSampler");
		SandNormalLocation = glGetUniformLocation(heightshader->pid, "sandNormal");
		CliffTextureLocation = glGetUniformLocation(heightshader->pid, "cliffSampler");
		CliffNormalLocation = glGetUniformLocation(heightshader->pid, "cliffNormal");
		// Then bind the uniform samplers to texture units:
		glUseProgram(heightshader->pid);
		glUniform1i(GrassTextureLocation, 0);
		glUniform1i(SnowTextureLocation, 1);
		glUniform1i(SandTextureLocation, 2);
		glUniform1i(CliffTextureLocation, 3);
		glUniform1i(CliffNormalLocation, 4);
		glUniform1i(SnowNormalLocation, 5);
		glUniform1i(GrassNormalLocation, 6);
		glUniform1i(SandNormalLocation, 7);

		SkyTextureLocation = glGetUniformLocation(progSky->pid, "dayTexSampler");
		NightTextureLocation = glGetUniformLocation(progSky->pid, "nightTexSampler");
		glUseProgram(progSky->pid);
		glUniform1i(SkyTextureLocation, 0);
		glUniform1i(NightTextureLocation, 1);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);


		// Initialize the GLSL program.
		heightshader = std::make_shared<Program>();
		heightshader->setVerbose(true);
		heightshader->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl", resourceDirectory + "/tesscontrol.glsl", resourceDirectory + "/tesseval.glsl");
		if (!heightshader->init())
		{
			std::cerr << "Heightmap shaders failed to compile... exiting!" << std::endl;
			int hold;
			cin >> hold;
			exit(1);
		}
		heightshader->addUniform("P");
		heightshader->addUniform("V");
		heightshader->addUniform("M");
		heightshader->addUniform("camoff");
		heightshader->addUniform("campos");
		heightshader->addUniform("time");
		heightshader->addUniform("resolution");
		heightshader->addUniform("meshsize");
		heightshader->addUniform("drawGrey");
		heightshader->addAttribute("vertPos");
		heightshader->addAttribute("vertTex");

		// Initialize the GLSL progSkyram.
		progSky = std::make_shared<Program>();
		progSky->setVerbose(true);
		progSky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!progSky->init())
		{
			std::cerr << "Skybox shaders failed to compile... exiting!" << std::endl;
			int hold;
			cin >> hold;
			exit(1);
		}
		progSky->addUniform("P");
		progSky->addUniform("V");
		progSky->addUniform("M");
		progSky->addUniform("campos");
		progSky->addUniform("time");
		progSky->addAttribute("vertPos");
		progSky->addAttribute("vertTex");

		// Initialize the GLSL program.
		progWater = std::make_shared<Program>();
		progWater->setVerbose(true);
		progWater->setShaderNames(resourceDirectory + "/water_vertex.glsl", resourceDirectory + "/water_fragment.glsl");
		if (!progWater->init())
		{
			std::cerr << "Water shaders failed to compile... exiting!" << std::endl;
			int hold;
			cin >> hold;
			exit(1);
		}
		progWater->addUniform("P");
		progWater->addUniform("V");
		progWater->addUniform("M");
		progWater->addUniform("camoff");
		progWater->addUniform("campos");
		progWater->addUniform("time");
		progWater->addAttribute("vertPos");
		progWater->addAttribute("vertTex");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		double frametime = get_last_elapsed_time();
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, MESHSIZE * RESOLUTION); //so much type casting... GLM metods are quite funny ones

		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateX, TransXYZ, TransY, S;

		RotateX = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		TransXYZ = glm::translate(glm::mat4(1.0f), camp);
		S = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));

		M = TransXYZ * RotateX * S;

		if (mycam.timeAdd) {
			time += 0.05;
		}
		else if (mycam.timeSub) {
			time -= 0.05;
		}
		else if (DRAW_LINES == false) {
			time += 0.004;
		}

		//cout << "\rx: " << mycam.pos.x << ", y: "<< mycam.pos.y << ", z: " << mycam.pos.z << "\tvisible: " << MESHSIZE * RESOLUTION;
		//cout << "visible: " << RESOLUTION * MESHSIZE << endl;

		// GL POLYGON _________________________________________________
		if (DRAW_LINES == true) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		// ____________________________________________________________

		// Draw the skybox ----------------------------------------------------------------
		progSky->bind();


		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//send the matrices to the shaders 
		glUniformMatrix4fv(progSky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(progSky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(progSky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(progSky->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1f(progSky->getUniform("time"), time);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SkyTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NightTexture);
		glDisable(GL_DEPTH_TEST);
		skySphere->draw(progSky, FALSE);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		progSky->unbind();
		float centerOffset = -MESHSIZE * RESOLUTION / 2.0f;
		vec3 offset = mycam.pos;
		offset.y = 0;
		offset.x = ((int)(offset.x / RESOLUTION)) * RESOLUTION;
		offset.z = ((int)(offset.z / RESOLUTION)) * RESOLUTION;
		// Draw the Water -----------------------------------------------------------------
		if (DRAW_GREY == false || DRAW_LINES == false) {

			progWater->bind();
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
			TransY = glm::translate(glm::mat4(1.0f), glm::vec3(centerOffset, 2.0f, centerOffset));
			M = TransY;
			glUniformMatrix4fv(progWater->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(progWater->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(progWater->getUniform("V"), 1, GL_FALSE, &V[0][0]);

			glUniform3fv(progWater->getUniform("camoff"), 1, &offset[0]);
			glUniform3fv(progWater->getUniform("campos"), 1, &mycam.pos[0]);
			glUniform1f(progWater->getUniform("time"), time);
			glBindVertexArray(WaterVertexArrayID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WaterIndexBufferIDBox);
			//glDisable(GL_DEPTH_TEST);
			// Must use gl_patches w/ tessalation
			//glPatchParameteri(GL_PATCH_VERTICES, 3.0f);
			glDrawElements(GL_TRIANGLES, MESHSIZE*MESHSIZE * 6, GL_UNSIGNED_INT, (void*)0);
			//glEnable(GL_DEPTH_TEST);
			progWater->unbind();
		}

		// Draw the terrain -----------------------------------------------------------------
		heightshader->bind();

		

		

		TransY = glm::translate(glm::mat4(1.0f), glm::vec3(centerOffset, 0.0f, centerOffset));
		M = TransY;
		glUniformMatrix4fv(heightshader->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(heightshader->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(heightshader->getUniform("V"), 1, GL_FALSE, &V[0][0]);


		
		glUniform3fv(heightshader->getUniform("camoff"), 1, &offset[0]);
		glUniform3fv(heightshader->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1f(heightshader->getUniform("time"), time);
		glUniform1i(heightshader->getUniform("meshsize"), MESHSIZE);
		glUniform1f(heightshader->getUniform("resolution"), RESOLUTION);
		glUniform1i(heightshader->getUniform("drawGrey"), DRAW_GREY);
		glBindVertexArray(TerrainVertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SnowTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, SandTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, CliffTexture);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, CliffNormal);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, SnowNormal);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, GrassNormal);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, SandNormal);

		glPatchParameteri(GL_PATCH_VERTICES, 3.0f);
		glDrawElements(GL_PATCHES, MESHSIZE*MESHSIZE * 6, GL_UNSIGNED_INT, (void*)0);


		heightshader->unbind();


		



		glBindVertexArray(0);

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
		// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
