#pragma once
#include<windows.h>
#include"vector.h"

#define INITPTR_INVALID_WORLD	1 << 0
#define INITPTR_INVALID_PLAYER	1 << 1
#define INITPTR_INVALID_VEHICLE	1 << 2
#define INITPTR_INVALID_WEAPON	1 << 3
#define INITPTR_INVALID_TUNABLE 1 << 4
#define INITPTR_INVALID_GLOBAL  1 << 5
#define INITPTR_INVALID_PLAYER_LIST 1 << 6
#define INITPTR_INVALID_REPLAY_INTERFACE 1 << 7
#define INITPTR_INVALID_UNK_MODEL 1 << 7


FLOAT GetPlayerMaxHealth();//玩家最大血量
VOID SetPlayerMaxHealth(float value);

FLOAT GetPlayerHealth();//玩家当前血量
VOID SetPlayerHealth(float value);

FLOAT GetPlayerArmor();//防弹衣护甲值
VOID SetPlayerArmor(float value);

VOID SetWaterProof(float value);
VOID SetWaterProof(float value);

DWORD GetWanted();
VOID SetWanted(DWORD value);

FLOAT GetWantedCanChange();
VOID SetWantedCanChange(bool option);

VOID SetPlayerGod(bool option);
BYTE GetPlayerGod();
VOID SetVehicleGod(bool option);
BYTE GetVehicleGod();

VOID SetSwimSpeed(float times);
VOID SetRunSpeed(float times);

VOID SetNPCIgnore(bool option);
DWORD GetNPCIgnore();

VOID SetRagDoll(byte value);
BYTE GetRagDoll();
VOID NoRagDoll(bool option);

VOID SetFrameFlags(DWORD value);
DWORD GetFrameFlags();
VOID MakeFrameFlags(bool superJump, bool explosiveMelee, bool fireAmmo, bool explosiveAmmo);

BYTE IsIntVehicle();
v3 GetPlayerPos();
VOID SetPlayerPos(v3 player_pos);
v3 GetVehiclePos();
VOID SetVehiclePos(v3 vehiclePos);
v3 GetWayPoint();
VOID TransportToWayPoint();
v3 GetObjectivePoint();
VOID TransportToObjectivePoint();

FLOAT GetWeaponRange();
VOID SetWeaponRange(float value);
VOID InfWeaponRange(bool option);

VOID UndeadOffRadar(bool option);
VOID InfAmmo(bool option);
VOID QuickReload(bool option);
VOID  NoReload(bool option);
VOID NoRecoil(bool option);
VOID SetSpread(float value);
FLOAT GetSpread();
VOID NoSpread(bool option);
FLOAT GetRecoil();
VOID SetRecoil(float value);

VOID SetRocketRechargeSpeed(float value);
FLOAT GetRocketRechargeSpeed();
VOID SetApMult(float value);
FLOAT GetApMult();
VOID SetRpMult(float value);
FLOAT GetRpMult();
FLOAT GetSwimSpeed();
FLOAT GetRunSpeed();