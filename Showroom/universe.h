#pragma once
#define _USE_MATH_DEFINES // for C++
#include <cmath>

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>

#include <tiny_obj_loader.h>

#include <SFML\Window.hpp>
#include <SFML\OpenGL.hpp>
#include <SFML\Graphics\Image.hpp>

struct texture_t {
	GLuint id;
	sf::Image texture;
};

struct object_t{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<texture_t> textures;
};

/*struct sceneobj{
	object_t* object[3];
	sf::Vector3f position;
	sf::Vector3f rotation;
	float scale;
};*/

struct sceneobj{
	//tinyobj::shape_t shape;
	//tinyobj::material_t mat;
	object_t* object[3];
	sf::Vector3f position;
	sf::Vector3f rotation;
	float scale;
	float dist;

	bool operator < (const sceneobj& t) const
	{
		return (dist < t.dist);
	}
};

class universe
{
public:
	universe();
	~universe();

	bool addObject(char* dir, char* file);
	void run();

private:
	void resize();
	void light();
	void draw(tinyobj::shape_t& shape, tinyobj::material_t& mat);
	void display(sceneobj& o, bool drawTrans = false, int lod = 4);
	void init();
	void printError(const char* chapter = NULL);
	float distance(sf::Vector3f& a, sf::Vector3f& b);

	//std::vector<sf::Vector3f> abarths;

	std::vector<object_t> objects;

	std::vector<sceneobj> scene;

	sf::Window m_window;

	sf::Clock clock;
	float dt = 0.016f;

	sf::Vector3f m_camPos;
	sf::Vector2f m_camRot;

	sf::Vector3f m_light;

	GLfloat redDiffuseMaterial[3] = { 1.0f, 0.0f, 0.0f }; //set the material to red
	GLfloat whiteSpecularMaterial[3] = { 1.0f, 1.0f, 1.0f }; //set the material to white
	GLfloat greenEmissiveMaterial[3] = { 0.0f, 1.0f, 0.0f }; //set the material to green
	GLfloat whiteSpecularLight[3] = { 1.0f, 1.0f, 1.0f }; //set the light specular to white
	GLfloat blackAmbientLight[3] = { 0.0f, 0.0f, 0.0f }; //set the light ambient to black
	GLfloat whiteDiffuseLight[3] = { 1.0f, 1.0f, 1.0f }; //set the diffuse light to white
	GLfloat blankMaterial[3] = { 0.0f, 0.0f, 0.0f }; //set the diffuselight to white
	GLfloat mShininess[1] = { 64 }; //set the shininess of the material

	enum Quality { Q_LOW = 2, Q_MID = 1, Q_HIGH = 0, Q_ALL = -1 };
};

