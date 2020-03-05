#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;
uniform vec3 CAMERA_WORLD_POSITION;
uniform vec3 GLOBAL_INDOOR_LIGHT_COLOR;
uniform vec3 GLOBAL_OUTDOOR_LIGHT_COLOR;
uniform vec3 SKY_COLOR;
uniform vec2 NEAR_FAR_FOG_DISTANCE;

in vec4 passColor; // NEW, passed color
in vec3 passWorldPosition;
in vec2 passUV; 

out vec4 outColor; 
  
void main() 
{
	//color diffuse accordint to highest light value (indoor vs outdoor)
   vec4 diffuse = texture( gTexDiffuse, passUV );
   float indoorLightValue = passColor.r;
   float outdoorLightValue = passColor.g;

   vec3 indoorLightColor = GLOBAL_INDOOR_LIGHT_COLOR * indoorLightValue;
   vec3 outdoorLightColor = GLOBAL_OUTDOOR_LIGHT_COLOR * outdoorLightValue;

   vec3 highestLightColor = max(indoorLightColor, outdoorLightColor);
  
   vec4 modifiedDiffuse = diffuse * vec4(highestLightColor, passColor.a);

   //fog
   float cameraToTexel = distance(CAMERA_WORLD_POSITION, passWorldPosition);
   float fogNormalized = (cameraToTexel - NEAR_FAR_FOG_DISTANCE.x) / (NEAR_FAR_FOG_DISTANCE.y - NEAR_FAR_FOG_DISTANCE.x); //normalized fog
   fogNormalized = clamp(fogNormalized, 0.0, 1.0);

   vec3 outColorRGB = mix(modifiedDiffuse.rgb, SKY_COLOR, fogNormalized); 
   //outColorRGB = mix( SKY_COLOR, NEAR_FAR_FOG_DISTANCE.x * GLOBAL_OUTDOOR_LIGHT_COLOR * GLOBAL_INDOOR_LIGHT_COLOR * CAMERA_WORLD_POSITION, 0.f );
   outColor = vec4(outColorRGB, 1.0); 
}
