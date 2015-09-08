/*
 * Visualizer.h
 *
 *  Created on: 2 Sep 2015
 *      Author: becker
 */

#ifndef VISUALIZER_H_
#define VISUALIZER_H_
#define GLEW_STATIC
#define GLV_MAJOR_VERSION 3
#define GLV_MINOR_VERSION 1

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <thread>
#include <mutex>

/// Class to visulize a system of discs
/** This class is used to visulize DEM results using openGL 3.2 or higher
     it depends on SFML, OpenGL, SOIL
     this packeage must be installed on your system
*/

class Visualizer {
public:
	Visualizer(); ///< standart Konstruktur - 800x600, no antialising;
	~Visualizer();

	///initialize openGL window
	void initializeWindow(
			const int &width, ///< windows width in points
			const int &hight, ///< windows hight in points
			const int &antialising=0, ///< antialising level
			const int &dephbits=24, ///<  dephbuffer width
			const int &stecilBufferBits=8); ///< stencil Buffer width

	///initialize Particle pointer and System zise
	void initializeSystem(
			const float* particles, ///< Pointer to the particle positions
			const int &numberOfParticles,   ///< Number of particles
			const float &radius,  ///< Particle radius, particle must be monodispersed s far
			const int &lengthX, ///< width of the visualization box
			const int &lengthY, // hight of the visualization box
			const int &offsetX=0,  ///< offset X for the vis. box
			const int &offsetY=0,  ///< offset X for the vis. box
			const int &sides=15) ///< number of sides for circle approximation
	{
		this->particles=particles;
		this->Nparticle=numberOfParticles;
		this->lengthX=lengthX;
		this->lengthY=lengthY;
		this->offsetX=offsetX;
		this->offsetY=offsetY;
		this->radius=radius;
		initializeGL(sides);
	}

	//initalize openGL stuff
	void initializeGL(
			const int &sides); ///< number of sites used for

	/// returns the number of particles
	/// \return the number of particles
	const int& getNparticle() const {
		return Nparticle; ///< number of particles
	}

	/// update the image
	void updateimage() {
		graphicsNeedsUpdate=true;
	};

	/// close the gl window
	void close() {
		if(visthread->joinable()) {
			puts("Close Window \n");
			windowMustClosed=true;
			visthread->join();
		}
	}

	void snapshot(const char* filename) {
		snapshotmutex.lock();
		this->filename=filename;
		takeSnapshot=true;
		snapshotmutex.unlock();
	}


private:
	sf::RenderWindow * window=nullptr; ///< Window for openGL output
	const GLfloat * particles=nullptr; //< Pointer to particle Positions
	int Nparticle=0; //<particle Number
	float lengthX=0; ///<< X Width of the visualization area
	float lengthY=0; ///<< Y Width of the visualization area
	float radius=0; ///<< Particle Radius;
	float offsetX=0; ///< X offset of visualization area
	float offsetY=0; ///< Y offset of visualization area

	bool glinitokay=false; ///< is set to true when opengl is initialised
	GLuint *vertexBufferObject=nullptr; ///< pointer to vertex buffer identifier
	GLuint vertexShader; ///< pointer to vertex Shader
	GLuint geometryShader; ///< pointer to geometry Shader
	GLuint fragmentShader; ///< pointer to fragment Shader
	GLuint shaderProgram; ///<pointer to the shader program
	GLuint vertexArrayObject; ///<vertx array object index

	std::mutex snapshotmutex;
	std::thread *visthread=nullptr; ///<Thread for openGL output

	///Events for the thread in the thread, should replaced by
	///an event queue
	bool graphicsNeedsUpdate=false; ///<Flag wich indicate that the GL thread                                /// should update the windwo
	bool windowMustClosed=false;  ///< flag to close the thread
	bool takeSnapshot=false;
	const char* filename; ///<filename

	int acticeWriteBuffer=0;

	/// The gl main loop
	void glMainloop();

	///Compile a Shader file
	GLuint loadAndCompileShader(
			const GLenum &type, ///< Type of Shader,e.g. GL_FRAGMENT_SHADER
			const char* filename) ///< Filename of Shader source
	const;
};

#endif /* VISUALIZER_H_ */
