#include "Game\GameCommon.hpp"


// globals ----------------------------------------------
bool g_isDebug = false;
bool g_isQuitting = false;

// physics globals ----------------------------------------------
Vector3 g_gravity = Vector3(0.f, 0.f, -20.f);
float g_groundFrictionAmount = 7.5f;
float g_airFrictionAmount = 0.1f;

// player globals ----------------------------------------------
float g_playerWidth = 0.6f;
float g_playerHeight = 1.8f;
float g_playerEyesOffset = 1.65f;
float g_playerWalkSpeed = 5.f;
float g_playerWalkAcceleration = 160.f;
float g_playerFlySpeed = 8.f;
float g_jumpStrength = 7.5f;
//float g_playerRunSpeed = 7.5f;

//lighting values ----------------------------------------------
Rgba g_minLightColor = Rgba(0.15f, 0.15f, 0.15f, 1.f);
Rgba g_maxLightColor = Rgba(1.f, 1.f, 1.f, 1.f);
Rgba g_lightBlue = Rgba(126, 174, 252, 255);
Rgba g_darkBlue = Rgba(20, 40, 40, 255);
Rgba g_white = Rgba(1.f, 1.f, 1.f, 1.f);

Rgba g_defaultIndoorLightColor = Rgba(1.0f, 0.9f, 0.8f, 1.f);
Rgba g_defaultOutdoorLightColor = Rgba(0.8f, 0.9f, 1.0f, 1.0f);
Rgba g_minOutdoorLightColor = Rgba(0.05f, 0.05f, 0.075f, 1.0f);
Rgba g_dayLightColor = g_lightBlue;
Rgba g_nightLightColor = g_darkBlue;

// block data ----------------------------------------------
BlockLocator g_invalidBlockLocator;

//convenience directions ----------------------------------------------
Vector3 g_worldUp = Vector3(0.f, 0.f, 1.f);
Vector3 g_worldForward = Vector3(1.f, 0.f, 0.f);
Vector3 g_worldRight = Vector3(0.f, -1.f, 0.f);

Vector3 g_east = Vector3(1.f, 0.f, 0.f);
Vector3 g_west = Vector3(-1.f, 0.f, 0.f);
Vector3 g_north = Vector3(0.f, 1.f, 0.f);
Vector3 g_south = Vector3(0.f, -1.f, 0.f);

IntVector2 g_chunkNorth = IntVector2(0,1);
IntVector2 g_chunkSouth = IntVector2(0,-1);
IntVector2 g_chunkEast = IntVector2(1,0);
IntVector2 g_chunkWest = IntVector2(-1,0);

//convenience matricies ----------------------------------------------
Matrix44 g_flipMatrix = Matrix44(Vector4(0.f, 0.f, 1.f, 0.f),
								Vector4(-1.f, 0.f, 0.f, 0.f),
								Vector4(0.f, 1.f, 0.f, 0.f),
								Vector4(0.f, 0.f, 0.f, 1.f));

int eventCountTest = 0;