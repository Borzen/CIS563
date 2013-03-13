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

// Returns true iif v1 can be considered equal to v2
bool is_near(float v1, float v2){
	return fabs( v1-v2 ) < 0.01f;
}

// Searches through all already-exported vertices
// for a similar one.
// Similar = same position + same UVs + same normal
bool getSimilarVertexIndex( 
	glm::vec3 & in_vertex, 
	glm::vec3 & in_normal, 
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec3> & out_normals,
	unsigned short & result
){
	// Lame linear search
	for ( unsigned int i=0; i<out_vertices.size(); i++ ){
		if (
			is_near( in_vertex.x , out_vertices[i].x ) &&
			is_near( in_vertex.y , out_vertices[i].y ) &&
			is_near( in_vertex.z , out_vertices[i].z ) &&
			is_near( in_normal.x , out_normals [i].x ) &&
			is_near( in_normal.y , out_normals [i].y ) &&
			is_near( in_normal.z , out_normals [i].z )
		){
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

void indexVBO_slow(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec3> & in_normals,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec3> & out_normals
){
	// For each input vertex
	for ( unsigned int i=0; i<in_vertices.size(); i++ ){

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex(in_vertices[i], in_normals[i], out_vertices, out_normals, index);

		if ( found ){ // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back( index );
		}else{ // If not, it needs to be added in the output data.
			out_vertices.push_back( in_vertices[i]);
			out_normals .push_back( in_normals[i]);
			out_indices .push_back( (unsigned short)out_vertices.size() - 1 );
		}
	}
}

bool importGEO(const char * path, std::vector<int> & out_nVertices, std::vector<unsigned short> & out_vertexindex, std::vector<glm::vec3> & out_vertexies, std::vector<glm::vec3> &out_normals){
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
			if(buf[1] == 'v'){
				fgets(buf,100,f);
				i--;
			}
			else{
			fscanf(f,"%i\n",&out_vertexindex[i]);
			if(max < out_vertexindex[i]){
				max = out_vertexindex[i];
			}
			}
		}
		out_vertexies.resize(max+1);
		std::vector<glm::vec3> tNormals;
		out_normals.resize(size);
		fgets(buf,100,f);
		fgets(buf,100,f);
		fgets(buf,100,f);
		fgets(buf,100,f);
		int i = 0;
		while(!feof(f)){
			if(buf[1] == 'l'){
				fgets(buf,100,f);
			}
			else{
				float a=0,b=0,c=0,d = 0;
				fscanf(f,"%f %f %f %f\n",&a,&b,&c,&d);
				out_vertexies[i] = glm::vec3(a,b,c);
				i++;
			}
		}
		fclose(f);
		int j = 0;
		i = 0;
		while(i < size){
			int a = 0;
			a = out_nVertices[i];
			std::vector<vec3> temp1;
			std::vector<vec3> temp2;
			temp1.resize(1);
			temp2.resize(1);
			std::vector<unsigned short> tVL;
			tVL.resize(3);
			tVL[0] = out_vertexindex[j];
			tVL[1] = out_vertexindex[j+1];
			tVL[2] = out_vertexindex[j+2];
			j += a;
			temp1[0].x = (out_vertexies[tVL[0]].x-out_vertexies[tVL[1]].x);
			temp1[0].y = (out_vertexies[tVL[0]].y-out_vertexies[tVL[1]].y);
			temp1[0].z = (out_vertexies[tVL[0]].z-out_vertexies[tVL[1]].z);

			temp2[0].x = (out_vertexies[tVL[2]].x-out_vertexies[tVL[1]].x);
			temp2[0].y = (out_vertexies[tVL[2]].y-out_vertexies[tVL[1]].y);
			temp2[0].z = (out_vertexies[tVL[2]].z-out_vertexies[tVL[1]].z);

			out_normals[i] = normalize(vec3((temp2[0].y*temp1[0].z)-(temp1[0].y*temp2[0].z),
				(temp2[0].x*temp1[0].z)-(temp2[0].z*temp1[0].x),
				(temp2[0].x*temp1[0].y)-(temp2[0].y*temp1[0].x)));
			i++;
		}
		return true;
	}

}

int main( void )
{
	std::vector<int> nVertices;
	std::vector<unsigned short> vertexindex;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	importGEO("scenefile1.geo",nVertices,vertexindex,vertices,normals);
	
	std::vector<unsigned short> o_vertexindex;
	std::vector<glm::vec3> o_vertices;
	std::vector<glm::vec3> o_normals;
	//indexVBO_slow(vertices,normals,o_vertexindex,o_vertices,o_normals);
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
	
	
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	//errors on next line and crashes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexindex.size()*sizeof(unsigned short), &vertexindex[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(glm::vec3),&normals[0],GL_STATIC_DRAW);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER,vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	do{

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT );


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER,normalbuffer);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		// Draw the triangles !
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elementbuffer);
		glDrawElements(
			GL_TRIANGLES,      // mode
			vertexindex.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//Swap buffers
		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		   glfwGetWindowParam( GLFW_OPENED ) );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	// Cleanup VBO
	//glDeleteBuffers(1, &vertexbuffer);	
	//glDeleteBuffers(1, &normalbuffer);
	//glDeleteBuffers(1, &elementbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	return 0;
}