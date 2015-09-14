/*
 * Visualizer.cpp
 *
 *  Created on: 2 Sep 2015
 *      Author: becker
 */
#include "Visualizer.h"
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SOIL/SOIL.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include <X11/Xlib.h>

Visualizer::Visualizer() {
	XInitThreads();
	initializeWindow(800,800);
}

Visualizer::Visualizer(const int & width,const int & hight) {
	XInitThreads();
	initializeWindow(width,hight);
}

Visualizer::~Visualizer() {
	if(glinitokay) {
		glDeleteProgram(shaderProgram);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteShader(geometryShader);
		glDeleteBuffers(1, vertexBufferObject);
		glDeleteVertexArrays(1,&vertexArrayObject);
		delete vertexBufferObject;
	}
	if(visthread!=nullptr) {
		this->close(); //warte bis thread beendet
		delete visthread;
		visthread=nullptr;
	}
	if(this->window != nullptr) {
		window->close();
		delete window;
	}

}

void Visualizer::initializeWindow(
			const int &width, ///< windows width in points
			const int &hight, ///< windows hight in points
			const int &antialising, ///< antialising level
			const int &dephbits, ///<  dephbuffer width
			const int &stecilBufferBits)  ///< stencil Buffer width
{
	sf::ContextSettings settings;
	settings.depthBits = dephbits;
	settings.stencilBits = stecilBufferBits;
	settings.majorVersion = GLV_MAJOR_VERSION;
	settings.minorVersion = GLV_MINOR_VERSION;
	settings.antialiasingLevel=antialising;
	if(window!=nullptr) {
		window->close();
		delete window;
		//delete existing window if it exists
	}
	window=new sf::RenderWindow(sf::VideoMode(width, hight, 32), "OpenGL",
			sf::Style::Titlebar | sf::Style::Close, settings);
	window->setVerticalSyncEnabled(true);
	window->display();
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	glewInit();
}

void Visualizer::initializeGL(const int &sides) {
	if(particles == nullptr || window == nullptr) {
		std::cerr << "Particles and Window must be initialized before setting up "
				"openGL Buffers" << std::endl;
		exit(EXIT_FAILURE);
	}
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	vertexBufferObject = new GLuint[1];
	glGenBuffers(1, vertexBufferObject); // Create a vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, *vertexBufferObject); // activate vertex Buffer

	glBufferData(GL_ARRAY_BUFFER,Nparticle*2*sizeof(GLfloat), particles,
					GL_DYNAMIC_DRAW); //write positions to buffer



	// Shader Compilation
	vertexShader = loadAndCompileShader(GL_VERTEX_SHADER,"/home/becker/workspace/openGLvisualizer/vertexShader.gls");
	geometryShader = loadAndCompileShader(GL_GEOMETRY_SHADER,"/home/becker/workspace/openGLvisualizer/geometryShader.gls");
	fragmentShader = loadAndCompileShader(GL_FRAGMENT_SHADER,"/home/becker/workspace/openGLvisualizer/fragmentShader.gls");

	// Link the shaders to a shader program
	// Link the vertex and fragment shader into a shader program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glAttachShader(shaderProgram, geometryShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),0);

	//Setup uniforms
	GLuint uniBoxHalfDimension=glGetUniformLocation(shaderProgram,"halfBoxDimension");
	glUniform2f(uniBoxHalfDimension,0.5f*lengthX,0.5f*lengthY);

	GLuint uniOffset=glGetUniformLocation(shaderProgram,"offset");
	glUniform2f(uniOffset,offsetX,offsetY);

	GLuint uniScaledRadius=glGetUniformLocation(shaderProgram,"scaledRadius");
	glUniform2f(uniScaledRadius,radius/lengthX*2,radius/lengthX*2);

	GLuint uniSides=glGetUniformLocation(shaderProgram,"sides");
	glUniform1i(uniSides,sides);
	this->glinitokay=true;
	window->setActive(false); // deactivate window to allow the other thread to use it
	visthread=new std::thread(&Visualizer::glMainloop,this); //starte glMainloop
}


void Visualizer::glMainloop() {
	window->setActive();
	while (window->isOpen()) {
		//evaluate events
		sf::Event windowEvent;
		while (window->pollEvent(windowEvent)) {
			switch (windowEvent.type) {
			case sf::Event::Closed:
				window->close();
				puts("Window terminated by user \n");
				break;
			default:
				break;
			}
		}
		//thread "events" todo: Replace by an thread save queue
		if(takeSnapshot) {
			snapshotmutex.lock();
			window->capture().saveToFile(this->filename);
			snapshotmutex.unlock();
		}
		if(graphicsNeedsUpdate) {
			glBufferData(GL_ARRAY_BUFFER, Nparticle * 2 * sizeof(GLfloat),
					particles,
					GL_DYNAMIC_DRAW); //write positions to buffer
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_POINTS, 0, Nparticle);
			window->display();
			graphicsNeedsUpdate=false;
		}
		if(windowMustClosed) {
			window->close();
			puts("Window terminated by function call \n");
			windowMustClosed=false;
		}
	}
	puts("visualization thread ready, good by...\n ");
}

GLuint Visualizer::loadAndCompileShader(const GLenum &type, const char* filename) const{
	std::ifstream sourceFile(filename); //open file
	std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
    std::istreambuf_iterator<char>()); //copy file to c_string
	GLuint shader = glCreateShader(type);
	GLint status=GL_TRUE;
	const char *src=sourceCode.c_str();
    glShaderSource(shader, 1,&src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		std::cerr << "compilation of " << type <<"  shader failed \n";
		char buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, buffer);
		std::cerr << buffer;
		exit(EXIT_FAILURE);
	}
	return shader;
}



