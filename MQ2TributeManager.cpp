// MQ2TributeManager.cpp : Defines the entry point for the DLL application.
//

// MQ2TributeManager
// Manages tribute for you based on combat status
// Author: alt228, alt228@nerdshack.com

#include "../MQ2Plugin.h"

enum TributeMode
{
	TributeMode_Manual,
	TributeMode_Auto,
	TributeMode_OffWhenExpired,
	TributeMode_Unused
};

enum CombatState
{
	CombatState_COMBAT,
	CombatState_DEBUFFED,
	CombatState_COOLDOWN,
	CombatState_ACTIVE,
	CombatState_RESTING
};

PreSetup("MQ2TributeManager");

//defaults
TributeMode mode = TributeMode_Manual;
unsigned int tributeFudge = 2000;
#define SKIP_PULSES 80
long SkipPulse = 0;

VOID SetTributeStatus(bool tributeOn)
{
	if( tributeOn )
	{
		if( !(bool)((*pTributeActive) ? true : false))
		{
			DebugSpewAlways("MQ2TributeManager::Turning on tribute");
			DoCommand( (PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWP_ActivateButton leftmouseup" );
			DoCommand( (PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWT_ActivateButton leftmouseup" );
		}
	}
	else
	{
		if( (bool)((*pTributeActive) ? true : false))
		{
			DebugSpewAlways("MQ2TributeManager::Turning off tribute");
			DoCommand( (PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWP_ActivateButton leftmouseup" );
			DoCommand( (PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWT_ActivateButton leftmouseup" );
		}
	}
}

VOID TributeManagerCmd(PSPAWNINFO characterSpawn, PCHAR line)
{
	bool syntaxError = false;
	if (line[0]==0)
	{
		syntaxError = true;
	}
	
	bool setMode = false;
	TributeMode newMode = TributeMode_Unused;

	bool setStatus = false;
	bool newStatus = true;

	bool showStatus = false;

    CHAR thisArg[MAX_STRING] = {0};
	int argNumber = 1;
	bool moreArgs = true;

	while(moreArgs && argNumber<10 )
	{
		DebugSpewAlways("MQ2TributeManager:: GetArg(%i)", argNumber);
		GetArg(thisArg,line,argNumber);
		argNumber++;
	
		if( !thisArg || (strlen(thisArg)==0) )
		{
			moreArgs = false;
		}
		else if(_stricmp(thisArg,"on") == 0)
		{
			setStatus = true;
			newStatus = true;
		}
		else if(_stricmp(thisArg,"off") == 0)
		{
			setMode = true;
			newMode = TributeMode_OffWhenExpired;
			WriteChatColor("Tribute mode: off when expired");
		}
		else if(_stricmp(thisArg,"forceoff") == 0)
		{
			setStatus = true;
			newStatus = false;
			setMode = true;
			newMode = TributeMode_Manual;
		}
		else if(_stricmp(thisArg,"auto") == 0)
		{
			setMode = true;
			newMode = TributeMode_Auto;
			WriteChatColor("Tribute mode: automatic");
		}
		else if(_stricmp(thisArg,"manual") == 0)
		{
			setMode = true;
			newMode = TributeMode_Manual;
			WriteChatColor("Tribute mode: manual");
		}
		else if(_stricmp(thisArg,"show") == 0)
		{
			showStatus = true;
		}
		else
		{
			syntaxError = true;
		}
	}

	if( syntaxError )
	{
		SyntaxError("Usage: /tribute <auto|manual|on|off|forceoff|show>");
		return;
	}

	if( setStatus )
	{
		SetTributeStatus(newStatus);
	}

	if( setMode )
	{
		mode = newMode;
	}

	if( showStatus )
	{
		if( mode == TributeMode_Manual )
		{
			WriteChatColor("Tribute mode: manual");
		}
		else if( mode == TributeMode_Auto )
		{
			WriteChatColor("Tribute mode: automatic");
		}
		else if( mode == TributeMode_OffWhenExpired )
		{
			WriteChatColor("Tribute mode: off when expired");
		}

		if( gGameState == GAMESTATE_INGAME )
		{
			DebugSpewAlways("MQ2TributeManager:: Active Favor Cost: %i", pEQMisc->GetActiveFavorCost());
			DebugSpewAlways("MQ2TributeManager:: Combat State: %i", (CombatState)((PCPLAYERWND)pPlayerWnd)->CombatState);
			DebugSpewAlways("MQ2TributeManager:: Tribute Active: %i", *pTributeActive);
			DebugSpewAlways("MQ2TributeManager:: Current Favor: %i", GetCharInfo()->CurrFavor);
			DebugSpewAlways("MQ2TributeManager:: Tribute Timer: %i ms", GetCharInfo()->TributeTimer);
		}
	}
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2TributeManager");
	AddCommand("/tribute",TributeManagerCmd);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2TributeManager");
	RemoveCommand("/tribute");
}

PLUGIN_API VOID SetGameState(DWORD GameState)
{
    DebugSpewAlways("MQ2TributeManager::SetGameState()");
    if (GameState==GAMESTATE_INGAME)
	{
		// have to do a little hack here or else other /notify commands will not work
		DebugSpewAlways("MQ2TributeManager::SetGameState::initializing tribute window");
		DoCommand((PSPAWNINFO)pCharSpawn, "/keypress TOGGLE_TRIBUTEBENEFITWIN");
		DoCommand((PSPAWNINFO)pCharSpawn, "/keypress TOGGLE_TRIBUTEBENEFITWIN");
	}
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	if( gGameState != GAMESTATE_INGAME )
	{
		return;
	}

    if (SkipPulse == SKIP_PULSES) {
        SkipPulse = 0;

		if( mode == TributeMode_Auto )
		{
			CombatState combatState = (CombatState)((PCPLAYERWND)pPlayerWnd)->CombatState;
			bool inCombat = false;
			if( combatState == CombatState_COMBAT )
			{
				inCombat = true;
			}
	
			unsigned int activeFavorCost = pEQMisc->GetActiveFavorCost();
			PCHARINFO myCharInfo = GetCharInfo();
	
			if( (inCombat) && (!*pTributeActive) && (activeFavorCost <= myCharInfo->CurrFavor) && (pEQMisc->GetActiveFavorCost() > 0) )
			{
				//activate tribute
				SetTributeStatus(true);
			}
			else if( (!inCombat) && (*pTributeActive) && (myCharInfo->TributeTimer < tributeFudge) )
			{
				SetTributeStatus(false);
			}
		}
		else if( mode == TributeMode_OffWhenExpired )
		{
			if( ((*pTributeActive) ? true : false) == false )
			{
				mode = TributeMode_Manual;
				return;
			}
	
			if( GetCharInfo()->TributeTimer < tributeFudge )
			{
				mode = TributeMode_Manual;
				SetTributeStatus(false);
			}
		}
	}
	SkipPulse++;
}
