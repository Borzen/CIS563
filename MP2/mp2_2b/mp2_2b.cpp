// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

bool importGEO(const char * path, std::vector<int> & out_nVertices, std::vector<int> & out_vertexindex, std::vector<glm::vec3> & out_vertexies){
	FILE* f;
	int sumOfN = 0;
	f = fopen(path,"r");
	int size = 0;
	char buf[100];
	if(f == NULL){
		perror ("Error opening file");
		return false;
	}
	else
	{
		fgets(buf,100,f);
		fgets(buf,100,f);
		fscanf(f,"%i\n",&size);
		fgets(buf,100,f);
		fgets(buf,100,f);
		fgets(buf,100,f);
		out_nVertices.resize(size);
		for(int i = 0; i < size; i++){
			int  b = 0;
			fscanf(f,"%i\n",&b);
			sumOfN += b;
			out_nVertices[i] = b;
		}
		out_vertexindex.resize(sumOfN);
		fgets(buf,100,f);
		fgets(buf,100,f);
		fgets(buf,100,f);
		int max = 0;
		for(int i = 0; i < sumOfN; i++){
			fscanf(f,"%i\n",&out_vertexindex[i]);
			if(max < out_vertexindex[i]){
				max = out_vertexindex[i];
			}
		}
		out_vertexies.resize(max+2);
		fgets(buf,100,f);
		fgets(buf,100,f);
		fgets(buf,100,f);
		fgets(buf,100,f);
		int i = 0;
		//out_vertexies[i] = glm::vec3 (0,0,0);
		//i++;
		while(!feof(f)){
			float a=0,b=0,c=0,d = 0;
			fscanf(f,"%f %f %f %f\n",&a,&b,&c,&d);
			out_vertexies[i] = glm::vec3(a,b,c);
			i++;
		}
		fclose(f);
		return true;
	}

}

int main( void )
{
	std::vector<int> nVertices;
	std::vector<int> vertexindex;
	std::vector<glm::vec3> vertices;
	importGEO("pCylinder1.geo",nVertices,vertexindex,vertices);
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( 1024, 768, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glfwSetWindowTitle( "MP2" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	
	//static const GLfloat g_vertex_buffer_data[] = { 
	//	-1.0f, -1.0f, 0.0f,
	//	 1.0f, -1.0f, 0.0f,
	//	 0.0f,  1.0f, 0.0f,
	//};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(glm::vec3),&vertices[0],GL_STATIC_DRAW);

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	do{

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT );


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		   glfwGetWindowParam( GLFW_OPENED ) );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);


	return 0;
}