#include "universe.h"

universe::universe()
{
	sf::ContextSettings ctxSettings;
	ctxSettings.depthBits = 24;
	m_window.create(sf::VideoMode(800, 600), "Showroom OpenGL", sf::Style::Default, ctxSettings);
}

universe::~universe()
{
	m_window.close();
}

bool universe::addObject(char* dir, char* file)
{
	bool ret = false;
	std::string err;
	std::string comp = dir;
	comp.append(file);
	
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	ret = tinyobj::LoadObj(shapes, materials, err, comp.c_str(), dir);

	if (!err.empty()) { // `err` may contain warning message.
		std::cout << err << std::endl;
	}
	if (!ret) {
		exit(1);
	}

	object_t tmp;
	tmp.shapes = shapes;
	tmp.materials = materials;

	for (int i = 0; i < materials.size(); ++i)
	{
		//if (materials.at(i).diffuse_texname.empty())
		//	break;

		texture_t tex;
		glGenTextures(1, &tex.id);
		glBindTexture(GL_TEXTURE_2D, tex.id);

		if (materials.at(i).diffuse_texname.empty())
		{
			tmp.textures.push_back(tex);
			continue;
		}

		sf::Image img;
		img.loadFromFile(materials.at(i).diffuse_texname);
		
		tex.texture = img;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.texture.getSize().x, tex.texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.texture.getPixelsPtr());
		tmp.textures.push_back(tex);
	}

	objects.push_back(tmp);

	return ret;
}

void universe::resize()
{
	int height = m_window.getSize().y;
	int width = m_window.getSize().x;
	// prevent division by zero
	if (m_window.getSize().y == 0) { height = 1; }

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat ratio = static_cast<float>(m_window.getSize().x) / m_window.getSize().y;
	glFrustum(-ratio, ratio, -1.f, 1.f, 1.f, 300.f);
	glMatrixMode(GL_MODELVIEW);
}

void universe::light()
{
	GLfloat lightPos[] = { 0.0f, 8.0f, 0.0f , 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, whiteSpecularLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, blackAmbientLight);
}

void universe::draw(tinyobj::shape_t & shape, tinyobj::material_t & mat)
{
	//float* diffuse = mat.diffuse;
	//mShininess[0] = mat.shininess;

	/*float diffuse[4];
	memcpy(diffuse, mat.diffuse, 3 * sizeof(float));
	diffuse[3] = mat.dissolve;*/

	if (mat.shininess < 0.0f)
		mat.shininess = 0.0f;

	if (mat.shininess > 128.0f)
		mat.shininess = 128.0f;

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat.diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat.specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blackAmbientLight);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat.emission);
	
	glTexCoordPointer(2, GL_FLOAT, 0, shape.mesh.texcoords.data());
	glVertexPointer(3, GL_FLOAT, 0, shape.mesh.positions.data());
	glNormalPointer(GL_FLOAT, 0, shape.mesh.normals.data());

	glDrawElements(GL_TRIANGLES, shape.mesh.indices.size(), GL_UNSIGNED_INT, shape.mesh.indices.data());
}

void universe::display(sceneobj& o, bool drawTrans, int lod)
{
	glPushMatrix();

	//apply transformations
	glScalef(o.scale, o.scale, o.scale);
	glTranslatef(o.position.x, o.position.y, o.position.z);
	glRotatef(o.rotation.y, 0, 1, 0);
	//***

	printError(std::to_string(__LINE__).c_str());

	//LOD
	float dist;
	int meshSwitch;
	dist = distance(m_camPos, o.position);

	if (lod >= 3)
	{
		meshSwitch = 2;
		if (dist > 15.0f)
			meshSwitch = 1;
		if (dist > 30.0f)
			meshSwitch = 0;
	}
	else
		meshSwitch = lod;
	//***

	glPushMatrix();

	//draw elements
	for (int i = 0; i < o.object[meshSwitch]->shapes.size(); ++i)
	{
		//filter transparent shapes
		float dissolve = o.object[meshSwitch]->materials.at(o.object[meshSwitch]->shapes.at(i).mesh.material_ids.at(0)).dissolve;
		if ((drawTrans == false && dissolve < 1.0f) || (drawTrans == true && dissolve >= 1.0f))
		{
			continue;
		}
		//***

		//set texture
		glBindTexture(GL_TEXTURE_2D, o.object[meshSwitch]->textures.at(o.object[meshSwitch]->shapes.at(i).mesh.material_ids.at(0)).id);

		//call draw method
		draw(o.object[meshSwitch]->shapes.at(i), o.object[meshSwitch]->materials.at(o.object[meshSwitch]->shapes.at(i).mesh.material_ids.at(0)));
	}
	//***

	glPopMatrix();

	glPopMatrix();
}

void universe::init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	//glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	//glEnable(GL_COLOR_MATERIAL);
	//glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	resize();
}

float universe::distance(sf::Vector3f& a, sf::Vector3f& b)
{
	sf::Vector3f diff(a - b);

	float dist = sqrt(pow(diff.x, 2) + pow(diff.y, 2) + pow(diff.z, 2));
	return dist;
}

void universe::printError(const char* chapter)
{
	GLenum err = glGetError();

	if (err == GL_NO_ERROR)
		return;

	if (chapter == NULL)
		std::cout << err << std::endl;
	else
		std::cout << chapter << " -> " << err << std::endl;
}

void universe::run()
{
	sf::Vector2f mouse_old(0, 0);

	m_camPos = sf::Vector3f(0, 5.0f, 0);

	//Build the scene
	sceneobj showroom;
	showroom.object[0] = &objects.at(0);
	showroom.object[1] = &objects.at(0);
	showroom.object[2] = &objects.at(0);
	showroom.position = showroom.rotation = sf::Vector3f(0, 0, 0);
	showroom.scale = 1.0f;

	scene.push_back(showroom);

	sceneobj abarth;
	abarth.object[0] = &objects.at(1);
	abarth.object[1] = &objects.at(2);
	abarth.object[2] = &objects.at(3);
	abarth.scale = 1.0f;
	abarth.position = sf::Vector3f(2.3f, 1.3f, -44.0f);
	abarth.rotation = sf::Vector3f(0, 0, 0);
	for (int i = 0; i <= 5; ++i)
	{
		scene.push_back(abarth);

		abarth.position.x += 15.0f;
		scene.push_back(abarth);
		abarth.position.x -= 15.0f;

		abarth.position.z += 12.0f;
	}

	sceneobj bmw;
	bmw.object[0] = &objects.at(4);
	bmw.object[1] = &objects.at(5);
	bmw.object[2] = &objects.at(6);
	bmw.scale = 0.8f;
	bmw.position = sf::Vector3f(-10.0f, 0.0f, -40.0f);
	bmw.rotation = sf::Vector3f(0, 90.0f, 0);
	for (int i = 0; i <= 3; ++i)
	{
		scene.push_back(bmw);

		bmw.position.x += 20.0f;
		scene.push_back(bmw);
		bmw.position.x -= 20.0f;

		bmw.position.z += 25.0f;
	}
	//***

	init();
	light();
	
	m_light = m_camPos;

	printError();

	//loop
	float cnt = 0;
	int lod = 4;
	bool quit = false;
	bool showInfo = true;
	bool cameraLight = false;
	bool sorting = true;

	while (!quit)
	{
		clock.restart();

		sf::Event event;
		while (m_window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				// end the program
				quit = true;
			}
			else if (event.type == sf::Event::Resized)
			{
				// adjust the viewport when the window is resized
				resize();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					quit = true;
				else if (event.key.code == sf::Keyboard::Num1)
				{
					sorting = !sorting;
					if (sorting)
						std::cout << "Sorting is ON\n";
					else
						std::cout << "Sorting is OFF\n";
				}
				else if (event.key.code == sf::Keyboard::Num2)
				{
					showInfo = !showInfo;
					if (showInfo)
						std::cout << "Info is ON\n";
					else
						std::cout << "Info is OFF\n";
				}
				else if (event.key.code == sf::Keyboard::Num3)
				{
					std::cout << "LightPos: " << m_camPos.x << ":" << m_camPos.y << ":" << m_camPos.z << std::endl;

					m_light = m_camPos;

					printError("LightPos");
				}
				else if (event.key.code == sf::Keyboard::Num4)
				{
					++lod;

					if (lod > 4)
						lod = 0;

					switch (lod)
					{
						case 0:
							std::cout << "LOD: lowest quality\n";
						break;
						case 1:
							std::cout << "LOD: mid quality\n";
						break;
						case 2:
							std::cout << "LOD: highest quality\n";
						break;
						case 3:
							std::cout << "LOD: automatic\n";
						break;
					}
				}
			}
		}

		//Mouse Input
		float sens = 3.0f;
		sf::Vector2f mouse_diff = sf::Vector2f(sf::Mouse::getPosition());
		m_camRot.y = (mouse_diff.x / sf::VideoMode::getDesktopMode().width) * 360.0f * sens;
		m_camRot.x = (mouse_diff.y / sf::VideoMode::getDesktopMode().height) * 360.0f * sens;

		while (m_camRot.y > 360.0) m_camRot.y -= 360.0f;
		while (m_camRot.y < 0.0) m_camRot.y += 360.0f;

		while (m_camRot.x > 360.0) m_camRot.x -= 360.0f;
		while (m_camRot.x < 0.0) m_camRot.x += 360.0f;

		if (m_camRot.x < 270.0f && m_camRot.x > 180.0f)
			m_camRot.x = 271.0f;
		else if (m_camRot.x > 90.0f && m_camRot.x < 180.0f)
			m_camRot.x = 89.0f;
		//***

		//Keyboard Input
		float pi = static_cast<float>(M_PI);
		float moveSpeed = 5.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			moveSpeed *= 2.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			m_camPos.z -= moveSpeed * dt * -sinf((m_camRot.y*pi) / 180.0f);
			m_camPos.x += moveSpeed * dt * cosf((m_camRot.y*pi) / 180.0f);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			m_camPos.z += moveSpeed * dt * -sinf((m_camRot.y*pi) / 180.0f);
			m_camPos.x -= moveSpeed * dt * cosf((m_camRot.y*pi) / 180.0f);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			m_camPos.z += moveSpeed * dt * cosf((m_camRot.y*pi) / 180.0f);
			m_camPos.x -= moveSpeed * dt * sinf((m_camRot.y*pi) / 180.0f);
			m_camPos.y += moveSpeed * dt * sinf((m_camRot.x*pi) / 180.0f);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			m_camPos.z -= moveSpeed * dt * cosf((m_camRot.y*pi) / 180.0f);
			m_camPos.x += moveSpeed * dt * sinf((m_camRot.y*pi) / 180.0f);
			m_camPos.y -= moveSpeed * dt * sinf((m_camRot.x*pi) / 180.0f);
		}
		//***

		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		glRotatef(m_camRot.x, 1, 0, 0);
		glRotatef(m_camRot.y, 0, 1, 0);
		glTranslatef(-m_camPos.x, -m_camPos.y, -m_camPos.z);

		printError("after camera movement");
		// draw...
		GLfloat lightPos[] = { m_light.x, m_light.y, m_light.z, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		//draw scene without transparent objects
		for (int i = 0; i < scene.size(); ++i)
		{
			display(scene.at(i), false, lod);
		}
		//***

		//draw transparents
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

		for (int i = 0; i < scene.size(); ++i)
		{
			scene.at(i).dist = distance(m_camPos, scene.at(i).position);
		}

		if (sorting)
			std::sort(scene.begin(), scene.end());

		for (int i = scene.size() - 1; i >= 0; --i)
			display(scene.at(i), true, lod);

		glDisable(GL_BLEND);
		//****

		//end the current frame (internally swaps the front and back buffers)
		m_window.display();

		//DEBUG INFORMATION
		if (cnt > 1 && showInfo)
		{
			//std::cout << cam_x << "\t" << cam_y << "\t" << cam_z << "\t" << cam_h << "\t" << cam_v << std::endl;
			std::cout << 1 / dt << std::endl;
			cnt = 0;
		}
		cnt += dt;
		//***

		mouse_old = sf::Vector2f(sf::Mouse::getPosition());
		dt = clock.getElapsedTime().asSeconds();
	}
}
