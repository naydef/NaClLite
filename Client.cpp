#include "SDK.h"
#include "Client.h"
#include "Util.h"
#include "Aimbot.h"
#include "Triggerbot.h"
#include "Misc.h"
#include "RemoveCondExploit.h"
#include "HvH.h"
Vector qLASTTICK;
//============================================================================================
bool __fastcall Hooked_CreateMove(PVOID ClientMode, int edx, float input_sample_frametime, CUserCmd* pCommand)
{
	VMTManager& hook = VMTManager::GetHook(ClientMode); //Get a pointer to the instance of your VMTManager with the function GetHook.
	bool bReturn = hook.GetMethod<bool(__thiscall*)(PVOID, float, CUserCmd*)>(gOffsets.iCreateMoveOffset)(ClientMode, input_sample_frametime, pCommand); //Call the original.
	try
	{
		if (!pCommand->command_number)
			return false;

		CBaseEntity* pLocal = GetBaseEntity(me);

		if (!pLocal)
			return bReturn;

		gMisc.Run(pLocal, pCommand);
		gAim.Run(pLocal, pCommand);
		gHvH.Run(pLocal, pCommand);
		gTrigger.Run(pLocal, pCommand);
		gCond.Run(pLocal, pCommand);
	}
	catch (...)
	{
		Log::Fatal("Failed Hooked_CreateMove");
	}
	qLASTTICK = pCommand->viewangles;
	return false/*bReturn*/;
}

void __fastcall FrameStageNotifyThink(PVOID CHLClient, void *_this, ClientFrameStage_t Stage)
{
	if (Stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (auto i = 1; i <= gInts.Engine->GetMaxClients(); i++)
		{
			CBaseEntity *entity = nullptr;
			player_info_t temp;

			if (!(entity = gInts.EntList->GetClientEntity(i)))
				continue;

			if (entity->IsDormant())
				continue;

			if (!gInts.Engine->GetPlayerInfo(i, &temp))
				continue;

			if (!entity->GetLifeState() == LIFE_ALIVE)
				continue;

			Vector vX = entity->GetAngles();
			Vector vY = entity->GetAnglesHTC();
			auto *m_angEyeAnglesX = reinterpret_cast<float*>(reinterpret_cast<DWORD>(entity) + gNetVars.get_offset("DT_TFPlayer", "tfnonlocaldata", "m_angEyeAngles[0]"));
			auto *m_angEyeAnglesY = reinterpret_cast<float*>(reinterpret_cast<DWORD>(entity) + gNetVars.get_offset("DT_TFPlayer", "tfnonlocaldata", "m_angEyeAngles[1]"));
			if (gCvars.aimbot_resolver)
			{
				if (gCvars.aimbot_resolver_fakeup) //idk why I'm doing it like this but I'm tired af.
				{
					if (vX.x == 90) //This is a sample resolver, do more yourself if you'd like. (Fake up resolver)
					{
						*m_angEyeAnglesX = -89;
					}
				}
			}
		}
	}

	if (gInts.Engine->IsInGame() && Stage == FRAME_RENDER_START)
	{
		if (gCvars.vismisc_active)
		{
			CBaseEntity *oEntity = gInts.EntList->GetClientEntity(gInts.Engine->GetLocalPlayer());

			int *Fov;
			int OldFov;

			int m_flFOVRate = 0xE5C;// Broken: nv.get_offset("DT_BasePlayer", "localdata", "m_flFOVRate");
			int &fovPtr = *(int*)(oEntity + gNetVars.get_offset("DT_BasePlayer", "m_iFOV")), defaultFov = *(int*)(oEntity + gNetVars.get_offset("DT_BasePlayer", "m_iDefaultFOV"));
			if (gCvars.misc_nozoom) //Thanks cademtz/Hold on! for this code, works amazingly aside from the inivisible sniper rifle xd
			{
				fovPtr = defaultFov;
				*(float*)(oEntity + m_flFOVRate) = 0;
			}

			if (gCvars.misc_thirdperson) //shows antiaims just fine
			{
				CBaseEntity *oEntity = gInts.EntList->GetClientEntity(gInts.Engine->GetLocalPlayer());

				auto *m_angEyeAngles = reinterpret_cast<float*>(reinterpret_cast<DWORD>(oEntity) + gNetVars.get_offset("DT_BasePlayer", "pl", "deadflag") + 8);

				auto *HTC = reinterpret_cast<float*>(reinterpret_cast<DWORD>(oEntity) + gNetVars.get_offset("DT_BasePlayer", "pl", "deadflag") + 4);

				*HTC = qLASTTICK.x;
				*m_angEyeAngles = qLASTTICK.y;

				ConVar* sv_cheats = gInts.cvar->FindVar("sv_cheats");
				if (sv_cheats->GetInt() == 0) sv_cheats->SetValue(1);
				ConVar* pThirdCamYaw = gInts.cvar->FindVar("cam_idealyaw");
				gInts.Engine->ClientCmd_Unrestricted("thirdperson");

				pThirdCamYaw->SetValue(0);
			}
			else if (!gCvars.misc_thirdperson || !gCvars.vismisc_active)
			{
				gInts.Engine->ClientCmd_Unrestricted("firstperson");
			}

			bool bSkyNeedsUpdate = true;
			if (gCvars.sky_changer && bSkyNeedsUpdate && gInts.Engine->IsInGame()) //I didn't add this to the menu simply because I'm lazy, this code is (atleast I think) made by plasmafart
			{
				if (gInts.cvar->FindVar("sv_skyname")->GetString() != "sky_night_01")
				{
					typedef bool(_cdecl* LoadNamedSkysFn)(const char*);
					static LoadNamedSkysFn LoadSkys = (LoadNamedSkysFn)gSignatures.GetEngineSignature("55 8B EC 81 EC ? ? ? ? 8B 0D ? ? ? ? 53 56 57 8B 01 C7 45");


					if (gCvars.sky_changer_value == 1)
					{
						LoadSkys("sky_night_01");
						bool bSkyNeedsUpdate = true;
					}

					if (gCvars.sky_changer_value == 2)
					{
						LoadSkys("sky_nightfall_01");
						bool bSkyNeedsUpdate = true;
					}

					if (gCvars.sky_changer_value == 3)
					{
						LoadSkys("sky_harvest_night_01 ");
						bool bSkyNeedsUpdate = true;
					}

					if (gCvars.sky_changer_value == 4)
					{
						LoadSkys("sky_halloween");
						bool bSkyNeedsUpdate = true;
					}


					if (gCvars.sky_changer_value == 5)
					{
						LoadSkys("sky_pyroland_01");
						bool bSkyNeedsUpdate = true;
					}

					if (gCvars.sky_changer_value == 0)
					{
						LoadSkys("go buy lmaobox.net instead of using this shit lmao xd");
						bool bSkyNeedsUpdate = true;
					}
				}
				bSkyNeedsUpdate = false;
			}

			for (auto i = 1; i <= gInts.Engine->GetMaxClients(); i++) //This is big heads/big torso, quite simple and at the same time shitty.
			{
				CBaseEntity* pEntity = gInts.EntList->GetClientEntity(i);
				CBaseEntity *entity = nullptr;
				player_info_t temp;

				if (!(entity = gInts.EntList->GetClientEntity(i)))
					continue;

				if (entity->IsDormant())
					continue;

				if (!gInts.Engine->GetPlayerInfo(i, &temp))
					continue;

				if (!entity->GetLifeState() == LIFE_ALIVE)
					continue;
				if (!(entity = gInts.EntList->GetClientEntity(i)))
					continue;

				if (entity->IsDormant())
					continue;

				if (!gInts.Engine->GetPlayerInfo(i, &temp))
					continue;

				if (!entity->GetLifeState() == LIFE_ALIVE)
					continue;

				if (gCvars.misc_bigheadisbig)
				{
					auto *headsize = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pEntity) + gNetVars.get_offset("DT_TFPlayer", "m_flHeadScale"));
					*headsize = 5.0f;
				}
				else if (!gCvars.misc_bigheadisbig || !gCvars.vismisc_active)
				{
					auto *headsize = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pEntity) + gNetVars.get_offset("DT_TFPlayer", "m_flHeadScale"));
					*headsize = 1.0f;
				}

				if (gCvars.misc_bigtorsoisbig)
				{
					auto *torsosize = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pEntity) + gNetVars.get_offset("DT_TFPlayer", "m_flTorsoScale"));
					*torsosize = 5.0f;
				}
				else if (!gCvars.misc_bigtorsoisbig || !gCvars.vismisc_active)
				{
					auto *torsosize = reinterpret_cast<float*>(reinterpret_cast<DWORD>(pEntity) + gNetVars.get_offset("DT_TFPlayer", "m_flTorsoScale"));
					*torsosize = 1.0f;
				}
			}
		}
	}



	VMTManager &HTCCNKBRKYLC = VMTManager::GetHook(CHLClient);
	return HTCCNKBRKYLC.GetMethod<void(__fastcall *)(PVOID, void *, ClientFrameStage_t)>(35)(CHLClient, _this, Stage);
}
//============================================================================================

typedef void(__thiscall* FrameStageNotify)(void *, ClientFrameStage_t);


int __fastcall Hooked_KeyEvent(PVOID CHLClient, int edx, int eventcode, int keynum, const char *currentBinding)
{
	if (eventcode == 1)
	{
		if (keynum == 72) //insert
		{
			gCheatMenu.bMenuActive = !gCheatMenu.bMenuActive;
		}

		if (gCheatMenu.bMenuActive)
		{
			if (keynum == 88 || keynum == 112) // Up
			{

				if (gCheatMenu.iMenuIndex > 0) gCheatMenu.iMenuIndex--;
				else gCheatMenu.iMenuIndex = gCheatMenu.iMenuItems - 1;
				return 0;

			}
			else if (keynum == 90 || keynum == 113) // Down
			{
				if (gCheatMenu.iMenuIndex < gCheatMenu.iMenuItems - 1) gCheatMenu.iMenuIndex++;
				else gCheatMenu.iMenuIndex = 0;
				return 0;

			}
			else if (keynum == 89 || keynum == 107) // Left
			{
				if (gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value)
				{
					gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value[0] -= gCheatMenu.pMenu[gCheatMenu.iMenuIndex].flStep;
					if (gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value[0] < gCheatMenu.pMenu[gCheatMenu.iMenuIndex].flMin)
						gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value[0] = gCheatMenu.pMenu[gCheatMenu.iMenuIndex].flMax;
				}
				return 0;

			}
			else if (keynum == 91 || keynum == 108) // Right
			{
				if (gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value)
				{
					gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value[0] += gCheatMenu.pMenu[gCheatMenu.iMenuIndex].flStep;
					if (gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value[0] > gCheatMenu.pMenu[gCheatMenu.iMenuIndex].flMax)
						gCheatMenu.pMenu[gCheatMenu.iMenuIndex].value[0] = gCheatMenu.pMenu[gCheatMenu.iMenuIndex].flMin;
				}
				return 0;

			}



		}
	}

	VMTManager &hook = VMTManager::GetHook(CHLClient); // Get a pointer to the instance of your VMTManager with the function GetHook.
	return hook.GetMethod<int(__thiscall *)(PVOID, int, int, const char *)>(gOffsets.iKeyEventOffset)(CHLClient, eventcode, keynum, currentBinding); // Call the original.
}