#include <windows.h>
#include <commctrl.h> 
#include <cstdlib>
#include "resource.h"
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HFONT g_hTitleFont = NULL;

using namespace std;

string DetectBootOSInfo() {
    char tempPath[MAX_PATH];
    char tempFile[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath); 
    sprintf_s(tempFile, "%sbcdinfo.txt", tempPath);

    char cmd[MAX_PATH * 2];
    sprintf_s(cmd, "cmd.exe /c bcdedit /enum osloader > \"%s\"", tempFile);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 2000); 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    ifstream file(tempFile);
    if (!file.is_open()) {
        return "Windows Operating System on (C:) Local Disk"; 
    }

    string line;
    string osName = "Unknown Windows OS";
    string driveLetter = "C";
    while (getline(file, line)) {
    
        if (line.find("device") != string::npos) {
            size_t partPos = line.find("partition=");
            if (partPos != string::npos) {
                driveLetter = line.substr(partPos + 10, 1); 
            }
        }
        
        if (line.find("description") != string::npos) {
            size_t descPos = line.find("description");
            string val = line.substr(descPos + 11);

            
            size_t first = val.find_first_not_of(" \t\r\n");
            size_t last = val.find_last_not_of(" \t\r\n");
            if (first != string::npos && last != string::npos) {
                osName = val.substr(first, (last - first + 1));
            }
        }
    }
    file.close();
    DeleteFileA(tempFile); 

    return osName + " on (" + driveLetter + ":) Local Disk";
}

//Avoid cmd to pop up
void RunCommand(const char* cmd) {

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmdBuffer[MAX_PATH * 2];
    strcpy_s(cmdBuffer, cmd);

    if (CreateProcessA(
        NULL,
        cmdBuffer,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

INT_PTR CALLBACK MyDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        g_hTitleFont = CreateFont(
            18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"System"
        );

        string osDetectedInfo = DetectBootOSInfo();
        string finalDisplayString = "Operating system: " + osDetectedInfo;
        SetDlgItemTextA(hDlg, IDC_OS_INFO_TEXT, finalDisplayString.c_str());

        HWND hTitleText = GetDlgItem(hDlg, IDC_TITLE_TEXT);
        if (hTitleText) {
            SendMessage(hTitleText, WM_SETFONT, (WPARAM)g_hTitleFont, TRUE);
        }
        
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);

        HICON hSmallIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

        HICON hBigIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

        if (hSmallIcon) {
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmallIcon);
        }
        if (hBigIcon) {
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hBigIcon);
        }

        return (INT_PTR)TRUE;
    }

    case WM_NOTIFY:
    {

        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->code == NM_CLICK || pnmh->code == NM_RETURN)
        {
            switch (pnmh->idFrom)
            {
            case IDC_SYSLINK2: // Startup Repair
                RunCommand("X:\\sources\\recovery\\StartRep.exe");
                return (INT_PTR)TRUE; 

            case IDC_SYSLINK3: // System Restore
                RunCommand("X:\\windows\\system32\\rstrui.exe");
                return (INT_PTR)TRUE;

            case IDC_SYSLINK4: // System Image Recovery
                RunCommand("X:\\windows\\system32\\bmrui.exe");
                return (INT_PTR)TRUE;

            case IDC_SYSLINK5: // Windows Memory Diagnostic
                RunCommand("X:\\windows\\system32\\mdsched.exe");
                return (INT_PTR)TRUE;

            case IDC_SYSLINK6: // Command Prompt
                WinExec("cmd.exe", SW_SHOW);
                return (INT_PTR)TRUE;
            }
        }
        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_BUTTON1: // Shut Down
            RunCommand("wpeutil shutdown");
            EndDialog(hDlg, wmId);
            return (INT_PTR)TRUE;

        case IDC_BUTTON2: // Restart
            RunCommand("wpeutil reboot");
            EndDialog(hDlg, wmId);
            return (INT_PTR)TRUE;

        case IDCANCEL: 
            EndDialog(hDlg, wmId);
            return (INT_PTR)TRUE;
        }
        break;
    }

    case WM_DESTROY:
        if (g_hTitleFont) {
            DeleteObject(g_hTitleFont);
        }
        break;
    }
    return (INT_PTR)FALSE;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LINK_CLASS;
    InitCommonControlsEx(&icex);

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MyDialogProc);

    return 0;
}