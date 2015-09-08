//============================================================================
// Name        : openGLvisulizer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "Visualizer.h"
#include <iostream>
#include <unistd.h>
///Test der Funktionalität
int main(void) {
	GLfloat particles[] {
			50.0f, 50.0f,
			75.0f, 75.0f,
			25.0f,25.0f
	};
	Visualizer output;
	output.initializeSystem(particles,3,1,100,100,24,25,15);
	for(int i=0;i<100;++i) {
		particles[0]=(float)i;
		output.updateimage();
		usleep(20000);
	}
	output.snapshot("test.png");
	output.close();
	puts("!!!Hello World!!!");
	return EXIT_SUCCESS;
}
