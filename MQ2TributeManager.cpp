// MQ2TributeManager.cpp : Defines the entry point for the DLL application.
//

// MQ2TributeManager
// Manages tribute for you based on combat status
// Author: alt228, alt228@nerdshack.com
// 
// Now Saves/Loads previous settings. Still defaults to manual - wired420
// Added option to turn tribute on for names, then back off when no named
//   and timer is close to out. - wired420

#include "../MQ2Plugin.h"

enum TributeMode
{
	TributeMode_Manual,
	TributeMode_Auto,
	TributeMode_OffWhenExpired,
	TributeMode_Named,
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

// Save & Load
char szTemp[MAX_STRING], ourMode[MAX_STRING];
bool initDone = false;

//defaults
TributeMode mode = TributeMode_Manual;
unsigned int tributeFudge = 2000;
#define SKIP_PULSES 80
long SkipPulse = 0;

void updateINIFn() {
	sprintf_s(INIFileName, "%s\\%s_%s.ini", gszINIPath, EQADDR_SERVERNAME, GetCharInfo()->Name);
}

VOID SaveINI(VOID) {
	updateINIFn();
	sprintf_s(szTemp, "MQ2TributeManager");
	WritePrivateProfileSection(szTemp, "", INIFileName);
	if (mode == TributeMode_Named) {
		WritePrivateProfileString(szTemp, "Mode", "3", INIFileName);
	}
	else if (mode == TributeMode_Auto) {
		WritePrivateProfileString(szTemp, "Mode", "2", INIFileName);
	}
	else if (mode == TributeMode_OffWhenExpired) {
		WritePrivateProfileString(szTemp, "Mode", "1", INIFileName);
	}
	else {
		WritePrivateProfileString(szTemp, "Mode", "0", INIFileName);
	}
}

VOID LoadINI(VOID) {
	updateINIFn();
	sprintf_s(szTemp, "MQ2TributeManager");
	DWORD loadMode = GetPrivateProfileString(szTemp, "Mode", "", ourMode, MAX_STRING, INIFileName);
	int setTheMode = atoi(ourMode);
	if (setTheMode == 0) {
		mode = TributeMode_Manual;
	}
	else if (setTheMode == 1) {
		mode = TributeMode_OffWhenExpired;
	}
	else if (setTheMode == 2) {
		mode = TributeMode_Auto;
	}
	else if (setTheMode == 3) {
		mode = TributeMode_Named;
	}
	initDone = true;
}

VOID SetTributeStatus(bool tributeOn)
{
	if (tributeOn)
	{
		if (!(bool)((*pTributeActive) ? true : false))
		{
			DebugSpewAlways("MQ2TributeManager::Turning on tribute");
			DoCommand((PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWP_ActivateButton leftmouseup");
			DoCommand((PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWT_ActivateButton leftmouseup");
		}
	}
	else
	{
		if ((bool)((*pTributeActive) ? true : false))
		{
			DebugSpewAlways("MQ2TributeManager::Turning off tribute");
			DoCommand((PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWP_ActivateButton leftmouseup");
			DoCommand((PSPAWNINFO)pCharSpawn, "/notify TributeBenefitWnd TBWT_ActivateButton leftmouseup");
		}
	}
}

VOID TributeManagerCmd(PSPAWNINFO characterSpawn, PCHAR line)
{
	bool syntaxError = false;
	if (line[0] == 0)
	{
		syntaxError = true;
	}

	bool setMode = false;
	TributeMode newMode = TributeMode_Unused;

	bool setStatus = false;
	bool newStatus = true;

	bool showStatus = false;

	CHAR thisArg[MAX_STRING] = { 0 };
	int argNumber = 1;
	bool moreArgs = true;

	while (moreArgs && argNumber < 10)
	{
		DebugSpewAlways("MQ2TributeManager:: GetArg(%i)", argNumber);
		GetArg(thisArg, line, argNumber);
		argNumber++;

		if (!thisArg || (strlen(thisArg) == 0))
		{
			moreArgs = false;
		}
		else if (_stricmp(thisArg, "on") == 0)
		{
			setStatus = true;
			newStatus = true;
		}
		else if (_stricmp(thisArg, "off") == 0)
		{
			setMode = true;
			newMode = TributeMode_OffWhenExpired;
			WriteChatColor("Tribute mode: off when expired");
		}
		else if (_stricmp(thisArg, "forceoff") == 0)
		{
			setStatus = true;
			newStatus = false;
			setMode = true;
			newMode = TributeMode_Manual;
		}
		else if (_stricmp(thisArg, "auto") == 0)
		{
			setMode = true;
			newMode = TributeMode_Auto;
			WriteChatColor("Tribute mode: automatic");
		}
		else if (_stricmp(thisArg, "named") == 0)
		{
			setMode = true;
			newMode = TributeMode_Named;
			WriteChatColor("Tribute mode: named");
		}
		else if (_stricmp(thisArg, "manual") == 0)
		{
			setMode = true;
			newMode = TributeMode_Manual;
			WriteChatColor("Tribute mode: manual");
		}
		else if (_stricmp(thisArg, "show") == 0)
		{
			showStatus = true;
		}
		else
		{
			syntaxError = true;
		}
	}

	if (syntaxError)
	{
		SyntaxError("Usage: /tribute <auto|named|manual|on|off|forceoff|show>");
		return;
	}

	if (setStatus)
	{
		SetTributeStatus(newStatus);
	}

	if (setMode)
	{
		mode = newMode;
		SaveINI();
	}

	if (showStatus)
	{
		if (mode == TributeMode_Manual)
		{
			WriteChatColor("Tribute mode: manual");
		}
		else if (mode == TributeMode_Auto)
		{
			WriteChatColor("Tribute mode: automatic");
		}
		else if (mode == TributeMode_Named)
		{
			WriteChatColor("Tribute mode: Named");
		}
		else if (mode == TributeMode_OffWhenExpired)
		{
			WriteChatColor("Tribute mode: off when expired");
		}

		if (gGameState == GAMESTATE_INGAME)
		{
			DebugSpewAlways("MQ2TributeManager:: Active Favor Cost: %i", pEQMisc->GetActiveFavorCost());
			DebugSpewAlways("MQ2TributeManager:: Combat State: %i", (CombatState)((PCPLAYERWND)pPlayerWnd)->CombatState);
			DebugSpewAlways("MQ2TributeManager:: Tribute Active: %i", *pTributeActive);
			DebugSpewAlways("MQ2TributeManager:: Current Favor: %i", GetCharInfo()->CurrFavor);
			DebugSpewAlways("MQ2TributeManager:: Tribute Timer: %i ms", GetCharInfo()->TributeTimer);
		}
	}
}
// Are we in combat? We used this check enough that it was time to quit replicating code - wired420
bool inCombat(void) {
	CombatState combatState = (CombatState)((PCPLAYERWND)pPlayerWnd)->CombatState;
	if (combatState == CombatState_COMBAT)
	{
		return true;
	}
	return false;
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2TributeManager");
	AddCommand("/tribute", TributeManagerCmd);
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
	if (GameState == GAMESTATE_INGAME)
	{
		if (!initDone) {
			LoadINI();
		}
		// have to do a little hack here or else other /notify commands will not work
		DebugSpewAlways("MQ2TributeManager::SetGameState::initializing tribute window");
		DoCommand((PSPAWNINFO)pCharSpawn, "/keypress TOGGLE_TRIBUTEBENEFITWIN");
		DoCommand((PSPAWNINFO)pCharSpawn, "/keypress TOGGLE_TRIBUTEBENEFITWIN");
	}
	if (GameState != GAMESTATE_INGAME && GameState != GAMESTATE_LOGGINGIN) {
		if (initDone) {
			initDone = false;
		}
	}
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	if (gGameState != GAMESTATE_INGAME)
		return;
	if (!initDone)
		return;

	if (SkipPulse == SKIP_PULSES) {
		SkipPulse = 0;

		if (mode == TributeMode_Named)
		{
			unsigned int activeFavorCost = pEQMisc->GetActiveFavorCost();
			PCHARINFO myCharInfo = GetCharInfo();

			if ((inCombat() && !*pTributeActive) && (ppTarget && IsNamed(PSPAWNINFO(pTarget))) && (activeFavorCost <= myCharInfo->CurrFavor) && (activeFavorCost > 0))
			{
				SetTributeStatus(true);
			}
			else if (*pTributeActive && (!inCombat() || (inCombat() && ppTarget && !IsNamed(PSPAWNINFO(pTarget)))) && (myCharInfo->TributeTimer < tributeFudge))
			{
				SetTributeStatus(false);
			}
		}

		if (mode == TributeMode_Auto)
		{
			unsigned int activeFavorCost = pEQMisc->GetActiveFavorCost();
			PCHARINFO myCharInfo = GetCharInfo();

			if ((inCombat()) && (!*pTributeActive) && (activeFavorCost <= myCharInfo->CurrFavor) && (activeFavorCost > 0))
			{
				//activate tribute
				SetTributeStatus(true);
			}
			else if ((!inCombat()) && (*pTributeActive) && (myCharInfo->TributeTimer < tributeFudge))
			{
				SetTributeStatus(false);
			}
		}
		else if (mode == TributeMode_OffWhenExpired)
		{
			if (((*pTributeActive) ? true : false) == false)
			{
				mode = TributeMode_Manual;
				return;
			}

			if (GetCharInfo()->TributeTimer < tributeFudge)
			{
				mode = TributeMode_Manual;
				SetTributeStatus(false);
			}
		}
	}
	SkipPulse++;
}