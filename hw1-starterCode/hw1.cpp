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
int mode = 1;

int windowWidth = 1280;
int windowHeight = 720;
int imgWidth, imgHeight;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

GLuint triVertexBuffer, triColorVertexBuffer;
GLuint triVertexArray;
GLuint pointsVertexBuffer, pointsColorVertexBuffer; //mine
GLuint pointsVertexArray;//mine
GLuint linesVertexBuffer, linesColorVertexBuffer; //mine
GLuint linesVertexArray;//mine
// GLuint pointsVertexBuffer, pointsColorVertexBuffer; //mine
// GLuint pointsVertexArray;//mine
// GLuint pointsVertexBuffer, pointsColorVertexBuffer; //mine
// GLuint pointsVertexArray;//mine
int nargc;
char **nargv;
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
  matrix.LookAt(imgWidth/2.0, 300, -1.0f*(float)(imgHeight/2.0f), imgWidth/2.0, 0, -1.0f*(float)(imgHeight/2.0f), 0, 0, -1);
  matrix.Translate(landTranslate[0],landTranslate[1], landTranslate[2]);
  matrix.Rotate(landRotate[0], 1,0,0);
  matrix.Rotate(landRotate[1], 0,1,0);
  matrix.Rotate(landRotate[2], 0,0,1);// landRotate[0], landRotate[1], landRotate[2]);
  matrix.Scale(landScale[0],landScale[1],landScale[2]);


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

    if(mode == 1) {
        glBindVertexArray(pointsVertexArray);
        glDrawArrays(GL_POINTS, 0, imgWidth*imgHeight);
    } else if(mode == 2) {
        glBindVertexArray(linesVertexArray);
        glDrawArrays(GL_LINES, 0, 2*imgWidth*(imgHeight-1) + 2*(imgWidth-1)*imgHeight);
    }

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
  matrix.Perspective(60.0f, (float)w / (float)h, 0.01f, 1000.0f);
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
    
    case '1':
        mode = 1;
        // initScene(nargc, nargv);
    break;

    case '2':
        mode = 2;
        //initScene(nargc, nargv);
    break;
    
    case '3':
        mode = 3;
        //initScene(nargc, nargv);
    break;
    
    case '4':
        mode = 4;
        //initScene(nargc, nargv);
    break;

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

void addToVector(vector<float>& position, vector<float>& color, int i, int j) {
    position.push_back((float)i);
    position.push_back((float)(scale * (float)heightmapImage->getPixel(i,j,0)));
    position.push_back(-1.0f*(float)j);
    //color
    color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0f);
    color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0f);
    color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0f);
    color.push_back(1.0f);
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
    vector<float> lineVertices, lineColors;
    imgWidth = heightmapImage->getWidth();
    imgHeight = heightmapImage->getHeight();

    //points
    for(int i = 0; i < imgHeight; i++) {
        for(int j =0; j < imgWidth; j++) {
            // //vertex
            // position.push_back((float)i);
            // position.push_back((float)currHeight);
            // position.push_back(-1.0*(float)j);
            // //color
            // color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0);
            // color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0);
            // color.push_back((float)heightmapImage->getPixel(i,j,0)/255.0);
            // color.push_back(1.0f);
            addToVector(position, color, i, j);
        }
    }

    //LINES VERTICES
    for(int i = 0; i < imgHeight; i++) {
        for(int j = 0; j < imgWidth; j++) {
            if(j <imgWidth-1) {
            addToVector(lineVertices, lineColors, j+1, i);
            addToVector(lineVertices, lineColors, j, i);
            }
            if(i <imgHeight-1) {
            addToVector(lineVertices, lineColors, j, i);
            addToVector(lineVertices, lineColors, j, i+1);
            }
        }
    }
    // for(int i = 0; i < imgWidth; i++) {
    //     for(int j = 0; j < imgHeight-1; j++) {
    //         addToVector(lineVertices, lineColors, i+1, j);
    //         addToVector(lineVertices, lineColors, i, j);
    //     }
    // }
    cout << "VERTeX SISZ: " << lineVertices.size() << " " << imgWidth << endl;



    
    //POINTS VBO
    glGenBuffers(1, &pointsVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position[0]) * position.size(), &position[0], GL_STATIC_DRAW);
    //POINTS color buffer
    glGenBuffers(1, &pointsColorVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pointsColorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color[0]) * color.size(), &color[0], GL_STATIC_DRAW);

    //lines VBOS
    glGenBuffers(1, &linesVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, linesVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices[0]) * lineVertices.size(), &lineVertices[0], GL_STATIC_DRAW);
    //lines color buffer
    glGenBuffers(1, &linesColorVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, linesColorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineColors[0]) * lineColors.size(), &lineColors[0], GL_STATIC_DRAW);
  
    pipelineProgram = new BasicPipelineProgram;
    int ret = pipelineProgram->Init(shaderBasePath);
    if (ret != 0) abort();

    //my vao for points
    glGenVertexArrays(1, &pointsVertexArray);
    glBindVertexArray(pointsVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVertexBuffer);
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, pointsColorVertexBuffer);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);

/* 
        LINESSSSSSSSS
*/
    glGenVertexArrays(1, &linesVertexArray);
    glBindVertexArray(linesVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, linesVertexBuffer);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, linesColorVertexBuffer);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
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

  nargc = argc;
  nargv = argv;

  // sink forever into the glut loop
  glutMainLoop();
}


