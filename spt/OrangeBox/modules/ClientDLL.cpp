#include "stdafx.hpp"

#include "..\cvars.hpp"
#include "..\modules.hpp"
#include "..\patterns.hpp"
#include "..\..\strafestuff.hpp"
#include "..\..\sptlib-wrapper.hpp"
#include <SPTLib\memutils.hpp>
#include <SPTLib\detoursutils.hpp>
#include <SPTLib\hooks.hpp>
#include "ClientDLL.hpp"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <algorithm>

using std::uintptr_t;
using std::size_t;

void __cdecl ClientDLL::HOOKED_DoImageSpaceMotionBlur(void* view, int x, int y, int w, int h)
{
	return clientDLL.HOOKED_DoImageSpaceMotionBlur_Func(view, x, y, w, h);
}

bool __fastcall ClientDLL::HOOKED_CheckJumpButton(void* thisptr, int edx)
{
	return clientDLL.HOOKED_CheckJumpButton_Func(thisptr, edx);
}

void __stdcall ClientDLL::HOOKED_HudUpdate(bool bActive)
{
	return clientDLL.HOOKED_HudUpdate_Func(bActive);
}

int __fastcall ClientDLL::HOOKED_GetButtonBits(void* thisptr, int edx, int bResetState)
{
	return clientDLL.HOOKED_GetButtonBits_Func(thisptr, edx, bResetState);
}

void __fastcall ClientDLL::HOOKED_AdjustAngles(void* thisptr, int edx, float frametime)
{
	return clientDLL.HOOKED_AdjustAngles_Func(thisptr, edx, frametime);
}

void __fastcall ClientDLL::HOOKED_CreateMove(void* thisptr, int edx, int sequence_number, float input_sample_frametime, bool active)
{
	return clientDLL.HOOKED_CreateMove_Func(thisptr, edx, sequence_number, input_sample_frametime, active);
}

void __fastcall ClientDLL::HOOKED_CViewRender__OnRenderStart(void* thisptr, int edx)
{
	return clientDLL.HOOKED_CViewRender__OnRenderStart_Func(thisptr, edx);
}

void ClientDLL::Hook(const std::wstring& moduleName, HMODULE hModule, uintptr_t moduleStart, size_t moduleLength)
{
	Clear(); // Just in case.

	this->hModule = hModule;
	this->moduleStart = moduleStart;
	this->moduleLength = moduleLength;
	this->moduleName = moduleName;

	MemUtils::ptnvec_size ptnNumber;

	uintptr_t pHudUpdate,
		pGetButtonBits,
		pAdjustAngles,
		pCreateMove,
		pCViewRender__OnRenderStart,
		pMiddleOfCAM_Think,
		pGetGroundEntity,
		pCalcAbsoluteVelocity;

	auto fHudUpdate = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsHudUpdate, &pHudUpdate);
	auto fGetButtonBits = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsGetButtonBits, &pGetButtonBits);
	auto fAdjustAngles = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsAdjustAngles, &pAdjustAngles);
	auto fCreateMove = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsCreateMove, &pCreateMove);
	auto fCViewRender__OnRenderStart = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsCViewRender__OnRenderStart, &pCViewRender__OnRenderStart);
	auto fMiddleOfCAM_Think = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsMiddleOfCAM_Think, &pMiddleOfCAM_Think);
	auto fGetGroundEntity = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsGetGroundEntity, &pGetGroundEntity);
	auto fCalcAbsoluteVelocity = std::async(std::launch::async, MemUtils::FindUniqueSequence, moduleStart, moduleLength, Patterns::ptnsCalcAbsoluteVelocity, &pCalcAbsoluteVelocity);

	// DoImageSpaceMotionBlur
	uintptr_t pDoImageSpaceMotionBlur = NULL;
	ptnNumber = MemUtils::FindUniqueSequence(moduleStart, moduleLength, Patterns::ptnsDoImageSpaceMotionBlur, &pDoImageSpaceMotionBlur);
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_DoImageSpaceMotionBlur = (_DoImageSpaceMotionBlur)pDoImageSpaceMotionBlur;
		EngineDevMsg("[client dll] Found DoImageSpaceMotionBlur at %p (using the build %s pattern).\n", pDoImageSpaceMotionBlur, Patterns::ptnsDoImageSpaceMotionBlur[ptnNumber].build.c_str());

		switch (ptnNumber)
		{
		case 0:
			pgpGlobals = *(uintptr_t **)(pDoImageSpaceMotionBlur + 132);
			break;

		case 1:
			pgpGlobals = *(uintptr_t **)(pDoImageSpaceMotionBlur + 153);
			break;

		case 2:
			pgpGlobals = *(uintptr_t **)(pDoImageSpaceMotionBlur + 129);
			break;

		case 3:
			pgpGlobals = *(uintptr_t **)(pDoImageSpaceMotionBlur + 171);
			break;

		case 4:
			pgpGlobals = *(uintptr_t **)(pDoImageSpaceMotionBlur + 177);
			break;
			
		case 5:
			pgpGlobals = *(uintptr_t **)(pDoImageSpaceMotionBlur + 128);
			break;
		}

		EngineDevMsg("[client dll] pgpGlobals is %p.\n", pgpGlobals);
	}
	else
	{
		EngineDevWarning("[client dll] Could not find DoImageSpaceMotionBlur!\n");
		EngineWarning("y_spt_motion_blur_fix has no effect.\n");
	}

	//Client-side CheckJumpButton
	//Only for client prediction, when we're playing on a server.

	uintptr_t pCheckJumpButton = NULL;
	ptnNumber = MemUtils::FindUniqueSequence(moduleStart, moduleLength, Patterns::ptnsClientCheckJumpButton, &pCheckJumpButton);
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_CheckJumpButton = (_CheckJumpButton)pCheckJumpButton;
		EngineDevMsg("[client dll] Found CheckJumpButton at %p (using the build %s pattern).\n", pCheckJumpButton, Patterns::ptnsClientCheckJumpButton[ptnNumber].build.c_str());

		switch (ptnNumber)
		{
		case 0:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
			break;
		case 1:
			off1M_nOldButtons = 1;
			off2M_nOldButtons = 40;
			break;

		case 2:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
			break;

		case 3:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
			break;

		case 4:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
			break;

		case 5:
			off1M_nOldButtons = 1;
			off2M_nOldButtons = 40;
			break;

		case 6:
			off1M_nOldButtons = 1;
			off2M_nOldButtons = 40;
			break;

		case 7:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
			break;

		case 8:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
			break;
			
		case 9:
			off1M_nOldButtons = 1;
			off2M_nOldButtons = 40;
			break;
			
		case 10:
			off1M_nOldButtons = 2;
			off2M_nOldButtons = 40;
		}
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CheckJumpButton!\n");
		EngineWarning("y_spt_autojump has no effect in multiplayer.\n");
	}

	// HudUpdate
	ptnNumber = fHudUpdate.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_HudUpdate = (_HudUpdate)pHudUpdate;
		EngineDevMsg("[client dll] Found CHLClient::HudUpdate at %p (using the build %s pattern).\n", pHudUpdate, Patterns::ptnsHudUpdate[ptnNumber].build.c_str());
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CHLClient::HudUpdate.\n");
		EngineWarning("_y_spt_afterframes has no effect.\n");
	}

	// GetButtonBits
	ptnNumber = fGetButtonBits.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_GetButtonBits = (_GetButtonBits)pGetButtonBits;
		EngineDevMsg("[client dll] Found CInput::GetButtonBits at %p (using the build %s pattern).\n", pGetButtonBits, Patterns::ptnsGetButtonBits[ptnNumber].build.c_str());
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CInput::GetButtonBits.\n");
		EngineWarning("+y_spt_duckspam has no effect.\n");
	}

	// AdjustAngles
	ptnNumber = fAdjustAngles.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_AdjustAngles = (_AdjustAngles)pAdjustAngles;
		EngineDevMsg("[client dll] Found CInput::AdjustAngles at %p (using the build %s pattern).\n", pAdjustAngles, Patterns::ptnsAdjustAngles[ptnNumber].build.c_str());
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CInput::AdjustAngles.\n");
	}

	// CreateMove
	ptnNumber = fCreateMove.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_CreateMove = (_CreateMove)pCreateMove;
		EngineDevMsg("[client dll] Found CInput::CreateMove at %p (using the build %s pattern).\n", pCreateMove, Patterns::ptnsCreateMove[ptnNumber].build.c_str());

		switch (ptnNumber) {
		case 0:
			offM_pCommands = 180;
			offForwardmove = 24;
			offSidemove = 28;
			break;

		case 1:
			offM_pCommands = 196;
			offForwardmove = 24;
			offSidemove = 28;
			break;
			
		case 2:
			offM_pCommands = 196;
			offForwardmove = 24;
			offSidemove = 28;
			break;
			
		case 3:
			offM_pCommands = 196;
			offForwardmove = 24;
			offSidemove = 28;
			break;
			
		case 4:
			offM_pCommands = 196;
			offForwardmove = 24;
			offSidemove = 28;
			break;
		}
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CInput::CreateMove.\n");
	}

	if (!ORIG_AdjustAngles || !ORIG_CreateMove)
	{
		EngineWarning("_y_spt_setangles has no effect.\n");
		EngineWarning("_y_spt_setpitch and _y_spt_setyaw have no effect.\n");
		EngineWarning("_y_spt_pitchspeed and _y_spt_yawspeed have no effect.\n");
	}

	// GetGroundEntity
	ptnNumber = fGetGroundEntity.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		GetGroundEntity = (_GetGroundEntity)pGetGroundEntity;
		EngineDevMsg("[client dll] Found GetGroundEntity at %p (using the build %s pattern).\n", pGetGroundEntity, Patterns::ptnsGetGroundEntity[ptnNumber].build.c_str());

		switch (ptnNumber) {
		case 0:
			offMaxspeed = 4136;
			offFlags = 736;
			offAbsVelocity = 248;
			offDucking = 3545;
			offDuckJumpTime = 3552;
			offServerSurfaceFriction = 3812;
			offServerPreviouslyPredictedOrigin = 3692;
			offServerAbsOrigin = 580;
			break;

		case 1:
			offMaxspeed = 4076;
			offFlags = 732;
			offAbsVelocity = 244;
			offDucking = 3489;
			offDuckJumpTime = 3496;
			offServerSurfaceFriction = 3752;
			offServerPreviouslyPredictedOrigin = 3628;
			offServerAbsOrigin = 580;
			break;
			
		case 2:
			offMaxspeed = 4312;
			offFlags = 844;
			offAbsVelocity = 300;
			offDucking = 3713;
			offDuckJumpTime = 3720;
			offServerSurfaceFriction = 3872;
			offServerPreviouslyPredictedOrigin = 3752;
			offServerAbsOrigin = 636;
			break;
			
		case 3:
			offMaxspeed = 4320;
			offFlags = 844;
			offAbsVelocity = 300;
			offDucking = 3721;
			offDuckJumpTime = 3728;
			offServerSurfaceFriction = 3872;
			offServerPreviouslyPredictedOrigin = 3752;
			offServerAbsOrigin = 636;
			break;
		}
	} else
	{
		EngineDevWarning("[client dll] Could not find GetGroundEntity.\n");
	}

	// CalcAbsoluteVelocity
	ptnNumber = fCalcAbsoluteVelocity.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		CalcAbsoluteVelocity = (_CalcAbsoluteVelocity)pCalcAbsoluteVelocity;
		EngineDevMsg("[client dll] Found CalcAbsoluteVelocity at %p (using the build %s pattern).\n", pCalcAbsoluteVelocity, Patterns::ptnsCalcAbsoluteVelocity[ptnNumber].build.c_str());
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CalcAbsoluteVelocity.\n");
	}

	// Middle of CAM_Think
	ptnNumber = fMiddleOfCAM_Think.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		switch (ptnNumber) {
		case 0:
			GetLocalPlayer = (_GetLocalPlayer)(*reinterpret_cast<uintptr_t*>(pMiddleOfCAM_Think + 29) + pMiddleOfCAM_Think + 33);
			break;

		case 1:
			GetLocalPlayer = (_GetLocalPlayer)(*reinterpret_cast<uintptr_t*>(pMiddleOfCAM_Think + 30) + pMiddleOfCAM_Think + 34);
			break;
			
		case 2:
			GetLocalPlayer = (_GetLocalPlayer)(*reinterpret_cast<uintptr_t*>(pMiddleOfCAM_Think + 21) + pMiddleOfCAM_Think + 25);
			break;
			
		case 3:
			GetLocalPlayer = (_GetLocalPlayer)(*reinterpret_cast<uintptr_t*>(pMiddleOfCAM_Think + 23) + pMiddleOfCAM_Think + 27);
			break;
		}

		EngineDevMsg("[client dll] Found the GetLocalPlayer pattern at %p (using the build %s pattern).\n", pMiddleOfCAM_Think, Patterns::ptnsMiddleOfCAM_Think[ptnNumber].build.c_str());
		EngineDevMsg("[client dll] Found GetLocalPlayer at %p.\n", GetLocalPlayer);
	}
	else
	{
		EngineDevWarning("[client dll] Could not find the GetLocalPlayer pattern.\n");
	}

	extern bool FoundEngineServer();
	if (ORIG_CreateMove
		&& GetGroundEntity
		&& CalcAbsoluteVelocity
		&& GetLocalPlayer
		&& _sv_airaccelerate
		&& _sv_accelerate
		&& _sv_friction
		&& _sv_maxspeed
		&& _sv_stopspeed
		&& FoundEngineServer())
	{
		tasAddressesWereFound = true;
	}
	else
	{
		EngineWarning("The full game TAS solutions are not available.\n");
	}

	// CViewRender::OnRenderStart
	ptnNumber = fCViewRender__OnRenderStart.get();
	if (ptnNumber != MemUtils::INVALID_SEQUENCE_INDEX)
	{
		ORIG_CViewRender__OnRenderStart = (_CViewRender__OnRenderStart)pCViewRender__OnRenderStart;
		EngineDevMsg("[client dll] Found CViewRender::OnRenderStart at %p (using the build %s pattern).\n", pCViewRender__OnRenderStart, Patterns::ptnsCViewRender__OnRenderStart[ptnNumber].build.c_str());
	}
	else
	{
		EngineDevWarning("[client dll] Could not find CViewRender::OnRenderStart.\n");
		EngineWarning("_y_spt_force_90fov has no effect.\n");
	}

	DetoursUtils::AttachDetours(moduleName, {
		{ (PVOID *) (&ORIG_DoImageSpaceMotionBlur), HOOKED_DoImageSpaceMotionBlur },
		{ (PVOID *) (&ORIG_CheckJumpButton), HOOKED_CheckJumpButton },
		{ (PVOID *) (&ORIG_HudUpdate), HOOKED_HudUpdate },
		{ (PVOID *) (&ORIG_GetButtonBits), HOOKED_GetButtonBits },
		{ (PVOID *)(&ORIG_AdjustAngles), HOOKED_AdjustAngles },
		{ (PVOID *)(&ORIG_CreateMove), HOOKED_CreateMove },
		{ (PVOID *)(&ORIG_CViewRender__OnRenderStart), HOOKED_CViewRender__OnRenderStart }
	});
}

void ClientDLL::Unhook()
{
	DetoursUtils::DetachDetours(moduleName, {
		{ (PVOID *)(&ORIG_DoImageSpaceMotionBlur), HOOKED_DoImageSpaceMotionBlur },
		{ (PVOID *) (&ORIG_CheckJumpButton), HOOKED_CheckJumpButton },
		{ (PVOID *)(&ORIG_HudUpdate), HOOKED_HudUpdate },
		{ (PVOID *)(&ORIG_GetButtonBits), HOOKED_GetButtonBits },
		{ (PVOID *)(&ORIG_AdjustAngles), HOOKED_AdjustAngles },
		{ (PVOID *)(&ORIG_CreateMove), HOOKED_CreateMove },
		{ (PVOID *)(&ORIG_CViewRender__OnRenderStart), HOOKED_CViewRender__OnRenderStart }
	});

	Clear();
}

void ClientDLL::Clear()
{
	IHookableNameFilter::Clear();
	ORIG_DoImageSpaceMotionBlur = nullptr;
	ORIG_CheckJumpButton = nullptr;
	ORIG_HudUpdate = nullptr;
	ORIG_GetButtonBits = nullptr;
	ORIG_AdjustAngles = nullptr;
	ORIG_CreateMove = nullptr;
	GetLocalPlayer = nullptr;
	GetGroundEntity = nullptr;
	CalcAbsoluteVelocity = nullptr;

	pgpGlobals = nullptr;
	off1M_nOldButtons = 0;
	off2M_nOldButtons = 0;
	offM_pCommands = 0;
	offForwardmove = 0;
	offSidemove = 0;
	offMaxspeed = 0;
	offFlags = 0;
	offAbsVelocity = 0;
	offDucking = 0;
	offDuckJumpTime = 0;
	offServerSurfaceFriction = 0;
	offServerPreviouslyPredictedOrigin = 0;
	offServerAbsOrigin = 0;
	pCmd = 0;

	tasAddressesWereFound = false;

	afterframesQueue.clear();
	afterframesPaused = false;

	duckspam = false;

	setPitch.set = false;
	setYaw.set = false;
	forceJump = false;
	cantJumpNextTime = false;
}

void ClientDLL::DelayAfterframesQueue(int delay)
{
	afterframesDelay = delay;
}

void ClientDLL::AddIntoAfterframesQueue(const afterframes_entry_t& entry)
{
	afterframesQueue.push_back(entry);
}

void ClientDLL::ResetAfterframesQueue()
{
	afterframesQueue.clear();
}

void ClientDLL::OnFrame()
{
	if (afterframesPaused)
	{
		return;
	}

	if (afterframesDelay-- > 0)
	{
		return;
	}

	for (auto it = afterframesQueue.begin(); it != afterframesQueue.end(); )
	{
		it->framesLeft--;
		if (it->framesLeft <= 0)
		{
			EngineConCmd(it->command.c_str());
			it = afterframesQueue.erase(it);
		}
		else
			++it;
	}

	if (engineDLL.Demo_IsPlayingBack() && !engineDLL.Demo_IsPlaybackPaused())
	{
		auto tick = y_spt_pause_demo_on_tick.GetInt();
		if (tick != 0)
		{
			if (tick < 0)
				tick += engineDLL.Demo_GetTotalTicks();

			if (tick == engineDLL.Demo_GetPlaybackTick())
				EngineConCmd("demo_pause");
		}
	}
}

void __cdecl ClientDLL::HOOKED_DoImageSpaceMotionBlur_Func(void* view, int x, int y, int w, int h)
{
	uintptr_t origgpGlobals = NULL;

	/*
	Replace gpGlobals with (gpGlobals + 12). gpGlobals->realtime is the first variable,
	so it is located at gpGlobals. (gpGlobals + 12) is gpGlobals->curtime. This
	function does not use anything apart from gpGlobals->realtime from gpGlobals,
	so we can do such a replace to make it use gpGlobals->curtime instead without
	breaking anything else.
	*/
	if (pgpGlobals)
	{
		if (y_spt_motion_blur_fix.GetBool())
		{
			origgpGlobals = *pgpGlobals;
			*pgpGlobals = *pgpGlobals + 12;
		}
	}

	ORIG_DoImageSpaceMotionBlur(view, x, y, w, h);

	if (pgpGlobals)
	{
		if (y_spt_motion_blur_fix.GetBool())
		{
			*pgpGlobals = origgpGlobals;
		}
	}
}

bool __fastcall ClientDLL::HOOKED_CheckJumpButton_Func(void* thisptr, int edx)
{
	/*
	Not sure if this gets called at all from the client dll, but
	I will just hook it in exactly the same way as the server one.
	*/
	const int IN_JUMP = (1 << 1);

	int *pM_nOldButtons = NULL;
	int origM_nOldButtons = 0;

	if (y_spt_autojump.GetBool())
	{
		pM_nOldButtons = (int *)(*((uintptr_t *)thisptr + off1M_nOldButtons) + off2M_nOldButtons);
		origM_nOldButtons = *pM_nOldButtons;

		if (!cantJumpNextTime) // Do not do anything if we jumped on the previous tick.
		{
			*pM_nOldButtons &= ~IN_JUMP; // Reset the jump button state as if it wasn't pressed.
		}
		else
		{
			// EngineDevMsg( "Con jump prevented!\n" );
		}
	}

	cantJumpNextTime = false;

	bool rv = ORIG_CheckJumpButton(thisptr, edx); // This function can only change the jump bit.

	if (y_spt_autojump.GetBool())
	{
		if (!(*pM_nOldButtons & IN_JUMP)) // CheckJumpButton didn't change anything (we didn't jump).
		{
			*pM_nOldButtons = origM_nOldButtons; // Restore the old jump button state.
		}
	}

	if (rv)
	{
		// We jumped.
		if (_y_spt_autojump_ensure_legit.GetBool())
		{
			cantJumpNextTime = true; // Prevent consecutive jumps.
		}

		// EngineDevMsg( "Jump!\n" );
	}

	EngineDevMsg("Engine call: [client dll] CheckJumpButton() => %s\n", (rv ? "true" : "false"));

	return rv;
}

void __stdcall ClientDLL::HOOKED_HudUpdate_Func(bool bActive)
{
	OnFrame();

	return ORIG_HudUpdate(bActive);
}

int __fastcall ClientDLL::HOOKED_GetButtonBits_Func(void* thisptr, int edx, int bResetState)
{
	int rv = ORIG_GetButtonBits(thisptr, edx, bResetState);

	if (bResetState == 1)
	{
		static bool duckPressed = false;
		
		if (duckspam)
		{
			if (duckPressed)
				duckPressed = false;
			else
			{
				duckPressed = true;
				rv |= (1 << 2); // IN_DUCK
			}
		}
		else
			duckPressed = false;

		if (forceJump) {
			forceJump = false;
			rv |= (1 << 1); // IN_JUMP
		}
	}

	return rv;
}

void __fastcall ClientDLL::HOOKED_AdjustAngles_Func(void* thisptr, int edx, float frametime)
{
	ORIG_AdjustAngles(thisptr, edx, frametime);

	if (!pCmd)
		return;

	float va[3];
	EngineGetViewAngles(va);

	double pitchSpeed = atof(_y_spt_pitchspeed.GetString()),
		yawSpeed = atof(_y_spt_yawspeed.GetString());

	if (pitchSpeed != 0.0f)
		va[PITCH] += pitchSpeed;
	if (setPitch.set)
	{
		setPitch.set = false;
		va[PITCH] = setPitch.angle;
	}

	if (yawSpeed != 0.0f)
		va[YAW] += yawSpeed;
	if (setYaw.set)
	{
		setYaw.set = false;
		va[YAW] = setYaw.angle;
	}
	else if (tasAddressesWereFound && yawSpeed == 0.0f && tas_strafe.GetBool())
	{
		auto player = GetLocalPlayer();
		auto onground = (GetGroundEntity(player, 0) != NULL); // TODO: This should really be a proper check.
		auto maxspeed = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + offMaxspeed);

		if (tas_force_onground.GetBool())
			onground = true;

		auto vars = MovementVars();
		vars.Accelerate = _sv_accelerate->GetFloat();

		if (tas_force_airaccelerate.GetString()[0] != '\0')
			vars.Airaccelerate = tas_force_airaccelerate.GetFloat();
		else
			vars.Airaccelerate = _sv_airaccelerate->GetFloat();

		vars.EntFriction = 1;
		vars.Frametime = 0.015f;
		vars.Friction = _sv_friction->GetFloat();
		vars.Maxspeed = (maxspeed > 0) ? std::min(maxspeed, _sv_maxspeed->GetFloat()) : _sv_maxspeed->GetFloat();
		vars.Stopspeed = _sv_stopspeed->GetFloat();

		if (tas_force_wishspeed_cap.GetString()[0] != '\0')
			vars.WishspeedCap = tas_force_wishspeed_cap.GetFloat();
		else
			vars.WishspeedCap = 30;

		// Lgagst requires more prediction that is done here for correct operation.
		auto curState = CurrentState();
		curState.LgagstMinSpeed = tas_strafe_lgagst_minspeed.GetFloat();
		curState.LgagstFullMaxspeed = tas_strafe_lgagst_fullmaxspeed.GetBool();

		auto pl = PlayerData();
		CalcAbsoluteVelocity(player, 0);
		float *vel = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + offAbsVelocity);
		pl.Velocity = Vector(vel[0], vel[1], vel[2]);

		//DevMsg("[Strafing] speed pre-friction = %.8f\n", pl.Velocity.Length2D());
		//DevMsg("[Strafing] velocity pre-friction: %.8f %.8f %.8f\n", pl.Velocity.x, pl.Velocity.y, pl.Velocity.z);
		//EngineConCmd("getpos");

		bool reduceWishspeed = onground && ((*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(player) + offFlags)) & FL_DUCKING);
		bool jumped = false;
		const int IN_JUMP = (1 << 1);

		extern IServerUnknown* GetServerPlayer();
		auto server_player = GetServerPlayer();

		auto previouslyPredictedOrigin = reinterpret_cast<Vector*>(reinterpret_cast<uintptr_t>(server_player) + offServerPreviouslyPredictedOrigin);
		auto absOrigin = reinterpret_cast<Vector*>(reinterpret_cast<uintptr_t>(server_player) + offServerAbsOrigin);
		bool gameCodeMovedPlayer = (*previouslyPredictedOrigin != *absOrigin);

		vars.EntFriction = *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(server_player) + offServerSurfaceFriction);

		if (gameCodeMovedPlayer) {
			if (tas_reset_surface_friction.GetBool()) {
				vars.EntFriction = 1.0f;
			}

			if (pl.Velocity.z <= 140.f) {
				if (onground) {
					// TODO: This should check the real surface friction.
					vars.EntFriction = 1.0f;
				} else if (pl.Velocity.z > 0.f) {
					vars.EntFriction = 0.25f;
				}
			}
		}

		//DevMsg("[Strafing] EntFriction = %f\n", vars.EntFriction);

		auto btns = StrafeButtons();
		bool usingButtons = (sscanf(tas_strafe_buttons.GetString(), "%hhu %hhu %hhu %hhu", &btns.AirLeft, &btns.AirRight, &btns.GroundLeft, &btns.GroundRight) == 4);
		auto type = static_cast<StrafeType>(tas_strafe_type.GetInt());
		auto dir = static_cast<StrafeDir>(tas_strafe_dir.GetInt());
		
		ProcessedFrame out;
		out.Jump = false;
		{
			bool cantjump = false;
			// This will report air on the first frame of unducking and report ground on the last one.
			if ((*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(player) + offDucking)) && ((*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(player) + offFlags)) & FL_DUCKING)) {
				cantjump = true;
			}

			auto djt = (*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + offDuckJumpTime));
			djt -= vars.Frametime * 1000;
			djt = std::max(0.f, djt);
			float flDuckMilliseconds = std::max(0.0f, 1000.f - djt);
			float flDuckSeconds = flDuckMilliseconds * 0.001f;
			if (flDuckSeconds > 0.2) {
				djt = 0;
			}
			if (djt > 0) {
				cantjump = true;
			}

			if (!cantjump && onground) {
				if (tas_strafe_lgagst.GetBool()) {
					LgagstJump(pl, vars, curState, onground, ((*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(player) + offFlags)) & FL_DUCKING), type, dir, tas_strafe_yaw.GetFloat(), va[YAW] * M_DEG2RAD, out, reduceWishspeed, btns, false);
					if (out.Jump) {
						onground = false;
						jumped = true;
					}
				}

				if (ORIG_GetButtonBits(thisptr, 0, 0) & IN_JUMP) {
					onground = false;
					jumped = true;
				}
			}
		}

		Friction(pl, onground, vars);
		Strafe(pl, vars, onground, jumped, ((*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(player) + offFlags)) & FL_DUCKING), type, dir, tas_strafe_yaw.GetFloat(), va[YAW], out, reduceWishspeed, btns, usingButtons);

		//EngineDevMsg("[Strafing] Yaw = %.8f\n", out.Yaw);

		va[YAW] = out.Yaw;
		if (out.Forward) {
			*reinterpret_cast<float*>(pCmd + offForwardmove) += vars.Maxspeed;
		}
		if (out.Back) {
			*reinterpret_cast<float*>(pCmd + offForwardmove) -= vars.Maxspeed;
		}
		if (out.Right) {
			*reinterpret_cast<float*>(pCmd + offSidemove) += vars.Maxspeed;
		}
		if (out.Left) {
			*reinterpret_cast<float*>(pCmd + offSidemove) -= vars.Maxspeed;
		}
		if (out.Jump) {
			forceJump = true;
		}
	}

	EngineSetViewAngles(va);
}

void __fastcall ClientDLL::HOOKED_CreateMove_Func(void* thisptr, int edx, int sequence_number, float input_sample_frametime, bool active)
{
	auto m_pCommands = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(thisptr) + offM_pCommands);
	pCmd = m_pCommands + 84 * (sequence_number % 90);

	ORIG_CreateMove(thisptr, edx, sequence_number, input_sample_frametime, active);

	pCmd = 0;
}

void __fastcall ClientDLL::HOOKED_CViewRender__OnRenderStart_Func(void* thisptr, int edx)
{
	ORIG_CViewRender__OnRenderStart(thisptr, edx);

	if (!_viewmodel_fov || !_y_spt_force_90fov.GetBool())
		return;

	float *fov = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(thisptr) + 52);
	float *fovViewmodel = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(thisptr) + 56);
	*fov = 90;
	*fovViewmodel = _viewmodel_fov->GetFloat();
}
