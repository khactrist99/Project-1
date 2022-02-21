/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: <type your USC username here>
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>
#include <vector> //mine

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
float scale = 0.2; // my global variable

int windowWidth = 1280;
int windowHeight = 720;
int width, heightOfImage;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

GLuint triVertexBuffer, triColorVertexBuffer;
GLuint triVertexArray;
GLuint vVertexBuffer, vColorVertexBuffer; //mine
GLuint vVertexArray;//mine
int sizeTri;

OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void displayFunc()
{
  // render some stuff...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  matrix.LookAt((width*heightOfImage)/2, scale*3, 0, (width*heightOfImage)/2, 0, (width*heightOfImage)/2, 0, 0, -1);

  float m[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(m);

  float p[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(p);
  //
  // bind shader
  pipelineProgram->Bind();

  // set variable
  pipelineProgram->SetModelViewMatrix(m);
  pipelineProgram->SetProjectionMatrix(p);

  glBindVertexArray(vVertexArray);
  glDrawArrays(GL_POINTS, 0, width*heightOfImage*sizeof(float));

  glutSwapBuffers();
}

void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)
    // saveScreenshot("temp.jpg");
  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(60.0f, (float)w / (float)h, 0.01f, 100.0f);
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);//me
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;
  }
}

void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  cout << argv[1] << endl;
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
 
  // modify the following code accordingly
  vector<float> position, color;
  vector<glm::vec3> vecs;
  vector<glm::vec4> clrs;
  width = heightmapImage->getWidth();
  heightOfImage = heightmapImage->getHeight();
  cout << sizeof(heightmapImage->getPixels()) << " width:" << width << " height: " << heightOfImage;

  for(unsigned int i = 0; i < width; i++) {
      for(unsigned int j =0; j < heightOfImage; j++) {
        //   cout << scale *heightmapImage->getPixel(i,j,0) << endl;
          float currHeight = (scale * (float)heightmapImage->getPixel(i,j,0))/255.0;
        //   cout << heightmapImage->getPixel(i,j,0)/255.0 << endl;
          //one vertex
        //   cout << " J: " << j << " -j: " << j*-1.0;
          position.push_back((float)i);
          position.push_back((float)currHeight);
          position.push_back((float)j*-1.0);
          vecs.push_back(glm::vec3(i, currHeight, j*-1));
        //   cout << vecs.size() << endl;

          //color
          color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0);
          color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0);
          color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0);
          color.push_back(1.0f);
          clrs.push_back(glm::vec4(heightmapImage->getPixel(i,j,0)/255.0,(heightmapImage->getPixel(i,j,0)/255.0),heightmapImage->getPixel(i,j,0)/255.0,1));

      }
  }
  cout << endl <<sizeof(glm::vec3) << " " << position.size() << " " << sizeof(float)<< endl;
//   glm::vec3 triangle[3] = {
//     glm::vec3(0, 0, 0), 
//     glm::vec3(0, 1, 0),
//     glm::vec3(1, 0, 0)
//   };

//   glm::vec4 color[3] = {
//     {0, 0, 1, 1},
//     {1, 0, 0, 1},
//     {0, 1, 0, 1},
//   };

//   glGenBuffers(1, &triVertexBuffer);
//   glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, triangle,
//                GL_STATIC_DRAW);

//mine
glGenBuffers(1, &vVertexBuffer);
glBindBuffer(GL_ARRAY_BUFFER, vVertexBuffer);
glBufferData(GL_ARRAY_BUFFER, sizeof(position[0]) * position.size(), &position[0], GL_STATIC_DRAW);
// glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vecs.size(), &vecs[0],            GL_STATIC_DRAW);

//   glGenBuffers(1, &triColorVertexBuffer);
//   glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * 3, color, GL_STATIC_DRAW);
cout << endl << "dffs";
//mine
  glGenBuffers(1, &vColorVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vColorVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(color[0]) * color.size(), &color[0], GL_STATIC_DRAW);
// glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * width*heightOfImage*3, &clrs, GL_STATIC_DRAW);



  pipelineProgram = new BasicPipelineProgram;
  int ret = pipelineProgram->Init(shaderBasePath);
  if (ret != 0) abort();

//   glGenVertexArrays(1, &triVertexArray);
//   glBindVertexArray(triVertexArray);
//   glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);

//my vao
    glGenVertexArrays(1, &vVertexArray);
    glBindVertexArray(vVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vVertexBuffer);

    GLuint loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, vColorVertexBuffer);
//   glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
//   glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glEnable(GL_DEPTH_TEST);

  sizeTri = 3;

  std::cout << "GL error: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


