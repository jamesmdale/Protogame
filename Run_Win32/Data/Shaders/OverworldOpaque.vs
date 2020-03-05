#version 420 core

//uniform
uniform mat4 PROJECTION;
uniform mat4 VIEW; 
uniform mat4 MODEL;

//in
in vec4 COLOR; 
in vec3 POSITION;     
in vec2 UV;   

//out
out vec4 passColor;
out vec3 passWorldPosition;
out vec2 passUV; 

void main() 
{
   vec4 local_pos = vec4( POSITION, 1 ); 
   vec4 clip_pos = PROJECTION * VIEW * MODEL * local_pos; 

   passColor = COLOR; // pass it on. 
   passUV = UV;
   passWorldPosition = POSITION;
   gl_Position = clip_pos; 
}