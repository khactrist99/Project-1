#version 150

in vec3 position;
in vec4 color;
in vec3 positionLeft, positionRight, positionUp, positionDown;
out vec4 col;


uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform int mode;
float eps = 0.00001f, smoothH;


void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
    if(mode == 1) {
        smoothH = (positionLeft.y+positionRight.y+positionUp.y+positionDown.y)/4;
        col = max(color, vec4(eps))/max(position.y,eps) * smoothH;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position.x,smoothH,position.z, 1.0f);
    } else if(mode == 0) {
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
        col = color;
    } 
  
}

