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
	glm::vec3 ** in_vertex, 
	glm::vec3 ** in_normal, 
	std::vector<glm::vec3> ** out_vertices,
	std::vector<glm::vec3> ** out_normals,
	unsigned short & result
){
	// Lame linear search
	for ( unsigned int i=0; i<(*out_vertices)->size(); i++ ){
		if (
			is_near( (*in_vertex)->x , (*out_vertices)->at(i).x ) &&
			is_near( (*in_vertex)->y , (*out_vertices)->at(i).y ) &&
			is_near( (*in_vertex)->z , (*out_vertices)->at(i).z ) &&
			is_near( (*in_normal)->x , (*out_normals)->at(i).x ) &&
			is_near( (*in_normal)->y , (*out_normals)->at(i).y ) &&
			is_near( (*in_normal)->z , (*out_normals)->at(i).z )
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
	std::vector<glm::vec3> ** in_vertices,
	std::vector<glm::vec3> ** in_normals,

	std::vector<unsigned short> ** out_indices,
	std::vector<glm::vec3> ** out_vertices,
	std::vector<glm::vec3> ** out_normals
){
	// For each input vertex
	for ( unsigned int i=0; i< (*in_vertices)->size(); i++ ){
		glm::vec3 temp1 = (*in_vertices)->at(i);
		glm::vec3 temp2 = (*in_normals)->at(i);
		glm::vec3 * ptemp1 = &temp1;
		glm::vec3 * ptemp2 = &temp2;
		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex( &ptemp1, &ptemp2, out_vertices, out_normals, index);

		if ( found ){ // A similar vertex is already in the VBO, use it instead !
			(*out_indices)->push_back( index );
		}else{ // If not, it needs to be added in the output data.
			(*out_vertices)->push_back( (*in_vertices)->at(i));
			(*out_normals)->push_back( (*in_normals)->at(i));
			(*out_indices)->push_back( (unsigned short)(*out_vertices)->size() - 1 );
		}
	}
}

bool importGEO(const char * path, std::vector<int> ** out_nVertices, std::vector<unsigned short> ** out_vertexindex, std::vector<glm::vec3> ** out_vertexies, std::vector<glm::vec3> ** out_normals){
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
		(*out_nVertices)->resize(size);
		for(int i = 0; i < size; i++){
			int  b = 0;
			fscanf(f,"%i\n",&b);
			sumOfN += b;
			(*out_nVertices)->at(i) = b;
		}
		(*out_vertexindex)->resize(sumOfN);
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
				fscanf(f,"%i\n",(*out_vertexindex)->at(i));
			if(max < (*out_vertexindex)->at(i)){
				max = (*out_vertexindex)->at(i);
			}
			}
		}
		(*out_vertexies)->resize(max+1);
		(*out_normals)->resize(size);
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
				(*out_vertexies)->at(i) = glm::vec3(a,b,c);
				i++;
			}
		}
		fclose(f);
		int j = 0;
		i = 0;
		while(i < size){
			int a = 0;
			a = (*out_nVertices)->at(i);
			std::vector<vec3> temp1;
			std::vector<vec3> temp2;
			temp1.resize(1);
			temp2.resize(1);
			std::vector<unsigned short> tVL;
			tVL.resize(3);
			tVL[0] = (*out_vertexindex)->at(j);
			tVL[1] = (*out_vertexindex)->at(j+1);
			tVL[2] = (*out_vertexindex)->at(j+2);
			j += a;
			temp1[0].x = ((*out_vertexies)->at(tVL[0]).x-(*out_vertexies)->at(tVL[1]).x);
			temp1[0].y = ((*out_vertexies)->at(tVL[0]).y-(*out_vertexies)->at(tVL[1]).y);
			temp1[0].z = ((*out_vertexies)->at(tVL[0]).z-(*out_vertexies)->at(tVL[1]).z);

			temp2[0].x = ((*out_vertexies)->at(tVL[2]).x-(*out_vertexies)->at(tVL[1]).x);
			temp2[0].y = ((*out_vertexies)->at(tVL[2]).y-(*out_vertexies)->at(tVL[1]).y);
			temp2[0].z = ((*out_vertexies)->at(tVL[2]).z-(*out_vertexies)->at(tVL[1]).z);

			(*out_normals)->at(i) = normalize(vec3((temp2[0].y*temp1[0].z)-(temp1[0].y*temp2[0].z),
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
	//importGEO("scenefile1.geo",&nVertices,&vertexindex,&vertices,&normals);
	
	FILE* f;
	int sumOfN = 0;
	f = fopen("scenefile1.geo","r");
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
		nVertices.resize(size);
		for(int i = 0; i < size; i++){
			int  b = 0;
			fscanf(f,"%i\n",&b);
			sumOfN += b;
			nVertices[i] = b;
		}
		vertexindex.resize(sumOfN);
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
				fscanf(f,"%i\n",&vertexindex[i]);
			if(max < vertexindex[i]){
				max = vertexindex[i];
			}
			}
		}
		vertices.resize(max+1);
		normals.resize(size);
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
				vertices[i] = glm::vec3(a,b,c);
				i++;
			}
		}
		fclose(f);
		int j = 0;
		i = 0;
		while(i < size){
			int a = 0;
			a = nVertices[i];
			std::vector<vec3> temp1;
			std::vector<vec3> temp2;
			temp1.resize(1);
			temp2.resize(1);
			std::vector<unsigned short> tVL;
			tVL.resize(3);
			tVL[0] = vertexindex[j];
			tVL[1] = vertexindex[j+1];
			tVL[2] = vertexindex[j+2];
			j += a;
			temp1[0].x = (vertices[tVL[0]].x-vertices[tVL[1]].x);
			temp1[0].y = (vertices[tVL[0]].y-vertices[tVL[1]].y);
			temp1[0].z = (vertices[tVL[0]].z- vertices[tVL[1]].z);

			temp2[0].x = (vertices[tVL[2]].x-vertices[tVL[1]].x);
			temp2[0].y = (vertices[tVL[2]].y-vertices[tVL[1]].y);
			temp2[0].z = (vertices[tVL[2]].z-vertices[tVL[1]].z);

			normals[i] = normalize(vec3((temp2[0].y*temp1[0].z)-(temp1[0].y*temp2[0].z),
				(temp2[0].x*temp1[0].z)-(temp2[0].z*temp1[0].x),
				(temp2[0].x*temp1[0].y)-(temp2[0].y*temp1[0].x)));
			i++;
		}

	std::vector<int> * o_nVertices;
	std::vector<unsigned short> * o_vertexindex;
	std::vector<glm::vec3> * o_vertices;
	std::vector<glm::vec3> * o_normals;
	//indexVBO_slow(&vertices,&normals,&o_vertexindex,&o_vertices,&o_normals);
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, o_vertexindex->size()*sizeof(unsigned short), &o_vertexindex[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER,o_normals->size()*sizeof(glm::vec3),&o_normals[0],GL_STATIC_DRAW);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER,vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, o_vertices->size()*sizeof(glm::vec3), &o_vertices[0], GL_STATIC_DRAW);

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
		glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		// Draw the triangles !
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elementbuffer);
		glDrawElements(
			GL_TRIANGLES,      // mode
			vertexindex->size(),    // count
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
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1,&elementbuffer);
	glDeleteBuffers(1,&normalbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	return 0;
}