/**
����Ҫʵ�ֵĶ���:
�򵥵�OpenGL����ʹ��glfw��ʵ��
*/
#define _CRT_SECURE_NO_WARNINGS
#define  GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib,"glew32s.lib")

#pragma pack(push, 1)
struct BitmapFileHeader
{
	unsigned short type;          //   λͼ�ļ������ͣ�����ΪBM
	unsigned long size;           //   λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ
	unsigned short reserved1;     //   λͼ�ļ������֣�����Ϊ0
	unsigned short reserved2;     //   λͼ�ļ������֣�����Ϊ0
	unsigned long offbits;        //   λͼ���ݵ���ʼλ�ã��������λͼ
};

struct BitmapInfoHeader
{
	unsigned long size;          //   ���ṹ��ռ���ֽ���
	unsigned long width;         //   λͼ�Ŀ�ȣ�������Ϊ��λ
	unsigned long height;        //   λͼ�ĸ߶ȣ�������Ϊ��λ
	unsigned short planes;       //   Ŀ���豸�ļ��𣬱���Ϊ1
	unsigned short bitcount;     //   ÿ�����������λ����������1(˫ɫ),4(16ɫ)��8(256ɫ)��24(���ɫ)֮һ
	unsigned long compression;   //   λͼѹ�����ͣ�������   0(��ѹ��),1(BI_RLE8ѹ������)��2(BI_RLE4ѹ������)֮һ
	unsigned long sizeimage;     //   λͼ�Ĵ�С�����ֽ�Ϊ��λ
	unsigned long xpelspermeter; //   λͼˮƽ�ֱ��ʣ�ÿ��������
	unsigned long ypelspermeter; //   λͼ��ֱ�ֱ��ʣ�ÿ��������
	unsigned long clrused;       //   λͼʵ��ʹ�õ���ɫ���е���ɫ��
	unsigned long clrimportant;  //   λͼ��ʾ��������Ҫ����ɫ��
};

struct RGB { unsigned char red, green, blue; };
#pragma pack(pop)
// bmp �洢���ݵ�������bgr, opengl����ʹ����ɫ˳����rgb,������Ҫת��һ��
inline void SwapBGR2RGB(RGB& rgb)
{
	unsigned char t = rgb.red;
	rgb.red = rgb.blue;
	rgb.blue = t;
}

unsigned int LoadBitmap24(const char* filename)
{
	unsigned int tex = 0;
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		return 0;

	BitmapFileHeader fileheader;
	BitmapInfoHeader infohead;

	fread(&fileheader, sizeof(BitmapFileHeader), 1, fp);
	fread(&infohead, sizeof(BitmapInfoHeader), 1, fp);

	int width = infohead.width;
	int height = infohead.height;
	int bitcount = infohead.bitcount;

	if (bitcount == 24)
	{
		int num = width * height;
		fseek(fp, fileheader.offbits, SEEK_SET);

		RGB* img = new RGB[num];
		fread(img, sizeof(RGB) * num, 1, fp);

		for (int i = 0; i < num; ++i)
			SwapBGR2RGB(img[i]);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Min Filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Mag Filter
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &img->red);

		delete[] img;
	}

	fclose(fp);
	return tex;
}

inline GLchar* SampleReadFileData(const char* filename, GLsizei &sz)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return NULL;
	fseek(fp, 0L, SEEK_END);
	sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	GLchar* data = new GLchar[sz];
	sz = fread((void*)data, 1, sz, fp);
	fclose(fp);
	return data;
}

GLuint  SampleBuildShader(const char* filename, GLenum type)
{
	GLsizei sz = 0;
	GLint succ;
	const GLchar* vsSource = SampleReadFileData(filename, sz);
	GLuint  vs = glCreateShader(type);
	glShaderSource(vs, 1, &vsSource, &sz);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &succ);
	if (succ == GL_FALSE)
	{
		GLchar err[256] = { 0 };
		GLsizei err_sz, len;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &err_sz);
		glGetShaderInfoLog(vs, err_sz, &len, err);
		printf("%s\n", err);
	}

	return vs;
}

GLuint SampleBuildProgram(const char*vsfn, const char* frfn)
{
	GLint linked;
	GLuint vsshader = SampleBuildShader(vsfn, GL_VERTEX_SHADER);
	GLuint frshader = SampleBuildShader(frfn, GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();
	glAttachShader(program, vsshader);
	glAttachShader(program, frshader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLchar err[256] = { 0 };
		GLsizei err_sz, len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &err_sz);
		glGetProgramInfoLog(program, err_sz, &len, err);
		printf("%s\n", err);
	}
	glUseProgram(program);
	return program;
}

static void error_callback(int error, const char* description) { fputs(description, stderr); fputs("\n", stderr); }
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char *argv[])
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	glfwInit();
	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	//glfwWindowHint(GLFW_SAMPLES, 0);    // 0x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // ����OPENGL�汾3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwSwapInterval(1);
	glfwSetKeyCallback(window, key_callback);

	GLuint tex = LoadBitmap24("Data/demo.bmp");     // ����һ��bmp����
	GLuint program = SampleBuildProgram("Data/shader.vert", "Data/shader.frag"); // ʹ��shader, ��r<->b������ɫ
	GLint t1 = glGetUniformLocation(program, "tex");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(t1, 0);;//��Ӧ�����һ��


	glClearColor(0, 0, 0, 1);      // ������ĻΪ��ɫ
	glEnable(GL_CULL_FACE);     // �������޳�
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);    // ������Ȳ���

	glEnable(GL_LIGHT0);        // ���õƹ�0
	glEnable(GL_NORMALIZE);     // ���÷���
	glEnable(GL_COLOR_MATERIAL);// ���ò���ģʽ
	glEnable(GL_LIGHTING);      // �򿪵ƹ�

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.9);//0.5���Ի����κ���0~1֮�����
	glShadeModel(GL_SMOOTH);

	// ���õƹ����ɫ
	const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);  // ���û�������ɫ
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);  // �������������ɫ
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);  // ���þ��淴�����ɫ
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);  // ���õƵ�λ��

														// ���ò��ʵ���ɫ
	const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat high_shininess[] = { 100.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);   // ���û�������ɫ
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);   // �������������ɫ
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);  // ���þ��淴�����ɫ
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);// ����ָ�� ��ֵԽС����ʾ����Խ�ֲڣ����Դ����Ĺ������䵽���棬Ҳ���Բ����ϴ������

	const GLfloat vers[] = {
		-0.5,-0.5, 0.5,  0.5,-0.5, 0.5,  0.5,0.5, 0.5, -0.5,0.5, 0.5,   // ���� 4������
		0.5,-0.5,-0.5, -0.5,-0.5,-0.5, -0.5,0.5,-0.5,  0.5,0.5,-0.5,   // ���� 4������

		-0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,-0.5, -0.5, 0.5,-0.5,   // ���� 4������
		-0.5,-0.5,-0.5, 0.5,-0.5,-0.5, 0.5,-0.5, 0.5, -0.5,-0.5, 0.5,   // ���� 4������

		-0.5,-0.5,-0.5, -0.5,-0.5, 0.5, -0.5,0.5, 0.5, -0.5,0.5,-0.5,   // ���� 4������
		0.5,-0.5, 0.5,  0.5,-0.5,-0.5,  0.5,0.5,-0.5,  0.5,0.5, 0.5,   // ���� 4������
	};

	const GLfloat cors[] = {
		1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f,
	};

	const GLfloat nors[] = {
		0,0,1, 0,0,1, 0,0,1, 0,0,1,
		0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1,
		0,1,0, 0,1,0, 0,1,0, 0,1,0,
		0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0,
		-1,0,0, -1,0,0, -1,0,0, -1,0,0,
	};

	const GLfloat uvs[] = {
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
	};

	// �������������˳����,ǰ��,����,����
	const GLint indices[] = {
		0,1,2, 0,2,3,
		4,5,6, 4,6,7,

		8,9,10, 8,10,11,
		12,13,14, 12,14,15,

		16,17,18, 16,18,19,
		20,21,22, 20,22,23,
	};

	float rx = 0.0f;
	float ry = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		float ratio = (float)width / (float)height;
		glViewport(0, 0, width, height);                // ��ͼ����ָ����,����ʾ����Ļ��
		glMatrixMode(GL_PROJECTION);                    // ѡ��͸�Ӿ���
		glLoadIdentity();                               // ���þ���
		glFrustum(-ratio, ratio, -1.0, 1.0, 2.0, 100.0);// ����͸�Ӿ���
		glMatrixMode(GL_MODELVIEW);                     // ѡ��ģ�;���
		glLoadIdentity();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ������ɫ����Ȼ���
															//glTranslatef(0.0f ,0.0f, -3.0f);
		gluLookAt(0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0);

		glRotatef(rx, 1.0f, 0.0f, 0.0f);
		glRotatef(ry, 0.0f, 1.0f, 0.0f);

		rx = rx > 360 ? 0 : (rx + 0.1f);
		ry = ry > 360 ? 0 : (ry + 0.1f);

		glBindTexture(GL_TEXTURE_2D, tex);      // ������
		glPushMatrix();
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vers);

		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 0, cors);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, nors);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, uvs);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
		glPopMatrix();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
