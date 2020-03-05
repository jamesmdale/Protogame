#include "Game\TheApp.hpp"
#include "Game\Game.hpp"
#include "Game\GameCommon.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Time\Time.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\Command.hpp"
#include "Engine\Core\ErrorWarningAssert.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\DevConsole.hpp"
#include "Engine\Debug\DebugRender.hpp"
#include "Engine\File\ObjectFileLoader.hpp"
#include "Engine\Net\NetAddress.hpp"
#include "Engine\Net\TCPSocket.hpp"
#include "Engine\Core\BytePacker.hpp"
#include "Engine\Net\RemoteCommandService.hpp"
#include "Engine\Net\NetSession.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\Profiler\ProfilerConsole.hpp"
#include "Engine\Core\NamedProperties.hpp"
#include "Engine\Events\EventSystem.hpp"

TheApp* g_theApp = nullptr;
static bool isRunning = false;


//  =============================================================================
TheApp::TheApp()
{ 
	Game::CreateInstance();
}


//  =============================================================================
TheApp::~TheApp()
{

}


//  =============================================================================
void TheApp::RunFrame()
{
	Renderer::GetInstance()->BeginFrame();	
	InputSystem::GetInstance()->BeginFrame();
	MasterClockBeginFrame();

	Update();
	PreRender();
	Render();
	PostRender();

	DebugRender::GetInstance()->EndFrame();
	InputSystem::GetInstance()->EndFrame();
	Renderer::GetInstance()->EndFrame();

	Sleep(1);
	TODO("Need to add sleep function to release CPU cycles and reduce system demand");
}


//  =============================================================================
void TheApp::Initialize()
{
	//register app commands
	RegisterCommand("quit", CommandRegistration(Quit, ": Use to quit the program", "Quitting..."));

	//start the masterclock
	Clock* masterClock = GetMasterClock();
	masterClock->ClockSystemStartup();
	masterClock = nullptr;	

	//init mouse input settings
	InputSystem::GetInstance()->GetMouse()->MouseLockToScreen(true);
	InputSystem::GetInstance()->GetMouse()->MouseShowCursor(true);
	InputSystem::GetInstance()->GetMouse()->SetMouseMode(MOUSE_RELATIVE_MODE);	

	Game::GetInstance()->Initialize();


	NamedProperties p;
	p.Set("FirstName", "Squirrel"); 	// Setting as c-string (const char*) data...	

	//float health = p.Get("Height", 1.75f);
	EventSystem* theEventSystem = EventSystem::GetInstance();
	theEventSystem->Subscribe("testEvent", TestFunction); //will add testEvent
	theEventSystem->Subscribe("testEvent", TestFunction2); //will Add testfunction 2
	theEventSystem->Subscribe("testEvent", TestFunction); //will not add second test event

	theEventSystem->Subscribe("testEvent2", TestFunction2); //will add test event 2

	theEventSystem->FireEvent("testEvent", p);
	theEventSystem->FireEvent("testEvent2");

	//object method test
	TestObject* object = new TestObject();
	theEventSystem->Subscribe("testMethodEvent", object, &TestObject::TestCallbackMethod); //will add test event 2
	theEventSystem->FireEvent("testMethodEvent");
}

//  =============================================================================
void TheApp::Update() 
{
	float deltaSeconds = GetMasterDeltaSeconds();
	deltaSeconds = UpdateInput(deltaSeconds);

	NetSession::GetInstance()->Update(deltaSeconds);
	
	if(DebugRender::GetInstance()->IsEnabled())
	{
		DebugRender::GetInstance()->Update(deltaSeconds);
	}

	if (ProfilerConsole::GetInstance()->IsOpen())
	{
		ProfilerConsole::GetInstance()->UpdateFromInput();
		ProfilerConsole::GetInstance()->Update();
	}

	if(DevConsole::GetInstance()->IsOpen())
	{
		DevConsole::GetInstance()->Update(deltaSeconds);
		//Game::GetInstance()->TestReliableSend();
	}
	else
	{
		Game::GetInstance()->Update();
	}
}

//  =============================================================================
void TheApp::PreRender()
{
	Game::GetInstance()->PreRender();
}

//  =============================================================================
void TheApp::Render()
{
	//set up screen
	Game::GetInstance()->Render();

	if(DebugRender::GetInstance()->IsEnabled())
	{
		DebugRender::GetInstance()->Render();
	}

	if(DevConsole::GetInstance()->IsOpen())
	{
		DevConsole::GetInstance()->Render();
	}	
}

//  =============================================================================
void TheApp::PostRender()
{
	Game::GetInstance()->PostRender();
}

//  =============================================================================
float TheApp::UpdateInput(float deltaSeconds)
{
	UNUSED(deltaSeconds); //remove if deltaSeconds needed

	if(InputSystem::GetInstance()->WasKeyJustPressed((InputSystem::GetInstance()->KEYBOARD_TILDE)))
	{
		if(!DevConsole::GetInstance()->IsOpen())
		{
			DevConsole::GetInstance()->Open();
		}
		else
		{
			DevConsole::GetInstance()->Close();
		}		
	}
	
	if (!DevConsole::GetInstance()->IsOpen())
	{
		Game::GetInstance()->UpdateInput(deltaSeconds);
	}

	return deltaSeconds;
}

//  =============================================================================
//	CONSOLE COMMANDS
//  =============================================================================
void Quit(Command& cmd)
{
	DevConsolePrintf(cmd.m_commandInfo->m_successMessage.c_str());
	g_isQuitting = true;
}

bool TestFunction(NamedProperties& args)
{
	std::string firstName = args.Get("FirstName", "Invalid");
	DebuggerPrintf("Hello %s %i \n", firstName.c_str(), eventCountTest);
	++eventCountTest;

	args.Set("FirstName", "Michael");
	return false;
}

bool TestFunction2(NamedProperties& args)
{
	std::string firstName = args.Get("FirstName", "Invalid");
	DebuggerPrintf("Goodbye %s %i \n", firstName.c_str(), eventCountTest);
	++eventCountTest;

	return false;
}




