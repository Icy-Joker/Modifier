#include<windows.h>
#include <CommCtrl.h>
#include <time.h>
#include<psapi.h>
#include"offset.h"
#include"hack.h"
#include"vector.h"

//#include"resource.h"

/************************************�궨��*************************************************************************/
#define WINDOW_WIDTH    400                         //���ڿ��
#define WINDOW_HEIGHT	300                      //���ڸ߶�
#define WINDOW_X(width)	(GetSystemMetrics(SM_CXSCREEN)-  width)/2                       //���ڸ߶�
#define WINDOW_Y(height)	(GetSystemMetrics(SM_CYSCREEN)-  height)/2                         //���ڸ߶�
#define WINDOW_TITLE    L"GTA5(1.52��)�޸���"			//���ڱ���
#define TARGET_PROGRAM_NAME		L"Grand Theft Auto V"//Grand Theft Auto V

#define ID_STATIC_ICON_TARGET		21

#define ReadMemory(x,y,z)		ReadMemoryEx(x,y,z,sizeof(z)/sizeof(z[0]))
#define WriteMemory(x,y,z)		WriteMemoryEx(x,y,z,sizeof(z)/sizeof(z[0]))
/************************************��������*************************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);  //���ڹ��̺���
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);//���̹���
LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam);//��깳��

DWORD WINAPI ThreadHack(LPVOID lpParameter);
DWORD WINAPI ThreadCheckStatus(LPVOID lpParameter);
DWORD WINAPI ThreadPlayerHack(LPVOID lpParameter);
DWORD WINAPI ThreadVehicleHack(LPVOID lpParameter);
DWORD WINAPI ThreadWeaponHack(LPVOID lpParameter);

void test();

BYTE InitModifier();
VOID RaiseToDebug();
VOID AttachProcess();
VOID SaveSettings();
VOID LoadSettings(HWND hwnd);

DWORD64 GetModuleAddress(const char* moduleName);

template<typename T, typename T2> VOID ReadMemoryEx(HANDLE hProcess, T* result, T2 address[], DWORD length);
template<typename T, typename T2> VOID WriteMemoryEx(HANDLE hProcess, T value, T2 address[], DWORD length);


void Init(HWND hwnd);//�����ʼ��
/************************************ȫ�ֱ���*************************************************************************/
HHOOK keyboardHook;//���̹���
HHOOK mouseHook;//���̹���
HWND hwnd;//��ǰ���ھ��
HWND hwndStatus;//״̬��

HANDLE handleThreadCheckStatus, handleThreadPlayerHack, handleThreadVehicleHack, handleThreadWeaponHack,handleThreadHack;


HWND hwndVehicleType;//�ؾ�����
HWND hwndVehicleItem;//�ؾ�����
HWND hwndSwimSpeed, hwndRunSpeed, hwmdAPMult, hwmdRPMult, hwmdReloadSpeed;
HWND hwndTarget;//Ŀ�괰�ھ��
HANDLE hProcess;//Ŀ�����ID

DWORD64 GAME_BASE;//Ŀ�����ģ����ص�ַ
DWORD64 WORLD_BASE;//GAME_BASE+ADDRESS_WORLD
DWORD64 PLAYER_BASE;//GAME_BASE+OFFSET_PLAYER
DWORD64 GLOBAL_BASE;//GAME_BASE+ADDRESS_GLOBAL
DWORD64 TUNABLE_BASE;//GAME_BASE+ADDRESS_TUNABLE
DWORD64 UNKMODEL_BASE;//GAME_BASE+ADDRESS_UNK_MODEL     UnkModelStruct=UnkModelBase+0
DWORD64 REPLAYINTERFACE_BASE;//GAME_BASE+ADDRESS_REPLAY_INTERFACE

DWORD64 ATTACKER_BASE;//PLAYER_BASE+OFFSET_ENTITY_ATTACKER
DWORD64 VEHICLE_BASE;//PLAYER_BASE+OFFSET_PLAYER_VEHICLE
DWORD64 PLAYER_INFO;//PLAYER_BASE+OFFSET_PLAYER_INFO
DWORD64 WEAPON_BASE;//PLAYER_BASE+ADDRESS_WEAPON
DWORD64 WEAPON_MANAGER;//PLAYER_BASE+OFFSET_WEAPON_MANAGER
DWORD64 WEAPON_CURRENT;//WEAPON_MANAGER+OFFSET_WEAPON_CURRENT
DWORD64 WEAPON_AMMOINFO;//WEAPON_CURRENT+WEAPON_AMMOINFO

DWORD		ADDRESS_WORLD = 0;
DWORD		ADDRESS_BLIP = 0;
DWORD		ADDRESS_AMMO = 0;
DWORD		ADDRESS_MAGAZINE = 0;
DWORD		ADDRESS_TUNABLE = 0;
DWORD		ADDRESS_TRIGGER = 0;
DWORD		ADDRESS_GLOBAL = 0;
DWORD		ADDRESS_PLAYER_LIST = 0;
DWORD		ADDRESS_REPLAY_INTERFACE = 0;
DWORD		ADDRESS_UNK_MODEL = 0;

TCHAR buff[1024];//���ݻ���
RECT rect;//�ͻ������νṹ
POINT mouse;
DWORD width[3];
DWORD length;

v3 player_pos_current;
v3 target_pos;

/************************************��������*************************************************************************/
int WINAPI   WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEX wndClass = { 0 };                                    //��WINDCLASSEX������һ��������
    wndClass.cbSize = sizeof(WNDCLASSEX);                           //���ýṹ����ֽ�����С
    wndClass.style = CS_HREDRAW | CS_VREDRAW;                       //���ô��ڵ���ʽ
    wndClass.lpfnWndProc = WndProc;                                 //����ָ�򴰿ڹ��̺�����ָ��
    wndClass.cbClsExtra = 0;                                        //������ĸ����ڴ棬ȡ0�Ϳ�����
    wndClass.cbWndExtra = 0;                                        //���ڵĸ����ڴ棬��Ȼȡ0������
    wndClass.hInstance = hInstance;                                 //ָ���������ڹ��̵ĳ����ʵ�������
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);				//���ؼ����Զ���icoͼ��
    wndClass.hCursor = LoadCursor(NULL, (LPCTSTR)IDC_ARROW);        //ָ��������Ĺ������
    wndClass.hbrBackground = (HBRUSH)CreateSolidBrush(0x00FFFFFF);   //ΪhbrBackground��Աָ��һ����ɫ��ˢ���    
    wndClass.lpszMenuName = NULL;                                   //ָ���˵���Դ�����֡�
    wndClass.lpszClassName = WINDOW_TITLE;							//ָ������������֡�

    if (!RegisterClassEx(&wndClass))				//����괰�ں���Ҫ�Դ��������ע�ᣬ�������ܴ��������͵Ĵ���
    {
        MessageBox(NULL, L"ע��ʧ��", WINDOW_TITLE, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindowEx(WS_EX_LAYERED,
        WINDOW_TITLE,
        WINDOW_TITLE,
        WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX,
        WINDOW_X(WINDOW_WIDTH),
        WINDOW_Y(WINDOW_HEIGHT),
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 220, LWA_ALPHA);//ȫ��͸��
    ShowWindow(hwnd, nShowCmd);                                         //����ShowWindow��������ʾ����
    UpdateWindow(hwnd);                                                 //�Դ��ڽ��и��£��������������·���Ҫװ��һ��

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)&KeyboardProc, GetModuleHandle(NULL), NULL);
    //mouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)&MouseProc, GetModuleHandle(NULL), NULL);

    MSG msg = { 0 };                                    //���岢��ʼ��msg
    while (msg.message != WM_QUIT)                      //ʹ��whileѭ���������Ϣ����WM_QUIT��Ϣ���ͼ���ѭ��
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))      //�鿴Ӧ�ó�����Ϣ���У�����Ϣʱ�������е���Ϣ�ɷ���ȥ��
        {
            TranslateMessage(&msg);                     //���������Ϣת��Ϊ�ַ���Ϣ
            DispatchMessage(&msg);                      //�ַ�һ����Ϣ�����ڳ���
        }
    }


    //UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);
    UnregisterClass(WINDOW_TITLE, wndClass.hInstance);       //����׼��������ע��������
    return 0;
}

/******************************************�ص�����*******************************************************************/
LRESULT CALLBACK  WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc, mdc;
    int wmId, wmCode;

    switch (message)                            //switch��俪ʼ
    {
    case WM_CREATE:
        Init(hwnd);
        RaiseToDebug();
        LoadSettings(hwnd);

        handleThreadCheckStatus = CreateThread(NULL, 0, ThreadCheckStatus, NULL, 0, NULL);
        handleThreadHack = CreateThread(NULL, 0, ThreadHack, NULL, 0, NULL);
        handleThreadPlayerHack = CreateThread(NULL, 0, ThreadPlayerHack, NULL, 0, NULL);
        handleThreadWeaponHack = CreateThread(NULL, 0, ThreadWeaponHack, NULL, 0, NULL);
        handleThreadVehicleHack = CreateThread(NULL, 0, ThreadVehicleHack, NULL, 0, NULL);

        break;
    case WM_PAINT:                              // ���ǿͻ����ػ���Ϣ
        //InvalidateRect(hwnd, NULL, TRUE);
        hdc = BeginPaint(hwnd, &ps);
        mdc = CreateCompatibleDC(hdc);
        // TODO: �ڴ���������ͼ����...

        DeleteDC(mdc);
        EndPaint(hwnd, &ps);
        break;

    case WM_COMMAND:
    {
        wmId = LOWORD(wParam);
        wmCode = HIWORD(wParam);
        switch (wmId)
        {
        case 523:

            switch (wmCode)
            {
            case BN_CLICKED:

                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetPlayerGod(true);
                }
                else
                {
                    SetPlayerGod(false);
                }
                break;
            default:
                break;
            }
            break;
        case 524:

            switch (wmCode)
            {
            case BN_CLICKED:

                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetVehicleGod(true);
                }
                else
                {
                    SetVehicleGod(false);
                }
                break;
            default:
                break;
            }
            break;
        case 525:

            switch (wmCode)
            {
            case BN_CLICKED:

                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetWantedCanChange(false);
                }
                else
                {
                    SetWantedCanChange(true);
                }
                break;
            default:
                break;
            }
            break;
        case 526:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetNPCIgnore(true);
                }
                else
                {
                    SetNPCIgnore(false);
                }
                break;
            default:
                break;
            }
            break;
        case 527:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    NoRagDoll(true);
                }
                else
                {
                    NoRagDoll(false);
                }
                break;
            default:
                break;
            }
        case 530:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    InfAmmo(true);
                }
                else
                {
                    InfAmmo(false);
                }
                break;
            default:
                break;
            }
            break;

        case 531:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    NoReload(true);
                }
                else
                {
                    NoReload(false);
                }
                break;
            default:
                break;
            }
            break;
        case 532:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    NoSpread(true);
                }
                else
                {
                    NoSpread(false);
                }
                break;
            default:
                break;
            }
            break;
        case 533:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    NoRecoil(true);
                }
                else
                {
                    NoRecoil(false);
                }
                break;
            default:
                break;
            }
            break;
       /* case 534:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    QuickReload(true);
                }
                else
                {
                    QuickReload(false);
                }
                break;
            default:
                break;
            }
            break;*/
        case 539:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    UndeadOffRadar(true);
                }
                else
                {
                    UndeadOffRadar(false);
                }
                break;
            default:
                break;
            }
            break;
        case 540:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    InfWeaponRange(true);
                }
                else
                {
                    InfWeaponRange(false);
                }
                break;
            default:
                break;
            }
            break;
        case 541:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                     SetSwimSpeed((float)SendMessage(hwndSwimSpeed, TBM_GETPOS, 0, 0));
                }
                else
                {
                    SetSwimSpeed(1.f);
                }
                break;
            default:
                break;
            }
            break;
        case 542:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetRunSpeed((float)SendMessage(hwndRunSpeed, TBM_GETPOS, 0, 0)); 
                }
                else
                {
                    SetRunSpeed(1.f);
                }
                break;
            default:
                break;
            }
            break;
        case 543:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetRpMult((float)SendMessage(hwmdRPMult, TBM_GETPOS, 0, 0));
                }
                else
                {
                    SetRpMult(1.f);
                }
                break;
            default:
                break;
            }
            break;
        case 544:
            switch (wmCode)
            {
            case BN_CLICKED:
                if (SendMessage(GetDlgItem(hwnd, wmId), BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    SetApMult((float)SendMessage(hwmdAPMult, TBM_GETPOS, 0, 0));
                }
                else
                {
                    SetApMult(1.f);
                }
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
    }
    break;
    case WM_NOTIFY:
        switch (wParam)
        {
        case 4:

            break;
        }
        break;
    case WM_CLOSE:
        switch (MessageBox(hwnd, L"Confrim to Quit", L"Notice", 1))
        {
        case IDOK:
            SaveSettings();
            DestroyWindow(hwnd);
            break;
        case IDCANCEL:
        default:
            break;
        }
        break;
    case WM_DESTROY:                //���Ǵ���������Ϣ
        PostQuitMessage(0);         //��ϵͳ�����и��߳�����ֹ����������ӦWM_DESTROY��Ϣ
        break;

    default:                        //������case�����������ϣ���ִ�и�default���
        return DefWindowProc(hwnd, message, wParam, lParam);        //����ȱʡ�Ĵ��ڹ���
    }
    return 0;           //�����˳�
}

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= HC_ACTION)
    {
        //DWORD vk_code = ((MOUSEHOOKSTRUCT*)lParam)->vkCode;
        switch (wParam)
        {
        case WM_LBUTTONDOWN:

            break;
        case WM_LBUTTONUP:

            break;
        case WM_MOUSEMOVE:
            break;
        default:
            break;
        }
    }
    return CallNextHookEx(mouseHook, code, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= HC_ACTION)
    {
        DWORD vk_code = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;
        switch (wParam)
        {
        case WM_KEYDOWN:
            switch (vk_code)
            {
            default:
                break;
            }
            break;
        case WM_KEYUP:
            switch (vk_code)
            {
            case VK_F1://�ָ�Ѫ���ͻ���
                SetPlayerHealth(GetPlayerMaxHealth());
                SetPlayerArmor(100.0);
                break;
            case VK_F2://ȡ��ͨ��
                SetWanted(0);
                break;
            case VK_F5:
                TransportToWayPoint();
                break;
            case VK_F6:
                TransportToObjectivePoint();
                break;
            case VK_F8:
                test();
            break;
            default:
                /*  wsprintf(buff, L"�������:%d\n", vk_code);
                  OutputDebugString(buff);*/
                break;
            }
            break;
        default:
            break;
        }
    }
    return CallNextHookEx(keyboardHook, code, wParam, lParam);
}

void test() {
    {
        OutputDebugString(L"789");
        static v3 objective_point;
         DWORD64 n;
         DWORD objColor, objIcon;

        static int ColorYellowMission = 66;
        static int ColorYellow = 0x0101;
        static int ColorWhite = 0x0000;
        static int ColorGreen =0x10;
        static int SpriteCrateDrop = 306;
        static int Spriaddressandard = 1;
        static int SpriteRaceFinish = 38;

        for (size_t i = 2000; i > 1; i--)
        {

            DWORD64 address0[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) };
            DWORD64 address1[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x40 };
            DWORD64 address2[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x48 };
            DWORD64 address3[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x10 };

            ReadMemory(hProcess, &n, address0);
            ReadMemory(hProcess, &objIcon, address1);
            ReadMemory(hProcess, &objColor, address2);
            /* if (n > 0 && objColor == ColorYellowMission || objIcon == Spriaddressandard
                 && objColor == ColorYellow || objIcon == Spriaddressandard
                 && objColor == ColorWhite || objIcon == SpriteRaceFinish
                 && objColor == ColorGreen || objIcon == Spriaddressandard
                 && objColor == SpriteCrateDrop||)
             {
                 ReadMemory(hProcess, &objective_point, address3);
                 break;
             }*/
            if (objColor!=0&&objColor!=2&&objColor!=2&&objColor != 55&& objColor != 62&& objColor != 3&& objColor != 4&& objColor != 1&& objColor != 5&& objColor != 60 && objColor != 51)
            {
                ReadMemory(hProcess, &objective_point, address3);
                wsprintf(buff, L"n: %d\tobjIcon:%d\tobjColor:%x\n", n, objIcon, objColor);
                OutputDebugString(buff);
            }

            
        }


    }
}

void Init(HWND hwnd)//��ʼ���ؼ�
{
    //	srand(time(NULL));
    GetClientRect(hwnd, &rect);

    //�����޸�
    CreateWindow(L"Button", L"�����޵�", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 0, 90, 20, hwnd, (HMENU)523, NULL, NULL);
    CreateWindow(L"Button", L"�ؾ��޵�", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 20, 90, 20, hwnd, (HMENU)524, NULL, NULL);
    CreateWindow(L"Button", L"����ͨ��", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 40, 90, 20, hwnd, (HMENU)525, NULL, NULL);
    CreateWindow(L"Button", L"NPC����", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 60, 90, 20, hwnd, (HMENU)526, NULL, NULL);
    CreateWindow(L"Button", L"�޲�����", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 80, 90, 20, hwnd, (HMENU)527, NULL, NULL);
    //CreateWindow(L"Button", L"ˮ������", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 100, 90, 20, hwnd, (HMENU)528, NULL, NULL);
    //CreateWindow(L"Button", L"��������", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 120, 90, 20, hwnd, (HMENU)529, NULL, NULL);
    CreateWindow(L"Button", L"�����ӵ�", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 140, 90, 20, hwnd, (HMENU)530, NULL, NULL);
    CreateWindow(L"Button", L"���軻��", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 140, 90, 20, hwnd, (HMENU)531, NULL, NULL);
    CreateWindow(L"Button", L"��������", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 100, 90, 20, hwnd, (HMENU)539, NULL, NULL);

    CreateWindow(L"Button", L"�����ۼ�", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 190, 120, 90, 20, hwnd, (HMENU)532, NULL, NULL);
    CreateWindow(L"Button", L"�޺�����", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 10, 120, 90, 20, hwnd, (HMENU)533, NULL, NULL);
    //CreateWindow(L"Button", L"���ٻ���", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 80, 90, 20, hwnd, (HMENU)534, NULL, NULL);
    //hwmdReloadSpeed = CreateWindow(TRACKBAR_CLASS, L"�����ٶ�:", TBS_BOTTOM | TBS_HORZ | TBS_RIGHT | TBS_TOOLTIPS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 190, 80, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    //SendMessage(hwmdReloadSpeed, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(10.0, 50.0));
    CreateWindow(L"Button", L"������Ծ", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 80, 90, 20, hwnd, (HMENU)535, NULL, NULL);
    CreateWindow(L"Button", L"��ը��ս", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX,190, 80, 90, 20, hwnd, (HMENU)536, NULL, NULL);
    CreateWindow(L"Button", L"��ը�ӵ�", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 100, 90, 20, hwnd, (HMENU)538, NULL, NULL);
    CreateWindow(L"Button", L"ȼ���ӵ�", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 190, 100, 90, 20, hwnd, (HMENU)537, NULL, NULL);
    CreateWindow(L"Button", L"�������", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 120, 90, 20, hwnd, (HMENU)540, NULL, NULL);

    //ˢ��
    //CreateWindow(L"STATIC", L"ˢ��", WS_CHILD | WS_VISIBLE, 20, 200, 50, 20, hwnd, (HMENU)5, NULL, NULL);
   /* hwndVehicleType = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 100, 20, 85, 200, hwnd, (HMENU)5, NULL, NULL);
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("---------------"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("����DLC"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("���� "));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("�ܳ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("�����ܳ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("����"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("���⳵"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("ԽҰ��"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("Ħ�г�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("���ó���"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("Ӧ������"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("������"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("�γ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("��ҵ����"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("С�ͳ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("���ó�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("�ɻ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("����;��"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("ֱ���ɻ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("��ʽ����"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("��"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("���г�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("��"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("�ϳ�"));
    SendMessage(hwndVehicleType, CB_ADDSTRING, 0, (LPARAM)TEXT("SUV"));
    SendMessage(hwndVehicleType, CB_SETCURSEL, 0, NULL);

    hwndVehicleItem = CreateWindow(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 220, 20, 90, 120, hwnd, (HMENU)3, NULL, NULL);
    SendMessage(hwndVehicleItem, CB_ADDSTRING, 0, (LPARAM)TEXT("---------------"));
    SendMessage(hwndVehicleItem, CB_SETCURSEL, 0, NULL);
    CreateWindow(L"BUTTON", L"ˢ��", WS_CHILD | WS_VISIBLE, 315, 21, 40, 20, hwnd, (HMENU)1, NULL, NULL);

    CreateWindow(L"STATIC", L"���Ѫ��", WS_CHILD | WS_VISIBLE|SS_CENTER, 100,42, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("1000"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NOHIDESEL | ES_WANTRETURN, 220, 41, 80, 20, hwnd, (HMENU)1, NULL, NULL);
    CreateWindow(L"BUTTON", L"����", WS_CHILD | WS_VISIBLE, 315, 41, 40, 20, hwnd, (HMENU)1, NULL, NULL);

    CreateWindow(L"STATIC", L"����", WS_CHILD | WS_VISIBLE | SS_CENTER, 100, 62, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("0"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NOHIDESEL | ES_WANTRETURN, 220, 61, 20, 20, hwnd, (HMENU)425, NULL, NULL);
    CreateWindow(UPDOWN_CLASS, NULL, UDS_SETBUDDYINT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 240, 61, 30, 20, hwnd, (HMENU)426, NULL, NULL);
    SendMessage(GetDlgItem(hwnd, 426), UDM_SETBUDDY, (WPARAM)GetDlgItem(hwnd, 425), 0);
    SendMessage(GetDlgItem(hwnd, 426), UDM_SETRANGE, 0, MAKELPARAM(5, 0));
    SendMessage(GetDlgItem(hwnd, 426), UDM_SETPOS, 0, 0);
    CreateWindow(L"BUTTON", L"����", WS_CHILD | WS_VISIBLE, 315, 61, 40, 20, hwnd, (HMENU)1, NULL, NULL);*/


    CreateWindow(L"BUTTON", L"�����ٶ�:", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 40, 90, 20, hwnd, (HMENU)542, NULL, NULL);
    hwndRunSpeed = CreateWindow(TRACKBAR_CLASS, L"�����ٶ�:", TBS_BOTTOM | TBS_HORZ | TBS_RIGHT | TBS_TOOLTIPS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 190, 40, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    SendMessage(hwndRunSpeed, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1.0, 5.0));

    CreateWindow(L"BUTTON", L"��Ӿ�ٶ�:", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 60, 90, 20, hwnd, (HMENU)541, NULL, NULL);
    hwndSwimSpeed = CreateWindow(TRACKBAR_CLASS, L"��Ӿ�ٶ�:", TBS_BOTTOM | TBS_HORZ | TBS_RIGHT | TBS_TOOLTIPS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 190, 60, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    SendMessage(hwndSwimSpeed, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1.0, 5.0));

    CreateWindow(L"BUTTON", L"RP����:", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100, 0, 90, 20, hwnd, (HMENU)543, NULL, NULL);
    hwmdRPMult = CreateWindow(TRACKBAR_CLASS, L"RP����:", TBS_BOTTOM | TBS_HORZ | TBS_RIGHT | TBS_TOOLTIPS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 190, 0, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    SendMessage(hwmdRPMult, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1.0, 1000.0));

    CreateWindow(L"BUTTON", L"AP����:", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 100,20, 90, 20, hwnd, (HMENU)544, NULL, NULL);
    hwmdAPMult = CreateWindow(TRACKBAR_CLASS, L"AP����:", TBS_BOTTOM | TBS_HORZ | TBS_RIGHT | TBS_TOOLTIPS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 190, 20, 90, 20, hwnd, (HMENU)1, NULL, NULL);
    SendMessage(hwmdAPMult, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1.0, 1000.0));





    hwndStatus = CreateWindow(STATUSCLASSNAME, L"", SBS_SIZEGRIP | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)999, NULL, NULL);
    GetClientRect(hwnd, &rect);
    length = rect.right / 10;
    width[0] = length * 2;
    width[1] = length * 8;
    width[2] = length * 10;
    SendMessage(hwndStatus, SB_SETPARTS, 3, (LPARAM)(LPINT)width);
}

VOID SaveSettings() {

    WritePrivateProfileString(L"Settings", L"PlayerGod", SendMessage(GetDlgItem(hwnd, 523), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"VehicleGod", SendMessage(GetDlgItem(hwnd, 524), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"NeverWanted", SendMessage(GetDlgItem(hwnd, 525), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"NPCIgnore", SendMessage(GetDlgItem(hwnd, 526), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"NoRagDoll", SendMessage(GetDlgItem(hwnd, 527), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"InfAmmo", SendMessage(GetDlgItem(hwnd, 530), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"NoReload", SendMessage(GetDlgItem(hwnd, 531), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"NoSpread", SendMessage(GetDlgItem(hwnd, 532), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"NoRecoil", SendMessage(GetDlgItem(hwnd, 533), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    //WritePrivateProfileString(L"Settings", L"QuickReLoad", SendMessage(GetDlgItem(hwnd, 534), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"SuperJump", SendMessage(GetDlgItem(hwnd, 535), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"ExplosiveMelee", SendMessage(GetDlgItem(hwnd, 536), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"FireAmmo", SendMessage(GetDlgItem(hwnd, 537), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"ExplosiveAmmo", SendMessage(GetDlgItem(hwnd, 538), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"UndeadOffRadar", SendMessage(GetDlgItem(hwnd, 539), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"InfWeaponRange", SendMessage(GetDlgItem(hwnd, 540), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"SwimSpeed", SendMessage(GetDlgItem(hwnd, 541), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"RunSpeed", SendMessage(GetDlgItem(hwnd, 542), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"RPMult", SendMessage(GetDlgItem(hwnd, 543), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    WritePrivateProfileString(L"Settings", L"APMult", SendMessage(GetDlgItem(hwnd, 544), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    //WritePrivateProfileString(L"Settings", L"", SendMessage(GetDlgItem(hwnd, 52), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
    //WritePrivateProfileString(L"Settings", L"", SendMessage(GetDlgItem(hwnd, 52), BM_GETCHECK, 0, 0) == BST_CHECKED ? L"1" : L"0", L".\\setting.ini");
}
VOID LoadSettings(HWND hwnd) {
    //HWND hwndPlayerGod, hwndVehicleGod, hwndNeverWanted, hwndNPCIgnore, hwndNoRagDoll, hwndInfAmmo, hwndNoReload, hwndNoSpread, hwndNoRecoil, hwndQuickLoad, hwndSuperJump, hwndExplosiveMelee, hwndFireAmmo, hwndExplosiveAmmo, hwndSwimSpeed, hwndRunSpeed;
    //MessageBox(NULL, GetPrivateProfileInt(L"Settings", L"PlayerGod", 0, L".\\setting.ini") == 1 ? L"BST_CHECKED" : L"BST_UNCHECKED", L"123", 0);
    SendMessage(GetDlgItem(hwnd, 523), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"PlayerGod", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 524), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"VehicleGod", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 525), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"NeverWanted", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 526), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"NPCIgnore", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 527), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"NoRagDoll", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 530), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"InfAmmo", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 531), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"NoReload", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 532), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"NoSpread", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 533), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"NoRecoil", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    //SendMessage(GetDlgItem(hwnd, 534), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"QuickReLoad", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 535), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"SuperJump", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 536), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"ExplosiveMelee", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 537), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"FireAmmo", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 538), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"ExplosiveAmmo", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 539), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"UndeadOffRadar", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 540), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"InfWeaponRange", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 541), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"SwimSpeed", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 542), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"RunSpeed", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 543), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"RPMult", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hwnd, 544), BM_SETCHECK, GetPrivateProfileInt(L"Settings", L"APMult", 1, L".\\setting.ini") == 1 ? BST_CHECKED : BST_UNCHECKED, 0);
}

DWORD WINAPI ThreadCheckStatus(LPVOID lpParameter)//�����Ϸ״̬
{

    while (true)
    {	//Grand Theft Auto V ; Red Alert 2 ;
        switch (FindWindow(NULL, TARGET_PROGRAM_NAME) != NULL)
        {
        case TRUE:
            AttachProcess();
            SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)TEXT("  ��Ϸ������"));
           
            while (FindWindow(NULL, TARGET_PROGRAM_NAME) != NULL);
            SuspendThread(handleThreadHack);
            SuspendThread(handleThreadPlayerHack);
            SuspendThread(handleThreadVehicleHack);
            SuspendThread(handleThreadWeaponHack);
            break;
        default:
            SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)TEXT("  ��Ϸδ����"));
            while (FindWindow(NULL, TARGET_PROGRAM_NAME) == NULL);
            ResumeThread(handleThreadHack);
            ResumeThread(handleThreadPlayerHack);
            ResumeThread(handleThreadVehicleHack);
            ResumeThread(handleThreadWeaponHack);
            break;
        }
    }
}

DWORD WINAPI ThreadHack(LPVOID lpParameter)
{
    while (true)
    {
        BYTE r = InitModifier();

        if (!(r & INITPTR_INVALID_WORLD) && !(r & INITPTR_INVALID_PLAYER))
        {
            if (!(r & INITPTR_INVALID_TUNABLE))
            {
                if (SendMessage(GetDlgItem(hwnd, 543), BM_GETCHECK, 0, 0) == BST_CHECKED && (float)SendMessage(hwmdRPMult, TBM_GETPOS, 0, 0) != GetRpMult())
                {
                    SetRpMult((float)SendMessage(hwmdRPMult, TBM_GETPOS, 0, 0));
                }
                if (SendMessage(GetDlgItem(hwnd, 544), BM_GETCHECK, 0, 0) == BST_CHECKED && (float)SendMessage(hwmdAPMult, TBM_GETPOS, 0, 0) != GetApMult())
                {
                    SetApMult((float)SendMessage(hwmdAPMult, TBM_GETPOS, 0, 0));
                }
            }
        }
        Sleep(5000);
    }
}



DWORD WINAPI ThreadPlayerHack(LPVOID lpParameter)
{
    while (true)
    {
        if (GAME_BASE!=0&& WORLD_BASE !=0)
        {
            OutputDebugString(L"Player Hack\n");
            ReadProcessMemory(hProcess, (LPVOID)(WORLD_BASE + OFFSET_PLAYER), &PLAYER_BASE, sizeof(DWORD64), 0);
            if (PLAYER_BASE != 0)
            {
                MakeFrameFlags(SendMessage(GetDlgItem(hwnd, 535), BM_GETCHECK, 0, 0) == BST_CHECKED, SendMessage(GetDlgItem(hwnd, 536), BM_GETCHECK, 0, 0) == BST_CHECKED, SendMessage(GetDlgItem(hwnd, 537), BM_GETCHECK, 0, 0) == BST_CHECKED, SendMessage(GetDlgItem(hwnd, 538), BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (SendMessage(GetDlgItem(hwnd, 523), BM_GETCHECK, 0, 0) == BST_CHECKED && GetPlayerGod() == 0)
                {
                    SetPlayerGod(true);
                }
                if (SendMessage(GetDlgItem(hwnd, 525), BM_GETCHECK, 0, 0) == BST_CHECKED && GetWanted() != 0)
                {
                    SetWanted(0);
                    SetWantedCanChange(false);
                }
                if (SendMessage(GetDlgItem(hwnd, 526), BM_GETCHECK, 0, 0) == BST_CHECKED && GetNPCIgnore() != 0x450000)
                {
                    SetNPCIgnore(true);
                }
                if (SendMessage(GetDlgItem(hwnd, 527), BM_GETCHECK, 0, 0) == BST_CHECKED && GetRagDoll() == 0x20)
                {
                    NoRagDoll(true);
                }
                if (SendMessage(GetDlgItem(hwnd, 539), BM_GETCHECK, 0, 0) == BST_CHECKED && GetPlayerMaxHealth() != 0)
                {
                    UndeadOffRadar(true);
                }
                if (SendMessage(GetDlgItem(hwnd, 541), BM_GETCHECK, 0, 0) == BST_CHECKED && (float)SendMessage(hwndSwimSpeed, TBM_GETPOS, 0, 0) != GetSwimSpeed())
                {
                    SetSwimSpeed((float)SendMessage(hwndSwimSpeed, TBM_GETPOS, 0, 0));
                }
                if (SendMessage(GetDlgItem(hwnd, 542), BM_GETCHECK, 0, 0) == BST_CHECKED && (float)SendMessage(hwndRunSpeed, TBM_GETPOS, 0, 0) != GetRunSpeed())
                {
                    SetRunSpeed((float)SendMessage(hwndRunSpeed, TBM_GETPOS, 0, 0));
                }
            }
        }
        Sleep(1);
    }
}
VOID MakeFrameFlags(bool superJump, bool explosiveMelee, bool fireAmmo, bool explosiveAmmo)
{
    if (!superJump && !explosiveMelee && !fireAmmo && !explosiveAmmo)
        return;
    DWORD flagValue = 0;
    if (superJump)
        flagValue += 64;
    if (explosiveMelee)
        flagValue += 32;
    if (fireAmmo)
        flagValue += 16;
    if (explosiveAmmo)
        flagValue += 8;
    SetFrameFlags(flagValue);
}


VOID SetFrameFlags(DWORD value) {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_FRAMEFLAGS };
    WriteMemory(hProcess, value, address);
}

DWORD GetFrameFlags() {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_FRAMEFLAGS };
    static DWORD value;
    ReadMemory(hProcess, &value, address);
    return value;
}
BYTE IsIntVehicle() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_PLAYER_INVEHICLE };
    static byte value;
    ReadMemory(hProcess, &value, address);
    return  !((value >> 4) & 1);
}

FLOAT GetPlayerMaxHealth() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_HEALTH_MAX };// ����ֵ���ֵ
    static float	play_max_health;
    ReadMemory(hProcess, &play_max_health, address);
    return play_max_health - 100;
}
void SetPlayerMaxHealth(float value) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_HEALTH_MAX };// ����ֵ���ֵ
    WriteMemory(hProcess, value + 100, address);
}
FLOAT GetPlayerHealth() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_HEALTH };// ��ǰ����ֵ
    static float	play_health;
    ReadMemory(hProcess, &play_health, address);
    return play_health;
}
void SetPlayerHealth(float value) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_HEALTH };// ��ǰ����ֵ
    WriteMemory(hProcess, value + 100, address);
}
FLOAT GetPlayerArmor() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_PLAYER_ARMOR };// �����»���ֵ
    static float	play_armor;
    ReadMemory(hProcess, &play_armor, address);
    return play_armor;
}
void SetPlayerArmor(float value) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_PLAYER_ARMOR };// �����»���ֵ
    WriteMemory(hProcess, value / 2, address);
}
void SetWaterProof(DWORD option) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_PLAYER_WATER_PROOF };// ��ˮ DWORD; +0x1000000 = on
    //WriteMemory(hProcess, (float)(armor / 2), address);
}

DWORD GetWanted() {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_WANTED };//ͨ���ȼ�
    static DWORD value;
    ReadMemory(hProcess, &value, address);
    return value;
}
void SetWanted(DWORD value) {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_WANTED };//ͨ���ȼ�
    WriteMemory(hProcess, value, address);
}

FLOAT GetWantedCanChange() {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_WANTED_CAN_CHANGE };//�ܷ�ͨ��
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}
void SetWantedCanChange(bool option) {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_WANTED_CAN_CHANGE };//�ܷ�ͨ��
    float value = option == true ? 1 : 0;
    WriteMemory(hProcess, value, address);
}
void SetPlayerGod(bool option) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_GOD };// �޵�ģʽ
    byte value = option == true ? 1 : 0;
    WriteMemory(hProcess, value, address);
}
BYTE GetPlayerGod() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_GOD };// �޵�ģʽ
    static byte value;
    ReadMemory(hProcess, &value, address);
    return value;
}
void SetPlayerPos(v3 player_pos) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_POS };
    WriteMemory(hProcess, player_pos, address);
    DWORD64 address2[] = { PLAYER_BASE + OFFSET_ENTITY_POSBASE,OFFSET_ENTITY_POSBASE_POS };
    WriteMemory(hProcess, player_pos, address2);
}
v3 GetPlayerPos() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_ENTITY_POS };
    v3 player_pos_current;
    ReadMemory(hProcess, &player_pos_current, address);
    return player_pos_current;
}

void SetSwimSpeed(float swim_speed) {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_SWIM_SPD };//��Ӿ�ٶ� float def.1
    WriteMemory(hProcess, swim_speed, address);
}
FLOAT GetSwimSpeed()
{
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_SWIM_SPD };//��Ӿ�ٶ� float def.1
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}
void SetRunSpeed(float run_speed) {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_RUN_SPD };//�����ٶ� float def.1
    WriteMemory(hProcess, run_speed, address);
}
FLOAT GetRunSpeed()
{
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_RUN_SPD };//�����ٶ� float def.1
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}
void SetNPCIgnore(bool option) {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_NPC_IGNORE };// NPC������� DWORD; everyone = 0x450000;
    DWORD value = option == true ? 0x450000 : 0x000000;
    WriteMemory(hProcess, value, address);
}

DWORD GetNPCIgnore() {
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_NPC_IGNORE };// NPC������� DWORD; everyone = 0x450000;
    static DWORD value;
    ReadMemory(hProcess, &value, address);
    return value;
}

BYTE GetRagDoll() {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_PLAYER_RAGDOLL };//������ϵͳ
    static byte value;
    ReadMemory(hProcess, &value, address);
    return value;
}

VOID SetRagDoll(byte value) {
    DWORD64 address[] = { PLAYER_BASE + OFFSET_PLAYER_RAGDOLL };//������ϵͳ
    WriteMemory(hProcess, value, address);
}

VOID NoRagDoll(bool option) {//0x20����  0x00/0x01�ر�
    BYTE value = option == true ? 0x01 : 0x20;
    SetRagDoll(value);
}
VOID SetStamina(float value)
{
    DWORD64 address[] = { PLAYER_INFO + OFFSET_PLAYER_INFO_STAMINA };
    return;
}

DWORD WINAPI ThreadVehicleHack(LPVOID lpParameter)
{
    while (true)
    {
        if (GAME_BASE != 0 && WORLD_BASE != 0 && PLAYER_BASE != 0)
        {
            OutputDebugString(L"Vehicle Hack\n");
            ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_PLAYER_INFO), &PLAYER_INFO, sizeof(DWORD64), 0);
            ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_PLAYER_VEHICLE), &VEHICLE_BASE, sizeof(DWORD64), 0);
            if (PLAYER_INFO != 0 && VEHICLE_BASE != 0)
            {
                if (SendMessage(GetDlgItem(hwnd, 524), BM_GETCHECK, 0, 0) != 0 && GetVehicleGod() == 0)
                {
                    SetVehicleGod(true);
                    //OutputDebugString(L"VehicleGod On\n");
                }
            }
        }
        Sleep(1);
    }
}
//�ؾ��޸�
v3 GetVehiclePos() {
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_ENTITY_POS };
    static v3 vehicle_pos_current;
    ReadMemory(hProcess, &vehicle_pos_current, address);
    return vehicle_pos_current;
}
VOID SetVehiclePos(v3 vehiclePos) {
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_ENTITY_POS };
    WriteMemory(hProcess, vehiclePos, address);
    DWORD64 address1[] = { VEHICLE_BASE + OFFSET_ENTITY_POSBASE,OFFSET_ENTITY_POSBASE_POS };
    WriteMemory(hProcess, vehiclePos, address1);
}
FLOAT GetVehicleHealth() {
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_VEHICLE_HEALTH };// �ؾ�����ֵ
    static float	value;
    ReadMemory(hProcess, &value, address);
    return value;
}
VOID SetVehicleHealth()
{

}

void SetVehicleGod(bool option) {
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_ENTITY_GOD };// �ؾ��޵�
    byte value = option == true ? 1 : 0;
    WriteMemory(hProcess, value, address);
}
BYTE GetVehicleGod() {
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_ENTITY_GOD };// �ؾ��޵�
    static byte value;
    ReadMemory(hProcess, &value, address);
    return value;
}
FLOAT GetRocketRechargeSpeed()
{
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_VEHICLE_RECHARGE_SPEED };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}

void SetRocketRechargeSpeed(float value)
{
    DWORD64 address[] = { VEHICLE_BASE + OFFSET_VEHICLE_RECHARGE_SPEED };
    WriteMemory(hProcess, value, address);
}


DWORD WINAPI ThreadWeaponHack(LPVOID lpParameter)
{
    while (true)
    {
        if (GAME_BASE != 0 && WORLD_BASE != 0 && PLAYER_BASE != 0)
        {
            OutputDebugString(L"Weapon Hack\n");
            ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_WEAPON_MANAGER), &WEAPON_MANAGER, sizeof(DWORD64), 0);
            ReadProcessMemory(hProcess, (LPVOID)(WEAPON_MANAGER + OFFSET_WEAPON_CURRENT), &WEAPON_CURRENT, sizeof(DWORD64), 0);
            if (WEAPON_MANAGER != 0 && WEAPON_CURRENT != 0)
            {
                ReadProcessMemory(hProcess, (LPVOID)(WEAPON_CURRENT + WEAPON_AMMOINFO), &WEAPON_AMMOINFO, sizeof(DWORD64), 0);
                if (SendMessage(GetDlgItem(hwnd, 530), BM_GETCHECK, 0, 0) != 0)
                {
                    InfAmmo(true);
                    //OutputDebugString(L" InfAmmo On\n");
                }
                if (SendMessage(GetDlgItem(hwnd, 531), BM_GETCHECK, 0, 0) != 0)
                {
                    NoReload(true);
                    //OutputDebugString(L" NoReload On\n");
                }

                if (SendMessage(GetDlgItem(hwnd, 532), BM_GETCHECK, 0, 0) != 0 && GetSpread() != 0)
                {
                    NoSpread(true);
                    //OutputDebugString(L" NoSpread On\n");
                }
                if (SendMessage(GetDlgItem(hwnd, 533), BM_GETCHECK, 0, 0) != 0 && GetRecoil() != 0)
                {
                    NoRecoil(true);
                    //OutputDebugString(L" NoRecoil On\n");
                }
                //if (SendMessage(GetDlgItem(hwnd, 534), BM_GETCHECK, 0, 0) != 0)
                //{
                //    QuickReload(true);
                //    //OutputDebugString(L" QuickReload On\n");
                //}
                if (SendMessage(GetDlgItem(hwnd, 540), BM_GETCHECK, 0, 0) != 0 && GetWeaponRange() != 1200)
                {
                    InfWeaponRange(true);
                    //OutputDebugString(L" InfWeaponRange On\n");
                }
            }
        }
        Sleep(1);
    }
}
//�����޸�
VOID InfAmmo(bool option) {
    // DWORD64 address = { GAME_BASE + ADDRESS_AMMO };
    BYTE cur[3] = {};
    BYTE value1[3] = { 0x90, 0x90, 0x90 };
    BYTE value2[3] = { 0x41, 0x2B, 0xD1 };
    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_AMMO), cur, sizeof(cur), 0);
    switch (option == true)
    {
    case true:
        if (cur[0] != value1[0])
            WriteProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_AMMO), value1, sizeof(value1), 0);
        break;
    default:
        if (cur[0] != value2[0])
            WriteProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_AMMO), value2, sizeof(value2), 0);
        break;
    }
}

float GetSpread() {
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_SPREAD };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}
VOID SetSpread(float value) {
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_SPREAD };
    WriteMemory(hProcess, value, address);
}

VOID NoSpread(bool option) {
    switch (option)
    {
    case true:
        SetSpread(0);
        break;
    default:
        OutputDebugString(L"�ָ���ɢ");
        break;
    }
}

float GetRecoil() {
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RECOIL };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}
VOID SetRecoil(float value) {
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RECOIL };
    WriteMemory(hProcess, value, address);
}

VOID NoRecoil(bool option) {
    switch (option)
    {
    case true:
        SetRecoil(0);
        break;
    default:
        OutputDebugString(L"�ָ�������");
        break;
    }
}

FLOAT GetWeaponRange() {
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RANGE };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}

VOID SetWeaponRange(float value) {
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RANGE };
    WriteMemory(hProcess, value, address);
}

VOID InfWeaponRange(bool option) {
    float value = option == true ? 1200 : 100;
    SetWeaponRange(value);
}
VOID UndeadOffRadar(bool option)
{
    float value = option == true ? 0 : 328;
    SetPlayerMaxHealth(value);
}
float GetReloadSpeed()
{
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RELOAD_MULTIPLIER };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}

void SetReloadSpeed(float value)
{
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RELOAD_MULTIPLIER };
    WriteMemory(hProcess, value, address);
}

float GetReloadVehicle()
{
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RELOAD_VEHICLE };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}

void SetReloadVehicle(float value)
{
    DWORD64 address[] = { WEAPON_CURRENT + OFFSET_WEAPON_RELOAD_VEHICLE };
    WriteMemory(hProcess, value, address);
}

VOID QuickReload(bool option) {
    switch (option)
    {
    case true:
        //SetReloadSpeed();
        SetReloadVehicle(0);
        break;
    default:
        OutputDebugString(L"�ָ������ٶ�");
        break;
    }
}

VOID NoReload(bool option)
{
    // DWORD64 address = { GAME_BASE + ADDRESS_AMMO };
    BYTE cur[3] = {};
    BYTE value1[3] = { 0x90, 0x90, 0x90 };
    BYTE value2[3] = { 0x41, 0x2B, 0xC9 };
    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_MAGAZINE), cur, sizeof(cur), 0);
    switch (option == true)
    {
    case true:
        if (cur[0] != value1[0])
            WriteProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_MAGAZINE), value1, sizeof(value1), 0);
        break;
    default:
        if (cur[0] != value2[0])
            WriteProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_MAGAZINE), value2, sizeof(value2), 0);
        break;
    }
}

//�����޸�
void TransportToWayPoint() {
    memset(&target_pos, 0, sizeof(target_pos));
    v3 way_point = GetWayPoint();
    if (way_point.x == 64000.f && way_point.y == 64000.f)
        return;
    target_pos.x = way_point.x;
    target_pos.y = way_point.y;
    target_pos.z = -225.f;// -225.f way_point.z+1.f

    if (IsIntVehicle())
    {
        SetVehiclePos(target_pos);
        return;
    }
    SetPlayerPos(target_pos);
    return;
}
void TransportToObjectivePoint() {
    memset(&target_pos,0,sizeof(target_pos));
    v3 objective_point = GetObjectivePoint();
    if (objective_point.x == 64000.f && objective_point.y == 64000.f && objective_point.z == 64000.f)
        return;
    target_pos.x = objective_point.x;
    target_pos.y = objective_point.y;
    target_pos.z = -225.f;//objective_point.z + 1.f
    if (IsIntVehicle())
    {
        SetVehiclePos(target_pos);
        return;
    }
    SetPlayerPos(target_pos);
    return;
}

v3 GetObjectivePoint() {
    static v3 objective_point;
    static DWORD64 n;
    static DWORD objColor, objIcon;

    static int ColorYellowMission = 66;
    static int ColorYellow = 5;
    static int ColorWhite = 0;
    static int ColorGreen = 2;
    static int SpriteCrateDrop = 306;
    static int Spriaddressandard = 1;
    static int SpriteRaceFinish = 38;

    for (size_t i = 2000; i > 1; i--)
    {

        DWORD64 address0[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) };
        DWORD64 address1[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x40 };
        DWORD64 address2[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x48 };
        DWORD64 address3[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x10 };

        ReadMemory(hProcess, &n, address0);
        ReadMemory(hProcess, &objIcon, address1);
        ReadMemory(hProcess, &objColor, address2);
        if (n > 0 && objColor == ColorYellowMission || 
            objIcon == Spriaddressandard && objColor == ColorYellow ||
            objIcon == Spriaddressandard&& objColor == ColorWhite ||
            objIcon == SpriteRaceFinish&& objColor == ColorGreen ||
            objIcon == Spriaddressandard && objColor == SpriteCrateDrop
            )
        {
            ReadMemory(hProcess, &objective_point, address3);
            break;
        }
    }
    return objective_point;
}

v3 GetWayPoint() {
    static v3 way_point;
    static DWORD64 n = 0;
    static DWORD objIcon, objColor;
    for (size_t i = 2000; i > 1; i--)
    {
        DWORD64 address0[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) };
        DWORD64 address1[] = { GAME_BASE + ADDRESS_BLIP + (i * 8),0x40 };
        DWORD64 address2[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x48 };
        DWORD64 address3[] = { GAME_BASE + ADDRESS_BLIP + (i * 8) ,0x10 };

        ReadMemory(hProcess, &n, address0);
        ReadMemory(hProcess, &objIcon, address1);
        ReadMemory(hProcess, &objColor, address2);
        if (n > 0 && objIcon == 8 && objColor == 84)
        {
            ReadMemory(hProcess, &way_point, address3);
            break;
        }
    }
    return way_point;
}

FLOAT GetRpMult()
{
    DWORD64 address[] = { TUNABLE_BASE + OFFSET_TUNABLE_RP_MULTIPLIER };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}

VOID SetRpMult(float value)
{
    DWORD64 address[] = { TUNABLE_BASE + OFFSET_TUNABLE_RP_MULTIPLIER };
    WriteMemory(hProcess, value, address);
}


FLOAT GetApMult()
{
    DWORD64 address[] = { TUNABLE_BASE + OFFSET_TUNABLE_AP_MULTIPLIER };
    static float value;
    ReadMemory(hProcess, &value, address);
    return value;
}

void SetApMult(float value)
{
    DWORD64 address[] = { TUNABLE_BASE + OFFSET_TUNABLE_AP_MULTIPLIER };
    WriteMemory(hProcess, value, address);
}


//
BYTE InitModifier()
{
    BYTE r = 0;
    GAME_BASE = GetModuleAddress("GTA5.exe");
    DWORD64 STEAM_BASE = GetModuleAddress("steam_api64.dll");
    if (STEAM_BASE != 0)
   {
		//Steam
		ADDRESS_WORLD = 0x24CD000;
		ADDRESS_BLIP = 0x1F524F0;
		ADDRESS_AMMO = 0x101B445;
		ADDRESS_MAGAZINE = 0x101B400;
		ADDRESS_TRIGGER = 0x1F63020;
		ADDRESS_GLOBAL = 0x2D765E0;
		ADDRESS_PLAYER_LIST = 0x1D98AE8;
		ADDRESS_REPLAY_INTERFACE = 0x1EC7E40;
		ADDRESS_UNK_MODEL = 0x24F0260;
	}
	else
	{
		//Epic
		ADDRESS_WORLD = 0x24E6D90;
		ADDRESS_BLIP = 0x1F6EF80;
		ADDRESS_AMMO = 0x102F8E9;
		ADDRESS_MAGAZINE = 0x102F8A4;
		ADDRESS_TRIGGER = 0x1F7CBE0;
		ADDRESS_GLOBAL = 0x2D9C4A0;
		ADDRESS_PLAYER_LIST = 0x1DB2648;
		ADDRESS_REPLAY_INTERFACE = 0x1EE18A8;
		ADDRESS_UNK_MODEL = 0x250A280;
	}
    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_WORLD), &WORLD_BASE, sizeof(DWORD64), 0);
    if (WORLD_BASE == 0)
        return INITPTR_INVALID_WORLD;
    //ReadProcessMemory(hProcess, (LPVOID)(WORLD_BASE + OFFSET_PLAYER), &PLAYER_BASE, sizeof(DWORD64), 0);
    //if (PLAYER_BASE == 0)
    //    return INITPTR_INVALID_PLAYER;

    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_GLOBAL), &GLOBAL_BASE, sizeof(DWORD64), 0);
    if (GLOBAL_BASE == 0)
        return INITPTR_INVALID_GLOBAL;
    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_TUNABLE), &TUNABLE_BASE, sizeof(DWORD64), 0);
    if (TUNABLE_BASE == 0)
        return INITPTR_INVALID_TUNABLE;
    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_UNK_MODEL), &UNKMODEL_BASE, sizeof(DWORD64), 0);
    if (UNKMODEL_BASE == 0)
        return INITPTR_INVALID_UNK_MODEL;
    // g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_dwpUnkModelBase + 0, &m_unkModel.m_dwpUnkModelStruct);
    ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_REPLAY_INTERFACE), &REPLAYINTERFACE_BASE, sizeof(DWORD64), 0);
    if (REPLAYINTERFACE_BASE == 0)
        return INITPTR_INVALID_REPLAY_INTERFACE;
    /* m_replayInterface.m_dwpPedInterface = m_dwpReplayInterfaceBase;
     g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_dwpReplayInterfaceBase + OFFSET_REPLAY_PED_INTERFACE, &m_replayInterface.m_dwpPedInterface);
     g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_replayInterface.m_dwpPedInterface + OFFSET_INTERFACE_LIST, &m_replayInterface.m_dwpPedList);
     g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_dwpReplayInterfaceBase + OFFSET_REPLAY_PICKUP_INTERFACE, &m_replayInterface.m_dwpPickUpInterface);
     g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_replayInterface.m_dwpPickUpInterface + OFFSET_INTERFACE_LIST, &m_replayInterface.m_dwpPickUpList);*/
    
    /*ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_PLAYER_INFO), &PLAYER_INFO, sizeof(DWORD64), 0);
    ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_PLAYER_VEHICLE), &VEHICLE_BASE, sizeof(DWORD64), 0);*/
    //if (VEHICLE_BASE == 0)
    //    r |= INITPTR_INVALID_VEHICLE;
    /* else
     {
         m_vehicle.m_dwpBase = m_dwpVehicleBase;
         g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_dwpVehicleBase + OFFSET_ENTITY_POSBASE, &m_vehicle.m_dwpPosBase);
         g_pMemMan->readMem<DWORD_PTR>((DWORD_PTR)m_dwpVehicleBase + OFFSET_VEHICLE_HANDLING, &m_vehicle.m_handlingCur.m_dwpHandling);
     }*/
    ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_ENTITY_ATTACKER), &ATTACKER_BASE, sizeof(DWORD64), 0);
    //ReadProcessMemory(hProcess, (LPVOID)(GAME_BASE + ADDRESS_WEAPON ), &WEAPON_BASE, sizeof(DWORD64), 0);

    //ReadProcessMemory(hProcess, (LPVOID)(PLAYER_BASE + OFFSET_WEAPON_MANAGER), &WEAPON_MANAGER, sizeof(DWORD64), 0);
    //ReadProcessMemory(hProcess, (LPVOID)(WEAPON_MANAGER + OFFSET_WEAPON_CURRENT), &WEAPON_CURRENT, sizeof(DWORD64), 0);
    //if (WEAPON_MANAGER == 0 || WEAPON_CURRENT == 0)// || WEAPON_BASE == 0
    //    r |= INITPTR_INVALID_WEAPON;
    //else
    //{
    //    ReadProcessMemory(hProcess, (LPVOID)(WEAPON_CURRENT + WEAPON_AMMOINFO), &WEAPON_AMMOINFO, sizeof(DWORD64), 0);
    //}
    return r;
}

void RaiseToDebug()//��ȡ����Ȩ��
{
    HANDLE hToken;
    HANDLE hProcesscess = GetCurrentProcess();  // ��ȡ��ǰ���̾��

    // �򿪵�ǰ���̵�Token������һ��Ȩ�����ƣ��ڶ�������������TOKEN_ALL_ACCESS
    if (OpenProcessToken(hProcesscess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        TOKEN_PRIVILEGES token_privileges;
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &token_privileges.Privileges[0].Luid))
        {
            token_privileges.PrivilegeCount = 1;
            token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            //֪ͨϵͳ�޸Ľ���Ȩ��
            BOOL bREt = AdjustTokenPrivileges(hToken, FALSE, &token_privileges, 0, NULL, 0);
        }
        CloseHandle(hToken);
    }
}
void AttachProcess() {//�򿪽���,��ȡ���̾��

    DWORD Process_ID = (DWORD)0;//Ŀ�����ID
    hwndTarget = FindWindow(NULL, TARGET_PROGRAM_NAME);
    GetWindowThreadProcessId(hwndTarget, &Process_ID);//Ŀ�����ID
    if (Process_ID == (DWORD)0) {	//�ж�Ŀ�����ID�Ƿ��ȡ�ɹ�
        //SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)TEXT("�޷���ȡ����ID"));
    }
    else
    {
        hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, Process_ID);//�򿪽��̶��󣬲���ȡ���̾��			
        if (hProcess == 0) {
            //SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)TEXT("�򿪽���ʧ��"));
        }
    }
}

DWORD64 GetModuleAddress(const char* moduleName) {//��ȡģ����ػ�ַ
    HMODULE hModule[0xff] = { 0 };
    DWORD dwRet = 0;
    int num = 0;
    int bRet = EnumProcessModulesEx(hProcess, (HMODULE*)(hModule), sizeof(hModule), &dwRet, LIST_MODULES_ALL);
    if (bRet == 0) {
        //printf("EnumProcessModules");
    }
    // ��ģ�����
    num = dwRet / sizeof(HMODULE);

    char lpBaseName[0xff];
    for (int i = 0; i < num; i++) {
        GetModuleBaseNameA(hProcess, hModule[i], lpBaseName, sizeof(lpBaseName));
        if (strcmp(lpBaseName, moduleName) == 0)
        {
            return  (DWORD64)hModule[i];
        }
    }
    return 0;
}

template<typename T, typename T2> void WriteMemoryEx(HANDLE hProcess, T value, T2 address[], DWORD length)
{
    T2 result = NULL;
    T2 addr = NULL;
    for (int i = 0; i < length; i++)
    {
        addr = result + address[i];
        ReadProcessMemory(hProcess, (LPVOID)addr, &result, sizeof(T2), 0);
    }
    if (addr != NULL)
    {
        WriteProcessMemory(hProcess, (LPVOID)addr, &value, sizeof(T), 0);
    }
}
template<typename T, typename T2> void ReadMemoryEx(HANDLE hProcess, T* result, T2 address[], DWORD length)
{
    T2 value = NULL;
    T2 addr = NULL;
    for (int i = 0; i < length; i++)
    {
        addr = value + address[i];
        ReadProcessMemory(hProcess, (LPVOID)addr, &value, sizeof(T2), 0);
    }
    memcpy(result, &value, sizeof(T));
}