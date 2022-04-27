#include<GL/freeglut.h>

bool bIsFullScreen = false;

int year = 0, day = 0;

int main(int argc, char* argv[])
{
	// fuction declarations

	void initialize(void);
	void uninitialize(void);
	void reshape(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);

	//code

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA );
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("My First OpenGL Program - Aishwarya Dekhane");

	initialize();

	//call backs

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);

	glutMainLoop();

	return (0);

}

void initialize(void)
{
	//code

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_FLAT);
}

void uninitialize(void)
{
	//code
}

void reshape(int width, int height)
{
	//code

	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f, (GLfloat)width / (GLfloat)height, 1.0f, 20.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display(void)
{
	//code

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glutWireSphere(1.0f, 20.0f, 16.0f); //sun
	glRotatef((GLfloat)year, 0.0f, 1.0f, 0.0f); //revaluation
	glTranslatef(2.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)day, 0.0f, 1.0f, 0.0f); //rotation
	glutWireSphere(0.2f, 10.0f, 8.0f); //earth
	glPopMatrix();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: glutLeaveMainLoop();
		break;

	case 'F':
	case 'f':
		if (bIsFullScreen == false)
		{
			glutFullScreen();
			bIsFullScreen = true;
		}
		else
		{
			glutLeaveFullScreen();
			bIsFullScreen = false;
		}
		break;

	case 'Y': year = (year + 5) % 360;
		glutPostRedisplay();
		break;

	case 'y': year = (year - 5) % 360;
		glutPostRedisplay();
		break;

	case 'D': day = (day + 5) % 360;
		glutPostRedisplay();
		break;

	case 'd': day = (day - 5) % 360;
		glutPostRedisplay();
		break;

	}
}

void mouse(int button, int state, int x, int y)
{
	//code
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;

	case GLUT_RIGHT_BUTTON:
		glutLeaveMainLoop();
		break;
	}
}