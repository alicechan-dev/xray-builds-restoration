#include "stdafx.h"
#include "Actor.h"
#include "Torch.h"
#include "trade.h"
#include "../CameraBase.h"
#include "Car.h"
#include "HudManager.h"
#include "UIGameSP.h"
#include "inventory.h"
#include "level.h"
#include "game_cl_base.h"
#include "xr_level_controller.h"
#include "UsableScriptObject.h"
void CActor::IR_OnKeyboardPress(int cmd)
{
	if (Remote())		return;
	if (IsSleeping())	return;
	if (IsTalking())	return;
	if (IsControlled())	return;


	if (!g_Alive()) return;

	if(m_holder && kUSE != cmd)
	{
		m_holder->OnKeyboardPress(cmd);
		return;
	}

	if(inventory().Action(cmd, CMD_START))						return;


	switch(cmd){
	case kACCEL:	mstate_wishful |= mcAccel;					break;
	case kR_STRAFE:	mstate_wishful |= mcRStrafe;				break;
	case kL_STRAFE:	mstate_wishful |= mcLStrafe;				break;
	case kFWD:		mstate_wishful |= mcFwd;					break;
	case kBACK:		mstate_wishful |= mcBack;					break;
	case kJUMP:		mstate_wishful |= mcJump;					break;
	case kCROUCH:	mstate_wishful |= mcCrouch;					break;
	case kCROUCH_TOGGLE:	
		{
			if (mstate_wishful & mcCrouch)
				mstate_wishful &=~mcCrouch;
			else
				mstate_wishful |= mcCrouch;					
		}break;

	case kCAM_1:	cam_Set			(eacFirstEye);				break;
	case kCAM_2:	cam_Set			(eacLookAt);				break;
	case kCAM_3:	cam_Set			(eacFreeLook);				break;

	case kNIGHT_VISION: {
		PIItem I = inventory().Get(CLSID_DEVICE_TORCH, false); 
		if (I){
			CTorch* torch = smart_cast<CTorch*>(I);
			if (torch) torch->SwitchNightVision();
		}
		}break;
	case kTORCH:{ 
		PIItem I = inventory().Get(CLSID_DEVICE_TORCH, false); 
		if (I){
			CTorch* torch = smart_cast<CTorch*>(I);
			if (torch) torch->Switch();
		}
		}break;
	case kWPN_1:	
	case kWPN_2:	
	case kWPN_3:	
	case kWPN_4:	
	case kWPN_5:	
	case kWPN_6:	
	case kWPN_7:	
	case kWPN_8:	
	case kWPN_9:	
		//Weapons->ActivateWeaponID	(cmd-kWPN_1);			
		break;
	case kBINOCULARS:
		//Weapons->ActivateWeaponID	(Weapons->WeaponCount()-1);
		break;
	case kWPN_RELOAD:
		//Weapons->Reload			();
		break;
	case kUSE:
		ActorUse();
		break;
	case kDROP:
		b_DropActivated			= TRUE;
		f_DropPower				= 0;
		break;
		//-----------------------------------------------------
		/*
	case kHyperJump:
		{
			Fvector pos	= Device.vCameraPosition;
			Fvector dir = Device.vCameraDirection;

			Level().CurrentControlEntity()->setEnabled(false);
			Collide::rq_result result;
			BOOL reach_wall = Level().ObjectSpace.RayPick(pos, dir, 100.0f, 
				Collide::rqtBoth, result) && !result.O;
			Level().CurrentControlEntity()->setEnabled(true);			
			////////////////////////////////////
			if (!reach_wall || result.range < 1) break;

			dir.mul(result.range-0.5f);
			Fmatrix	M = Level().CurrentControlEntity()->XFORM();
			M.translate_add(dir);
			Level().CurrentControlEntity()->ForceTransform(M);
		}break;
		*/
	case kHyperKick:
		{
			m_dwStartKickTime = Level().timeServer();
		}break;
		//-----------------------------------------------------
	}
}

void CActor::IR_OnKeyboardRelease(int cmd)
{
	if (Remote())		return;
	if (IsSleeping())	return;
	if (IsControlled())	return;

	if (g_Alive())	
	{
		if(m_holder)
		{
			m_holder->OnKeyboardRelease(cmd);
			return;
		}

		if(inventory().Action(cmd, CMD_STOP)) return;

		if (cmd == kUSE) 
		{
			PickupModeOff();
		}


		switch(cmd)
		{
		case kACCEL:	mstate_wishful &=~mcAccel;		break;
		case kR_STRAFE:	mstate_wishful &=~mcRStrafe;	break;
		case kL_STRAFE:	mstate_wishful &=~mcLStrafe;	break;
		case kFWD:		mstate_wishful &=~mcFwd;		break;
		case kBACK:		mstate_wishful &=~mcBack;		break;
		case kJUMP:		mstate_wishful &=~mcJump;		break;
		case kCROUCH:	mstate_wishful &=~mcCrouch;		break;

		case kHyperKick:
			{
				u32 FullKickTime = Level().timeServer() - m_dwStartKickTime;
				
				Collide::rq_result& RQ = HUD().GetCurrentRayQuery();
				CActor* pActor = smart_cast<CActor*>(RQ.O);
				if (!pActor || pActor->g_Alive()) break;

				Fvector original_dir, position_in_bone_space;
				original_dir.set(0, 1, 0);
				position_in_bone_space.set(0, 1, 0);				

				NET_Packet		P;
				CGameObject::u_EventGen	(P,GE_HIT,RQ.O->ID());
				P.w_u16			(ID());
				P.w_u16			(ID());
				P.w_dir			(original_dir);
				P.w_float		(0);
				P.w_s16			((s16)RQ.element);
				P.w_vec3		(position_in_bone_space);
				P.w_float		(float(FullKickTime)*10);
				P.w_u16			(2);
				Level().Send(P);

			}break;
		case kDROP:		if(GAME_PHASE_INPROGRESS == Game().Phase()) g_PerformDrop();				break;
		}
	}
}

void CActor::IR_OnKeyboardHold(int cmd)
{
	if (Remote() || !g_Alive())		return;
	if (IsSleeping())				return;
	if (IsControlled())				return;
	if (IsTalking())				return;

	if(m_holder)
	{
		m_holder->OnKeyboardHold(cmd);
		return;
	}

	switch(cmd)
	{
	case kUP:
	case kDOWN: 
	case kCAM_ZOOM_IN: 
	case kCAM_ZOOM_OUT: 
		cam_Active()->Move(cmd); break;
	case kLEFT:
	case kRIGHT:
		if (eacFreeLook!=cam_active) cam_Active()->Move(cmd); break;
	}
}

void CActor::IR_OnMouseMove(int dx, int dy)
{
	if (Remote())		return;
	if (IsSleeping())	return;

	if(m_holder) 
	{
		m_holder->OnMouseMove(dx,dy);
		return;
	}

	if (!IsControlled()) m_controlled_mouse_scale_factor = 1.0f;
	VERIFY(!fis_zero(m_controlled_mouse_scale_factor));

	CCameraBase* C	= cameras	[cam_active];
	float scale		= (C->f_fov/DEFAULT_FOV)*psMouseSens * psMouseSensScale/50.f  / m_controlled_mouse_scale_factor;
	if (dx){
		float d = float(dx)*scale;
		cam_Active()->Move((d<0)?kLEFT:kRIGHT, _abs(d));
	}
	if (dy){
		float d = ((psMouseInvert.test(1))?-1:1)*float(dy)*scale*3.f/4.f;
		cam_Active()->Move((d>0)?kUP:kDOWN, _abs(d));
	}
}

void CActor::ActorUse()
{
	PickupModeOn();
	bool picked_up = PickupTarget();
	BOOL trace_use = strstr(Core.Params,"-traceuse") ? TRUE : FALSE;

	if(m_PhysicMovementControl->PHCapture())
		m_PhysicMovementControl->PHReleaseObject();

	Collide::rq_result RQ = HUD().GetCurrentRayQuery();
	if(!RQ.O)
	{
		CObject* current_entity = Level().CurrentEntity();
		if(current_entity)
			current_entity->setEnabled(false);
		Level().ObjectSpace.RayPick(Device.vCameraPosition, Device.vCameraDirection, m_fPickupInfoRadius, Collide::rqtBoth, RQ);
		if(current_entity)
			current_entity->setEnabled(true);
	}

	CGameObject* focused_object = smart_cast<CGameObject*>(RQ.O);
	CPhysicsShellHolder* object = smart_cast<CPhysicsShellHolder*>(RQ.O);
	CUsableScriptObject* pFocusedUsable = smart_cast<CUsableScriptObject*>(RQ.O);
	CUsableScriptObject* pUsableObject = m_pUsableObject ? m_pUsableObject : pFocusedUsable;
	CInventoryOwner* pFocusedPerson = smart_cast<CInventoryOwner*>(RQ.O);
	CInventoryOwner* pPersonWeLookingAt = m_pPersonWeLookingAt ? m_pPersonWeLookingAt : pFocusedPerson;
	CHolderCustom* pFocusedVehicle = smart_cast<CHolderCustom*>(RQ.O);
	CHolderCustom* pVehicleWeLookingAt = m_pVehicleWeLookingAt ? m_pVehicleWeLookingAt : pFocusedVehicle;
	CCar* pFocusedCar = smart_cast<CCar*>(pVehicleWeLookingAt);
	if(!object && pFocusedCar)
		object = smart_cast<CPhysicsShellHolder*>(pFocusedCar);

	if(trace_use)
	{
		Msg("[use] ray obj=%p name=%s section=%s range=%.3f cls=%I64u picked=%d usable=%p person=%p vehicle=%p focused_vehicle=%p physics=%p shift=%d",
			RQ.O,
			focused_object ? *focused_object->cName() : "<none>",
			focused_object ? *focused_object->cNameSect() : "<none>",
			RQ.range,
			focused_object ? focused_object->SUB_CLS_ID : u64(0),
			picked_up ? 1 : 0,
			pUsableObject,
			pPersonWeLookingAt,
			m_pVehicleWeLookingAt,
			pFocusedVehicle,
			object,
			Level().IR_GetKeyState(DIK_LSHIFT) ? 1 : 0);
	}

	if(pUsableObject)
	{
		if(trace_use)
			Msg("[use] script usable: nonscript=%d", pUsableObject->nonscript_usable() ? 1 : 0);
		pUsableObject->use(this);
	}

	if(!pUsableObject || pUsableObject->nonscript_usable() || pFocusedVehicle)
	{
		if(pPersonWeLookingAt)
		{
			CEntityAlive* pEntityAliveWeLookingAt =
				smart_cast<CEntityAlive*>(pPersonWeLookingAt);

			VERIFY(pEntityAliveWeLookingAt);

			if(trace_use)
				Msg("[use] person branch: alive=%d shift=%d", pEntityAliveWeLookingAt->g_Alive() ? 1 : 0, Level().IR_GetKeyState(DIK_LSHIFT) ? 1 : 0);

			if(pEntityAliveWeLookingAt->g_Alive())
			{
				TryToTalk();
			}
			//обыск трупа
			else  if(!Level().IR_GetKeyState(DIK_LSHIFT))
			{
				//только если находимся в режиме single
				CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
				if(trace_use)
					Msg("[use] corpse inventory: ui=%p", pGameSP);
				if(pGameSP)pGameSP->StartCarBody(&inventory(), this,
					&pPersonWeLookingAt->inventory(),
					smart_cast<CGameObject*>(pPersonWeLookingAt));
			}
		}
		else if(pVehicleWeLookingAt && pFocusedCar && Level().IR_GetKeyState(DIK_LSHIFT))
		{
			//только если находимся в режиме single
			CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
			if(trace_use)
				Msg("[use] car inventory branch: ui=%p", pGameSP);
			if(pGameSP)pGameSP->StartCarBody(&inventory(), this,
				pVehicleWeLookingAt->GetInventory(),
				smart_cast<CGameObject*>(pVehicleWeLookingAt));

		}
		u16 element = BI_NONE;
		if(object)
			element = (u16)RQ.element;

		if(Level().IR_GetKeyState(DIK_LSHIFT))
		{

			if(!m_PhysicMovementControl->PHCapture())
			{
				if(trace_use)
					Msg("[use] physics capture: object=%p element=%d", object, element);
				m_PhysicMovementControl->PHCaptureObject(object,element);

			}

		}
		else
		{
			if (object)
			{
				if(trace_use)
					Msg("[use] physics holder branch: cls=%I64u", object->SUB_CLS_ID);
				switch (object->SUB_CLS_ID)
				{
				case CLSID_CAR:					if(use_Vehicle(object))			return;	break;
				case CLSID_OBJECT_W_MOUNTED:	if(use_MountedWeapon(object))	return;	break;
				}
			}
			else
			{
				if (m_holder)
				{
					CGameObject* holder			= smart_cast<CGameObject*>(m_holder);
					if(trace_use)
						Msg("[use] current holder branch: holder=%p cls=%I64u", holder, holder ? holder->SUB_CLS_ID : u64(0));
					switch (holder->SUB_CLS_ID)
					{
					case CLSID_CAR:					if(use_Vehicle(0))			return;	break;
					case CLSID_OBJECT_W_MOUNTED:	if(use_MountedWeapon(0))	return;	break;
					}
				}
				else if(trace_use)
					Msg("[use] no physics holder/current holder branch");
			}
		}
	}
	else if(trace_use)
		Msg("[use] blocked by script usable object");
}
//void CActor::IR_OnMousePress(int btn)