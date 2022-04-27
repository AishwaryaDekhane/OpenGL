#include <GL/freeglut.h>

bool bFullscreen = false;

GLfloat angleTri = 0.0f;
GLfloat angleSquare = 0.0f;

int main(int argc, char** argv)
{
	void display(void);
	void resize(int, int);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void update(void);
	void initialize(void);
	void uninitialize(void);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(800, 600);

	glutInitWindowPosition(100, 100);

	glutCreateWindow("Pyaramid And Cube using GLUT - Aishwarya");

	initialize();

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutIdleFunc(update);
	glutCloseFunc(uninitialize);

	glutMainLoop();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//PYARAMID

	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -6.0f);

	glRotatef(angleTri, 0.0f, 1.0f, 0.0f);

	glBegin(GL_TRIANGLES);

	//FRONT FACE
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//RIGHT FACE
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//BACK FACE
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	//LEFT FACE
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glEnd();

	//CUBE

	glLoadIdentity();					
	glTranslatef(1.5f, 0.0f, -6.0f);				//sequence should be always TSR
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(angleSquare, 1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);

	//TOP FACE
	glColor3f(1.0f, 0.0f, 0.0f);

	glVertex3f(1.0f, 1.0f, -1.0f);

	glVertex3f(-1.0f, 1.0f, -1.0f);

	glVertex3f(-1.0f, 1.0f, 1.0f);

	glVertex3f(1.0f, 1.0f, 1.0f);

	//BOTTOM FACE
	glColor3f(0.0f, 1.0f, 0.0f);

	glVertex3f(1.0f, -1.0f, -1.0f);

	glVertex3f(-1.0f, -1.0f, -1.0f);

	glVertex3f(-1.0f, -1.0f, 1.0f);

	glVertex3f(1.0f, -1.0f, 1.0f);

	//FRONT FACE
	glColor3f(0.0f, 0.0f, 1.0f);

	glVertex3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-1.0f, 1.0f, 1.0f);

	glVertex3f(-1.0f, -1.0f, 1.0f);

	glVertex3f(1.0f, -1.0f, 1.0f);

	//BACK FACE
	glColor3f(1.0f, 1.0f, 0.0f);

	glVertex3f(1.0f, 1.0f, -1.0f);

	glVertex3f(-1.0f, 1.0f, -1.0f);

	glVertex3f(-1.0f, -1.0f, -1.0f);

	glVertex3f(1.0f, -1.0f, -1.0f);

	//LEFT FACE
	glColor3f(0.0f, 1.0f, 1.0f);

	glVertex3f(-1.0f, 1.0f, 1.0f);

	glVertex3f(-1.0f, 1.0f, -1.0f);

	glVertex3f(-1.0f, -1.0f, -1.0f);

	glVertex3f(-1.0f, -1.0f, 1.0f);

	//RIGHT FACE
	glColor3f(1.0f, 0.0f, 1.0f);

	glVertex3f(1.0f, 1.0f, -1.0f);

	glVertex3f(1.0f, 1.0f, 1.0f);

	glVertex3f(1.0f, -1.0f, 1.0f);

	glVertex3f(1.0f, -1.0f, -1.0f);

	glEnd();
	glutSwapBuffers();
}

void initialize(void)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		glutLeaveMainLoop();
		break;

	case 'F':
	case 'f':
		if (bFullscreen == false)
		{
			glutFullScreen();
			bFullscreen = true;
		}
		else
		{
			glutLeaveFullScreen();
			bFullscreen = false;
		}
		break;
	default:
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

	default:
		break;
	}
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void update(void)
{
	angleTri = angleTri + 0.05f;
	if (angleTri >= 360.0f)
		angleTri = angleTri - 360.0f;

	angleSquare = angleSquare + 0.05f;
	if (angleSquare >= 360.0f)
		angleSquare = angleSquare - 360.0f;

	glutPostRedisplay();
}

void uninitialize(void)
{
	//code
}