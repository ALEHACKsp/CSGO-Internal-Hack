#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <Windows.h>
#include <mutex>

#include "GameStruct.h"

// ��ǰ�����汾
extern const std::wstring devVersion;

// Ŀ�����ID
extern DWORD targetPID;
// Ŀ���������
extern const std::wstring targetProcName;
// Ŀ�괰������
extern const std::wstring targetWndName;

// Ŀ�괰�ھ��
extern HWND hTargetWnd;
//
extern HWND hOverlayWnd;
// Ŀ�괰��Rect
extern RECT targetRect;
// Ŀ�괰�ڿ��
extern unsigned int targetWndWidth;
// Ŀ�괰�ڳ���
extern unsigned int targetWndHeight;

// ������ң���ʦ��а��
extern std::unique_ptr<Player> localPlayer;
// ���еĶ���
extern std::vector<std::unique_ptr<Player>> teammates;
// ���еĵ���
extern std::vector<std::unique_ptr<Player>> enemy;

// ����ʵ��
extern std::vector<std::unique_ptr<GlowObject>> glowObjects;
extern int glowObjectCount;
extern float glowColorTeammates[4];
extern float glowColorEnemy[4];
extern float glowColorWeapons[4];
extern float glowColorC4[4];

namespace FunctionEnableFlag
{
	extern bool bOverlay;
	extern bool bTriggerBot;
	extern bool bAimBot;
	extern bool bESP;
	extern bool bRadarHack;
	extern bool bBHop;
	extern bool bGlow;
	extern bool bWall;
	extern bool bMenu;
	extern bool bNULL;
	extern bool bSkinChanger;
	extern bool bReadLocalPlayerInfo;
	extern bool bReadOtherPlayerInfo;
	extern bool bReadGlowObjectInfo;
	extern bool bReadSkinInfo;
}

namespace ThreadExistFlag
{
	extern bool bGeneral;
	extern bool bOverlay;
	extern bool bTriggerBot;
	extern bool bAimBot;
	extern bool bESP;
	extern bool bRadarHack;
	extern bool bNULL;
}