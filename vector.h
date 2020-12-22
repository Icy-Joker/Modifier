#pragma once
struct v3
{
    float x = 0;
    float y = 0;
    float z = 0;
};

struct weaponData
{
	DWORD_PTR	m_dwpWeapon = 0;

	DWORD		m_dwHash = 0;
	DWORD		m_dwBulletBatch;
	DWORD		m_dwImpactType;
	DWORD		m_dwImpactExplosion;

	float		m_fSpread;
	float		m_fRecoil;
	float		m_fDamage;
	float		m_fReload;
	float		m_fReloadVeh;
	float		m_fRange;
	float		m_fSpinUp;
	float		m_fSpin;
	float		m_fMuzzleVelocity;
	float		m_fPenetration;
	float		m_fForceOnPed;
	float		m_fForceOnVehicle;
	float		m_fForceOnHeli;
};

enum ImpactTypeEnum
{
	Fists = 2,
	Bullets = 3,
	Explosives = 5,
};