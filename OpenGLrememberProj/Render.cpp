#include "Render.h"

//#include "glext.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"


inline double fL1(double p1, double p2, double p3, double t)
{
	return p1 * (1 - t) * (1 - t) + 2 * p2 * t * (1 - t) + p3 * t * t;
}

inline double f(double p1, double p2, double p3, double p4, double t)
{
	return p1 * (1 - t) * (1 - t) * (1 - t) + 3 * p2 * t * (1 - t) * (1 - t) + 3 * p3 * t * t * (1 - t) + p4 * t * t * t;
}
inline double ff(double p1, double p2, double r1, double r2, double t)
{
	return p1 * (2 * t * t * t - 3 * t * t + 1) + p2 * (-2 * t * t * t + 3 * t * t) + r1 * (t * t * t - 2 * t * t + t) + r2 * (t * t * t - t * t);
}

//������������ ����� �����
class a {
public:
	float f = 0;
	bool tuda = true;
	float h = 0.275; //-------���� ���� ��������
	void operator ++() {

		if (tuda)
		{
			if (f <= 1.0001)
			{
				f += h;
			}
			else
			{
				f -= h;
				if (f < 0)
					f = 0;
				tuda = false;
			}
		}
		else
		{
			if (f > 0)
			{
				f -= h;
				if (f < 0)
					f = 0;
			}
			else
			{
				f += h;
				tuda = true;
			}
		}
	}
};
a PEPEGA;


bool textureMode = true;
bool lightMode = false;
bool ispaused = false;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;


	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}


	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 32)
	{
		ispaused = !ispaused;
	}

	if (key == 'Y')
	{
		PEPEGA.h += 0.005;
		if (PEPEGA.h > 0.5)
			PEPEGA.h = 0.5;
	}

	if (key == 'H')
	{
		PEPEGA.h -= 0.005;
		if (PEPEGA.h < 0)
			PEPEGA.h = 0;
	}

	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;

//����������� ����� ������ ��������
void initRender(OpenGL* ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	////�������� ��������
	//glEnable(GL_TEXTURE_2D);


	////������ ����������� ���������  (R G B)
	//RGBTRIPLE* texarray;

	////������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	//char* texCharArray;
	//int texW, texH;
	//OpenGL::LoadBMP("bulba.bmp", &texW, &texH, &texarray);
	//OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	////���������
	////���������� �� ��� ��������
	//glGenTextures(1, &texId);
	////������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	//glBindTexture(GL_TEXTURE_2D, texId);

	////��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	////�������� ������
	//free(texCharArray);
	//free(texarray);

	////������� ����
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH);


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}




void Render(OpenGL* ogl)
{




	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//��������������
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  


	//������ �����
	double Tushka_NePopa[] = { 0,0,4 };
	double Tushka_NePopa_h[] = { 2.5,0,2.5 };
	double Tushka_Popa_h[] = { 2,0,-4 };
	double Tushka_Popa[] = { 0,0,-5 };

	glFrontFace(GL_CW);
	glColor3b(97, 67, 37);
	double x_, y_, z_, z = Tushka_Popa[2], y, y2, rn, r, dz = (Tushka_NePopa[2] - Tushka_Popa[2]) / 100;
	double v1[3], v2[3], N[3];
	for (double t = 0; t < 1.0001; t += 0.01)
	{
		r = f(Tushka_Popa[0] + 1, Tushka_Popa_h[0], Tushka_NePopa_h[0], Tushka_NePopa[0] + 1, t);
		rn = f(Tushka_Popa[0] + 1, Tushka_Popa_h[0], Tushka_NePopa_h[0], Tushka_NePopa[0] + 1, t + 0.01);
		glBegin(GL_QUAD_STRIP);
		bool f1 = true;
		for (double x = r; x >= -r; x -= r / 50)
		{
			y = sqrt(r * r - x * x);
			y2 = sqrt(rn * rn - x * x);


			if (f1)
			{
				//������� �������

				v1[0] = 0;
				v1[1] = -1;
				v1[2] = 0;
				v2[0] = x - x;
				v2[1] = y - y2;
				v2[2] = z - (z + dz);
			}
			else
			{
				v1[0] = x_ - x;
				v1[1] = y_ - y;
				v1[2] = z_ - z;

				v2[0] = x - x;
				v2[1] = y - y2;
				v2[2] = z - (z + dz);
			}
			N[0] = v1[1] * v2[2] - v2[1] * v1[2];
			N[1] = -v1[0] * v2[2] + v2[0] * v1[2];
			N[2] = v1[0] * v2[1] - v2[0] * v1[1];
			glNormal3dv(N);
			//.

			glVertex3d(x, y, z);
			glVertex3d(f1 ? rn : x, y2, z + dz);

			x_ = x;
			y_ = y;
			z_ = z;
			f1 = false;
		}
		f1 = true;
		for (double x = -r; x <= r; x += r / 50)
		{
			y = -sqrt(r * r - x * x);
			y2 = -sqrt(rn * rn - x * x);


			if (f1)
			{
				//������� �������

				v1[0] = 0;
				v1[1] = 1;
				v1[2] = 0;
				v2[0] = x - x;
				v2[1] = y - y2;
				v2[2] = z - (z + dz);
			}
			else
			{
				v1[0] = x_ - x;
				v1[1] = y_ - y;
				v1[2] = z_ - z;

				v2[0] = x - x;
				v2[1] = y - y2;
				v2[2] = z - (z + dz);
			}
			N[0] = v1[1] * v2[2] - v2[1] * v1[2];
			N[1] = -v1[0] * v2[2] + v2[0] * v1[2];
			N[2] = v1[0] * v2[1] - v2[0] * v1[1];
			glNormal3dv(N);
			//.

			glVertex3d(x, y, z);
			glVertex3d(f1 ? -rn : x, y2, z + dz);

			x_ = x;
			y_ = y;
			z_ = z;
			f1 = false;
		}
		glEnd();

		z += dz;
	}

	//������ ������
	glPushMatrix();
	{
		glColor3ub(77, 130, 37);
		double tmp[3];
		double r, r2, z2, h_r = 1.2, dz = h_r / 100;
		double Head_Center[] = { Tushka_NePopa[0],Tushka_NePopa[1],Tushka_NePopa[2] + h_r * 0.45 };
		glTranslated(Head_Center[0], Head_Center[1], Head_Center[2]);
		for (double z = -h_r; z <= h_r; z += dz)
		{
			glBegin(GL_TRIANGLE_STRIP);
			if (z > h_r)
				z = h_r;

			tmp[2] = z;
			r = sqrt(h_r * h_r - z * z);

			for (double t = 0; t <= 2 * PI + 0.0001; t += PI / 100)
			{
				//1
				if (z == -h_r)
				{
					glNormal3d(0, 0, -h_r);
					glVertex3d(0, 0, -h_r);
				}
				else
				{
					tmp[0] = r * cos(t);
					tmp[1] = r * sin(t);
					glNormal3dv(tmp);
					glVertex3dv(tmp);
				}
				//2
				z2 = z + dz;
				r2 = sqrt(h_r * h_r - z2 * z2);
				if (z2 >= h_r)
				{
					glNormal3d(0, 0, h_r);
					glVertex3d(0, 0, h_r);
				}
				else
				{
					tmp[2] = z2;
					tmp[0] = r2 * sin(t);
					tmp[1] = r2 * cos(t);
					glNormal3dv(tmp);
					glVertex3dv(tmp);
				}
			}
			//
			glEnd();

		}
	}
	glPopMatrix();

	//������ �����
	{
		r = f(Tushka_Popa[0] + 1, Tushka_Popa_h[0], Tushka_NePopa_h[0], Tushka_NePopa[0] + 1, 0);
		glBegin(GL_TRIANGLE_FAN);
		glNormal3d(0, 0, -1);
		glVertex3d(Tushka_Popa[0], Tushka_Popa[1], Tushka_Popa[2] - 0.5);
		bool f1 = true;
		for (double x = r; x >= -r; x -= r / 50)
		{
			y = sqrt(r * r - x * x);

			if (f1)
			{
				//������� �������
				v1[0] = 0;
				v1[1] = -1;
				v1[2] = 0;
			}
			else
			{
				v1[0] = x_ - x;
				v1[1] = y_ - y;
				v1[2] = z_ - z;
			}
			v2[0] = Tushka_Popa[0] - x;
			v2[1] = Tushka_Popa[1] - y;
			v2[2] = (Tushka_Popa[2] - 0.3) - z;
			N[0] = v1[1] * v2[2] - v2[1] * v1[2];
			N[1] = -v1[0] * v2[2] + v2[0] * v1[2];
			N[2] = v1[0] * v2[1] - v2[0] * v1[1];
			glNormal3dv(N);
			//.
			glVertex3d(x, y, Tushka_Popa[2]);

			x_ = x;
			y_ = y;
			z_ = z;
			f1 = false;
		}
	}
	{
		r = f(Tushka_Popa[0] + 1, Tushka_Popa_h[0], Tushka_NePopa_h[0], Tushka_NePopa[0] + 1, 0);
		glVertex3d(Tushka_Popa[0], Tushka_Popa[1], Tushka_Popa[2] - 0.5);
		bool f1 = true;
		for (double x = -r; x <= r; x += r / 50)
		{
			y = -sqrt(r * r - x * x);

			if (f1)
			{
				//������� �������
				v1[0] = 0;
				v1[1] = -1;
				v1[2] = 0;
			}
			else
			{
				v1[0] = x_ - x;
				v1[1] = y_ - y;
				v1[2] = z_ - z;
			}
			v2[0] = Tushka_Popa[0] - x;
			v2[1] = Tushka_Popa[1] - y;
			v2[2] = (Tushka_Popa[2] - 0.3) - z;
			N[0] = v1[1] * v2[2] - v2[1] * v1[2];
			N[1] = -v1[0] * v2[2] + v2[0] * v1[2];
			N[2] = v1[0] * v2[1] - v2[0] * v1[1];
			glNormal3dv(N);
			//.
			glVertex3d(x, y, Tushka_Popa[2]);

			x_ = x;
			y_ = y;
			z_ = z;
			f1 = false;
		}
		glEnd();
	}
	//..
	//���� �����
	{
		r = f(Tushka_Popa[0] + 1, Tushka_Popa_h[0], Tushka_NePopa_h[0], Tushka_NePopa[0] + 1, 1.01);
		glBegin(GL_TRIANGLE_FAN);
		glNormal3d(0, 0, -1);
		glVertex3d(Tushka_NePopa[0], Tushka_NePopa[1], Tushka_NePopa[2] + 0.37);
		bool f1 = true;
		for (double x = r; x >= -r; x -= r / 50)
		{
			y = sqrt(r * r - x * x);

			if (f1)
			{
				//������� �������
				v1[0] = 0;
				v1[1] = 1;
				v1[2] = 0;
			}
			else
			{
				v1[0] = x - x_;
				v1[1] = y - y_;
				v1[2] = z - z_;
			}
			v2[0] = Tushka_NePopa[0] - x;
			v2[1] = Tushka_NePopa[1] - y;
			v2[2] = (Tushka_NePopa[2] - 0.37) - z;
			N[0] = v1[1] * v2[2] - v2[1] * v1[2];
			N[1] = -v1[0] * v2[2] + v2[0] * v1[2];
			N[2] = v1[0] * v2[1] - v2[0] * v1[1];
			glNormal3dv(N);
			//.
			glVertex3d(x, y, Tushka_NePopa[2]);

			x_ = x;
			y_ = y;
			z_ = z;
			f1 = false;
		}
	}
	{
		r = f(Tushka_Popa[0] + 1, Tushka_Popa_h[0], Tushka_NePopa_h[0], Tushka_NePopa[0] + 1, 1.01);
		bool f1 = true;
		for (double x = -r; x <= r; x += r / 50)
		{
			y = -sqrt(r * r - x * x);

			if (f1)
			{
				//������� �������
				v1[0] = 0;
				v1[1] = 1;
				v1[2] = 0;
			}
			else
			{
				v1[0] = x - x_;
				v1[1] = y - y_;
				v1[2] = z - z_;
			}
			v2[0] = Tushka_NePopa[0] - x;
			v2[1] = Tushka_NePopa[1] - y;
			v2[2] = (Tushka_NePopa[2] - 0.37) - z;
			N[0] = v1[1] * v2[2] - v2[1] * v1[2];
			N[1] = -v1[0] * v2[2] + v2[0] * v1[2];
			N[2] = v1[0] * v2[1] - v2[0] * v1[1];
			glNormal3dv(N);
			//.
			glVertex3d(x, y, Tushka_NePopa[2]);

			x_ = x;
			y_ = y;
			z_ = z;
			f1 = false;
		}
		glEnd();
	}
	//..
	glFrontFace(GL_CCW);
	//.

	//����� 1
	glPushMatrix();
	glRotated(12.7, 0, 0, 1);
	glRotatef(71 * PEPEGA.f, 0, 0, 1);
	{
		double wing_Up_h1[] = { 5,0,19 };
		double wing_Up_h2[] = { 12,0,5 };
		double wing_Down_h3[] = { 14.1,0,-3.6 };
		double wing_Down_h4[] = { 4.5,0,-15 };

		double wCircleUp[] = { 5.028,0,6 };
		double wCircleDown[] = { 5.169,0,-4.399 };

		double wingUp[] = { 0.9,0,3.9 };
		double wingFar[] = { 5.7,0,0.3 };
		double wingCenter[] = { 0.9,0,0 };
		double wingDown[] = { 0.9,0,-3.9 };
		glColor4ub(100, 150, 180, 177);
		glNormal3d(0, -1, 0);

		glBegin(GL_QUADS);
		glVertex3d(0, 0, wingUp[2]);
		glVertex3dv(wingUp);
		glVertex3dv(wingDown);
		glVertex3d(0, 0, wingDown[2]);
		glEnd();

		double tmp[3];
		short int R = 223;
		short int G = 115;
		short int B = 255;
		int i = 0;
		glBegin(GL_TRIANGLE_FAN);
		glVertex3dv(wingCenter);
		for (double t = 0; t <= 1.0001; t += 0.01, i++)
		{
			tmp[0] = f(wingUp[0], wing_Up_h1[0], wing_Up_h2[0], wingFar[0], t);
			tmp[1] = f(wingUp[1], wing_Up_h1[1], wing_Up_h2[1], wingFar[1], t);
			tmp[2] = f(wingUp[2], wing_Up_h1[2], wing_Up_h2[2], wingFar[2], t);
			glColor4ub(R, G, B, 177);
			glVertex3dv(tmp);
			R--;
			if (i % 2 == 0)
				G++;
			if (i % 4 == 0)
				B--;
			if (i > 26 && i < 39)
			{
				R--;
				G += 3;
			}
			if (i > 39 && i < 70)
			{
				G -= 2;
				B -= 4;
			}
			if (i > 70 && i < 90)
			{
				B += 2;
				R += 3;
			}
			if (i > 90)
			{
				B -= 9;
				R -= 6;
				G -= 3;
			}
		}

		for (double t = 0; t <= 1.0001; t += 0.01, i--)
		{
			tmp[0] = f(wingFar[0], wing_Down_h3[0], wing_Down_h4[0], wingDown[0], t);
			tmp[1] = f(wingFar[1], wing_Down_h3[1], wing_Down_h4[1], wingDown[1], t);
			tmp[2] = f(wingFar[2], wing_Down_h3[2], wing_Down_h4[2], wingDown[2], t);
			glColor4ub(R, G, B, 177);
			glVertex3dv(tmp);

			if (i % 2 == 0)
				G++;
			if (i % 4 == 0)
				B--;
			if (i > 26 && i < 39)
			{
				R--;
				G += 3;
			}
			if (i > 39 && i < 70)
			{
				G -= 2;
				B -= 1;
			}
			if (i > 70 && i < 90)
			{
				B += 2;
				R += 3;
			}
			if (i > 90)
			{
				B -= 3;
				R -= 2;
				G -= 1;
			}
		}
		glEnd();

		glPushMatrix();
		glTranslated(wCircleUp[0], wCircleUp[1], wCircleUp[2]);
		r = 1.2;
		i = 0;
		tmp[1] = 0.005;
		while (i <= 1) {
			glColor4ub(130, 10, 10, 210);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3d(0, tmp[1], 0);
			glColor4ub(210, 200, 1, 210);

			for (double t = 0; t <= 2 * PI + 0.0001; t += PI / 100)
			{
				tmp[0] = r * cos(t);
				tmp[2] = r * sin(t);
				glVertex3dv(tmp);
			}

			glEnd();

			tmp[1] = -0.005;
			i++;
		}
		glPopMatrix();
		glPushMatrix();
		glTranslated(wCircleDown[0], wCircleDown[1], wCircleDown[2]);
		r = 1.7;
		i = 0;
		tmp[1] = 0.005;
		while (i <= 1) {
			glColor4ub(230, 100, 1, 210);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3d(0, tmp[1], 0);
			glColor4ub(130, 10, 10, 210);
			for (double t = 0; t <= 2 * PI + 0.0001; t += PI / 314)
			{
				tmp[0] = r * cos(t);
				tmp[2] = r * sin(t);
				glVertex3dv(tmp);
			}

			glEnd();

			tmp[1] = -0.005;
			i++;
		}
		glPopMatrix();
	}
	glPopMatrix();
	//.

	//����� 2
	glPushMatrix();
	glRotated(12.7, 0, 0, -1);
	glRotatef(71 * PEPEGA.f, 0, 0, -1);
	{
		double wing_Up_h1[] = { -5,0,19 };
		double wing_Up_h2[] = { -12,0,5 };
		double wing_Down_h3[] = { -14.1,0,-3.6 };
		double wing_Down_h4[] = { -4.5,0,-15 };

		double wCircleUp[] = { -5.028,0,6 };
		double wCircleDown[] = { -5.169,0,-4.399 };

		double wingUp[] = { -0.9,0,3.9 };
		double wingFar[] = { -5.7,0,0.3 };
		double wingCenter[] = { -0.9,0,0 };
		double wingDown[] = { -0.9,0,-3.9 };
		glColor4ub(100, 150, 180, 177);
		glNormal3d(0, -1, 0);

		glBegin(GL_QUADS);
		glVertex3d(0, 0, wingUp[2]);
		glVertex3dv(wingUp);
		glVertex3dv(wingDown);
		glVertex3d(0, 0, wingDown[2]);
		glEnd();

		double tmp[3];
		short int R = 223;
		short int G = 115;
		short int B = 255;
		int i = 0;
		glBegin(GL_TRIANGLE_FAN);
		glVertex3dv(wingCenter);
		for (double t = 0; t <= 1.0001; t += 0.01, i++)
		{
			tmp[0] = f(wingUp[0], wing_Up_h1[0], wing_Up_h2[0], wingFar[0], t);
			tmp[1] = f(wingUp[1], wing_Up_h1[1], wing_Up_h2[1], wingFar[1], t);
			tmp[2] = f(wingUp[2], wing_Up_h1[2], wing_Up_h2[2], wingFar[2], t);
			glColor4ub(R, G, B, 177);
			glVertex3dv(tmp);
			R--;
			if (i % 2 == 0)
				G++;
			if (i % 4 == 0)
				B--;
			if (i > 26 && i < 39)
			{
				R--;
				G += 3;
			}
			if (i > 39 && i < 70)
			{
				G -= 2;
				B -= 4;
			}
			if (i > 70 && i < 90)
			{
				B += 2;
				R += 3;
			}
			if (i > 90)
			{
				B -= 9;
				R -= 6;
				G -= 3;
			}
		}

		for (double t = 0; t <= 1.0001; t += 0.01, i--)
		{
			tmp[0] = f(wingFar[0], wing_Down_h3[0], wing_Down_h4[0], wingDown[0], t);
			tmp[1] = f(wingFar[1], wing_Down_h3[1], wing_Down_h4[1], wingDown[1], t);
			tmp[2] = f(wingFar[2], wing_Down_h3[2], wing_Down_h4[2], wingDown[2], t);
			glColor4ub(R, G, B, 177);
			glVertex3dv(tmp);

			if (i % 2 == 0)
				G++;
			if (i % 4 == 0)
				B--;
			if (i > 26 && i < 39)
			{
				R--;
				G += 3;
			}
			if (i > 39 && i < 70)
			{
				G -= 2;
				B -= 1;
			}
			if (i > 70 && i < 90)
			{
				B += 2;
				R += 3;
			}
			if (i > 90)
			{
				B -= 3;
				R -= 2;
				G -= 1;
			}
		}
		glEnd();

		glPushMatrix();
		glTranslated(wCircleUp[0], wCircleUp[1], wCircleUp[2]);
		r = 1.2;
		i = 0;
		tmp[1] = 0.005;
		while (i <= 1) {
			glColor4ub(130, 10, 10, 210);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3d(0, tmp[1], 0);
			glColor4ub(210, 200, 1, 210);

			for (double t = 0; t <= 2 * PI + 0.0001; t += PI / 100)
			{
				tmp[0] = r * cos(t);
				tmp[2] = r * sin(t);
				glVertex3dv(tmp);
			}

			glEnd();

			tmp[1] = -0.005;
			i++;
		}
		glPopMatrix();
		glPushMatrix();
		glTranslated(wCircleDown[0], wCircleDown[1], wCircleDown[2]);
		r = 1.7;
		i = 0;
		tmp[1] = 0.005;
		while (i <= 1) {
			glColor4ub(230, 100, 1, 210);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3d(0, tmp[1], 0);
			glColor4ub(130, 10, 10, 210);
			for (double t = 0; t <= 2 * PI + 0.0001; t += PI / 314)
			{
				tmp[0] = r * cos(t);
				tmp[2] = r * sin(t);
				glVertex3dv(tmp);
			}

			glEnd();

			tmp[1] = -0.005;
			i++;
		}
		glPopMatrix();

	}
	glPopMatrix();
	//.
	if (!ispaused)
		++PEPEGA;

	//��������� ������ ������


	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
									//(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(360, 300);
	rec.setPosition(10, ogl->getHeight() - 300 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	ss << std::endl << std::endl << "����������� ��������:" << std::endl;
	ss << "Y - �������" << std::endl;
	ss << "H - ���������" << std::endl;
	ss << "������ - �����" << std::endl;
	ss << std::endl << "��������� ��������:" << std::endl << "�������� = " << PEPEGA.h << (ispaused ? ", ��������������." : ", �������.") << std::endl;
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}