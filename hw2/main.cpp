#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#include "Object.h"
#include "glew.h"
#include "glut.h"
#include "shader.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <thread>
#include <chrono>
#include "stb_image.h"
#include "Vertex.h"

using namespace std;

int windowSize[2];

void light();
void display();
void init();
void keyboard(unsigned char key, int x, int y);
void reshape(GLsizei, GLsizei);
void idle();

void shaderInit();
void textureInit();
void bindBufferMulti(Object* model);
void bindBufferSingle(Object* model);
void DrawSphere(float radius, float slice, float stack);
void drawModel(Object* model);
void LoadTexture(unsigned int& texture, const char* tFileName,int i);
void Sleep(int ms);

// using different method to buffer data, you can set "true" , "false" to see the result.
bool single_mode = false;

// feeling free to adjust below value to fit your computer efficacy.
#define Rotate_Speed 1
#define MAX_FPS 120

Object* Pikachu = new Object("Pikachu.obj");
unsigned int Pikachu_texture;
unsigned int ball_texture;

GLuint program;
GLuint VAO[2], VBO[2];

vector<VertexAttribute> ball;

// timer for FPS control
clock_t Start, End;
float speed = Rotate_Speed;

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("HW2");

	DrawSphere(3, 60, 30);
	glewInit();
	shaderInit();
	textureInit();
	if (single_mode) {
		bindBufferSingle(Pikachu);
	}
	else {
		bindBufferMulti(Pikachu);
	}

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}

void reshape(GLsizei w, GLsizei h) {
	windowSize[0] = w;
	windowSize[1] = h;
}

void idle() {
	// FPS control
	clock_t CostTime = End - Start;
	float PerFrameTime = 1000.0 / MAX_FPS;
	if (CostTime < PerFrameTime) {
		Sleep(ceil(PerFrameTime) - CostTime);
	}
	glutPostRedisplay();
}

void light()
{
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_position[] = { 10.0, 10.0, 5.0, 0.0 };
	glShadeModel(GL_SMOOTH);
	// z buffer enable
	glEnable(GL_DEPTH_TEST);
	// enable lighting
	glEnable(GL_LIGHTING);
	// set light property
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case 's':
	{
		if (speed > 0) {
			speed = 0;
		}
		else {
			speed = Rotate_Speed;
		}
		break;
	}

	default:
		break;
	}
}

void display() {
	Start = clock();
	init();
	light();
	drawModel(Pikachu);
	End = clock();

	glutPostRedisplay();
	glutSwapBuffers();
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// viewport transformation
	glViewport(0, 0, windowSize[0], windowSize[1]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)windowSize[0] / (GLfloat)windowSize[1], 1.0, 1000.0);

	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(7.5, 5.0, 7.5,// eye
		0.0, 0.0, 0.0,     // center
		0.0, 1.0, 0.0);    // up
}

void shaderInit() {
	GLuint vert = createShader("Shaders/texture.vert", "vertex");
	GLuint frag = createShader("Shaders/texture.frag", "fragment");
	program = createProgram(vert, frag);

}

void textureInit() {
	glEnable(GL_TEXTURE_2D);
	// bind Pikachu texture with GL_TEXTURE0
	LoadTexture(Pikachu_texture, "Pikachu.png",0);
	// bind Pokeball texture with GL_TEXTURE1 = (GL_TEXTURE0 + 1)
	LoadTexture(ball_texture, "Pokeball.png",1);
}

void bindBufferMulti(Object* model) {
	// create buffer & get buffer ID
	glGenVertexArrays(2, VAO);
	glGenBuffers(2, VBO);

	glBindVertexArray(VAO[0]);

	// array to store the infomation of model
	vector<VertexAttribute> Pikachu_data;
	VertexAttribute temp;
	// store the vetex infomation of Pikachu
	for (int i = 0; i < model->positions.size() / 3; i++) {
		int idx = i * 3;
		temp.setPosition(model->positions[idx], model->positions[idx + 1], model->positions[idx + 2]);
		idx = i * 2;
		temp.setTexcoord(model->texcoords[idx], model->texcoords[idx + 1]);
		Pikachu_data.push_back(temp);
	}

	// rendering pikachu
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * Pikachu_data.size(), &Pikachu_data[0], GL_STATIC_DRAW);
	// point to model vertex coordinate
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)0);
	// point to texture coordinate
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(3 * sizeof(float)));

	// using another VAO to render PokeBall
	glBindVertexArray(VAO[1]);

	// array to store the infomation of model
	vector<VertexAttribute> Ball_data;
	// store the vetex infomation of PokeBall
	for (int i = 0; i < ball.size(); i++) {
		Ball_data.push_back(ball[i]);
	}

	// rendering PokeBall (this VAO point to another VBO)
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * Ball_data.size(), &Ball_data[0], GL_STATIC_DRAW);
	// point to model vertex coordinate
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)0);
	// point to texture coordinate
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Bind all model data in only one VAO 
void bindBufferSingle(Object* model) {
	// create buffer & get buffer ID
	glGenVertexArrays(1, VAO);
	glGenBuffers(1, VBO);

	// bind the VAO
	glBindVertexArray(VAO[0]);

	// array to store the infomation of model
	vector<VertexAttribute> data;
	VertexAttribute temp;
	// store the vetex infomation of Pikachu
	for (int i = 0; i < model->positions.size() / 3; i++) {
		int idx = i * 3;
		temp.setPosition(model->positions[idx], model->positions[idx + 1], model->positions[idx + 2]);
		idx = i * 2;
		temp.setTexcoord(model->texcoords[idx], model->texcoords[idx + 1]);
		data.push_back(temp);
	}
	// store the vetex infomation of PokeBall
	for (int i = 0; i < ball.size(); i++) {
		data.push_back(ball[i]);
	}

	// buffer the data of all model (Pikachu & sphere)
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * data.size(), &data[0], GL_STATIC_DRAW);
	// point to model vertex coordinate
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)0);
	// point to texture coordinate
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(3 * sizeof(float)));

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void DrawSphere(float radius, float slice, float stack) {
	float theta, phi, xy_step = 360 / slice, z_step = 180 / stack;
	Vertex vert;
	float u, v;
	for (phi = -90; phi <= 90; phi += z_step) {
		VertexAttribute temp;
		for (theta = 0; theta <= 360; theta += xy_step) {
			vert.x = radius * sin(theta * M_PI / 180) * cos(phi * M_PI / 180);
			vert.y = radius * cos(theta * M_PI / 180) * cos(phi * M_PI / 180);
			vert.z = radius * sin(phi * M_PI / 180);
			temp.setPosition(vert);
			vert.normalize();
			u = atan2(vert.x, vert.z) / (2 * M_PI) + 0.5f;
			v = asin(vert.y) / M_PI + 0.5f;
			temp.setTexcoord(u, v);
			ball.push_back(temp);

			vert.x = radius * sin(theta * M_PI / 180) * cos((phi + z_step) * M_PI / 180);
			vert.y = radius * cos(theta * M_PI / 180) * cos((phi + z_step) * M_PI / 180);
			vert.z = radius * sin((phi + z_step) * M_PI / 180);
			temp.setPosition(vert);
			vert.normalize();
			u = atan2(vert.x, vert.z) / (2 * M_PI) + 0.5f;
			v = asin(vert.y) / M_PI + 0.5f;
			temp.setTexcoord(u, v);
			ball.push_back(temp);
		}
	}
}

void drawModel(Object* model) {
	static float r = 0;
	glRotatef(r += speed, 0, 1, 0);
	glPushMatrix();
	glScalef(5.0f, 5.0f, 5.0f);
	// get projection & modelview matrix
	GLfloat pmtx[16];
	GLfloat mmtx[16];
	glGetFloatv(GL_PROJECTION_MATRIX, pmtx);
	glGetFloatv(GL_MODELVIEW_MATRIX, mmtx);
	// get an integer that represents the location of a specific uniform variable within a program object(shader)
	GLint pmatLoc = glGetUniformLocation(program, "Projection");
	GLint mmatLoc = glGetUniformLocation(program, "ModelView");
	glPopMatrix();

	glUseProgram(program);
	//input the modelview matrix into vertex shader
	glUniformMatrix4fv(pmatLoc, 1, GL_FALSE, pmtx);
	//input the rotation matrix into vertex shader
	glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, mmtx);

	if (single_mode) {
		// render pikachu with its texture
		glUniform1i(glGetUniformLocation(program, "texture"), 0);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, model->positions.size() / 3);

		// translate the PekoBall position
		glPushMatrix();
		glRotatef(0,1,0,0);
		glTranslatef(4.0f, 0.0f, 0.0f);
		glGetFloatv(GL_MODELVIEW_MATRIX, mmtx);
		glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, mmtx);
		glPopMatrix();

		// render PekoBall with texture
		glUniform1i(glGetUniformLocation(program, "texture"), 1);
		glDrawArrays(GL_TRIANGLE_STRIP, model->positions.size() / 3, ball.size());
	}
	else {
		// render pikachu with its texture
		glUniform1i(glGetUniformLocation(program, "texture"), 0);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, model->positions.size() / 3);

		//translate the PekoBall position
		glPushMatrix();
			glRotatef(0, 1, 0, 0);
			glTranslatef(4.0f, 0.0f, 0.0f);
			glGetFloatv(GL_MODELVIEW_MATRIX, mmtx);
			glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, mmtx);
		glPopMatrix();

		// render PekoBall with texture
		glUniform1i(glGetUniformLocation(program, "texture"), 1);
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, ball.size());

		// It's more flexible and easy for using . if you want to render another Pikachu ,
		// you just need to bind the VAO which store the data of Pikachu.
		// you don't need to remeber the start index of Pikachu in buffer. 
		// 
		//glPushMatrix();
		//	glTranslatef(0.0f, 5.0f, 0.0f);
		//	glGetFloatv(GL_MODELVIEW_MATRIX, mmtx);
		//	glUniformMatrix4fv(mmatLoc, 1, GL_FALSE, mmtx);
		//glPopMatrix();
		//glUniform1i(glGetUniformLocation(program, "texture"), 0);
		//glBindVertexArray(VAO[0]);
		//glDrawArrays(GL_TRIANGLES, 0, model->positions.size() / 3);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

// i indicate the texture unit number
void LoadTexture(unsigned int& texture, const char* tFileName, int i) {
	glActiveTexture(GL_TEXTURE0 + i);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(tFileName, &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		cout << "Failed to load texture" << endl;
	}
	
	stbi_image_free(data);
}

void Sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
