#include <windows.h>
#include <commctrl.h> 
#include <cstdlib>
#include "resource.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HFONT g_hTitleFont = NULL;

#ifndef VARIABLE_ATTRIBUTE_NON_VOLATILE
#define VARIABLE_ATTRIBUTE_NON_VOLATILE       0x00000001
#endif
#ifndef VARIABLE_ATTRIBUTE_BOOTSERVICE_ACCESS
#define VARIABLE_ATTRIBUTE_BOOTSERVICE_ACCESS 0x00000002
#endif
#ifndef VARIABLE_ATTRIBUTE_RUNTIME_ACCESS
#define VARIABLE_ATTRIBUTE_RUNTIME_ACCESS     0x00000004
#endif

using namespace std;

struct KeyboardInfo {
    wstring name;
    wstring id;
};
vector<KeyboardInfo> g_keyboardList;

RECT g_lastDlgRect = { 0 };
string g_selectedOSInfo = "Unknown";

void CenterWindowOnScreen(HWND hWnd) {
    RECT rcDlg, rcScreen;
    GetWindowRect(hWnd, &rcDlg);

    int dlgWidth = rcDlg.right - rcDlg.left;
    int dlgHeight = rcDlg.bottom - rcDlg.top;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int x = (screenWidth - dlgWidth) / 2;
    int y = (screenHeight - dlgHeight) / 2;

    SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

INT_PTR CALLBACK LangDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);
        HICON hSmallIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        HICON hBigIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

        if (hSmallIcon) {
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmallIcon);
        }
        if (hBigIcon) {
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hBigIcon);
        }

        HWND hLangCombo = GetDlgItem(hDlg, IDC_COMBO1);
        if (hLangCombo) {
            SendMessage(hLangCombo, CB_ADDSTRING, 0, (LPARAM)L"English (United States)");
            SendMessage(hLangCombo, CB_SETCURSEL, 0, 0); 
        }

        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO2);
        if (!hCombo) return (INT_PTR)TRUE;

        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            WCHAR subKeyName[256];
            DWORD subKeySize = 256;
            DWORD index = 0;

            g_keyboardList.clear();

            while (RegEnumKeyExW(hKey, index, subKeyName, &subKeySize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hSubKey;
                wstring fullPath = L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\" + wstring(subKeyName);

                if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, fullPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    WCHAR layoutText[256];
                    DWORD layoutTextSize = sizeof(layoutText);

                    if (RegQueryValueExW(hSubKey, L"Layout Text", NULL, NULL, (LPBYTE)layoutText, &layoutTextSize) == ERROR_SUCCESS) {
                        KeyboardInfo ki;
                        ki.name = layoutText;
                        ki.id = subKeyName;
                        g_keyboardList.push_back(ki);
                    }
                    RegCloseKey(hSubKey);
                }
                subKeySize = 256;
                index++;
            }
            RegCloseKey(hKey);
        }

        LRESULT usKeyboardIdx = 0;
        for (size_t i = 0; i < g_keyboardList.size(); ++i) {
            wstring displayName = g_keyboardList[i].name;
            LRESULT idx = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)displayName.c_str());
            SendMessage(hCombo, CB_SETITEMDATA, idx, (LPARAM)i);
        }

        LRESULT totalCount = SendMessage(hCombo, CB_GETCOUNT, 0, 0);

        for (LRESULT idx = 0; idx < totalCount; ++idx) {
            LRESULT dataIdx = SendMessage(hCombo, CB_GETITEMDATA, idx, 0);

            if (dataIdx != CB_ERR && dataIdx >= 0 && dataIdx < (LRESULT)g_keyboardList.size()) {
                if (g_keyboardList[dataIdx].id == L"00000409") {
                    usKeyboardIdx = idx; 
                    break;           
                }
            }
        }
        SendMessage(hCombo, CB_SETCURSEL, usKeyboardIdx, 0);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDOK)
        {
            HWND hCombo = GetDlgItem(hDlg, IDC_COMBO2);
            LRESULT selIdx = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            if (selIdx != CB_ERR) {
             
                LRESULT dataIdx = SendMessage(hCombo, CB_GETITEMDATA, selIdx, 0);

                
                if (dataIdx >= 0 && dataIdx < (LRESULT)g_keyboardList.size()) {
                    wstring targetID = g_keyboardList[dataIdx].id;

                    
                    HKL hkl = LoadKeyboardLayoutW(targetID.c_str(), KLF_ACTIVATE);
                    if (hkl != NULL) {
                        ActivateKeyboardLayout(hkl, 0);

                        PostMessageW(HWND_BROADCAST, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hkl);
                    }
                }
            }
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (wmId == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    case WM_DESTROY:
    {
        GetWindowRect(hDlg, &g_lastDlgRect);
        break;
    }

    }

    CenterWindowOnScreen(hDlg);
    return (INT_PTR)FALSE;
}

struct OSInfo {
    wstring name;        
    wstring driveLetter; 
    wstring sizeMB;      
};

wstring GetPartitionSizeInMB(wchar_t driveLetter) {
    wchar_t rootPath[] = { driveLetter, L':', L'\\', L'\0' };
    ULARGE_INTEGER freeBytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExW(rootPath, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        ULONGLONG totalMB = totalNumberOfBytes.QuadPart / (1024 * 1024);
        return to_wstring(totalMB) + L" MB";
    }
    return L"Unknown";
}


vector<OSInfo> ScanInstalledOSList() {
    vector<OSInfo> osList;

    wchar_t tempPath[MAX_PATH];
    wchar_t tempFile[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    swprintf_s(tempFile, L"%sbcdinfo.txt", tempPath);

    wchar_t cmd[MAX_PATH * 2];
    swprintf_s(cmd, L"cmd.exe /c bcdedit /enum osloader > \"%s\"", tempFile);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 2000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    wifstream file(tempFile);
    if (!file.is_open()) return osList;

    wstring line;
    OSInfo currentOS;
    bool hasOS = false;

    while (getline(file, line)) {
        size_t partPos = line.find(L"partition=");
        if (partPos != wstring::npos && partPos + 10 < line.length()) {
            wchar_t letter = line[partPos + 10];
            currentOS.driveLetter = L"(" + wstring(1, towupper(letter)) + L":) Local Disk";
            currentOS.sizeMB = GetPartitionSizeInMB(towupper(letter));
        }

        size_t descPos = line.find(L"description");
        if (descPos != wstring::npos) {
            wstring val = line.substr(descPos + 11);
            size_t first = val.find_first_not_of(L" \t\r\n");
            size_t last = val.find_last_not_of(L" \t\r\n");
            if (first != wstring::npos && last != wstring::npos) {
                currentOS.name = val.substr(first, (last - first + 1));
                hasOS = true;
            }
        }

        if (line.empty() && hasOS) {
            osList.push_back(currentOS);
            currentOS = OSInfo();
            hasOS = false;
        }
    }

    if (hasOS) {
        osList.push_back(currentOS);
    }

    file.close();
    DeleteFileW(tempFile);
    return osList;
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

void UpdateNextButtonState(HWND hDlg) {
    BOOL r1 = (IsDlgButtonChecked(hDlg, IDC_RADIO1) == BST_CHECKED);
    BOOL r2 = (IsDlgButtonChecked(hDlg, IDC_RADIO2) == BST_CHECKED);
    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), r1 || r2);
}

INT_PTR CALLBACK OSListDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HWND hList = GetDlgItem(hDlg, IDC_LIST3);
        if (hList) {
            ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

            LVCOLUMN lvc = { 0 };
            lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

            lvc.iSubItem = 0; lvc.pszText = (LPTSTR)TEXT("Operating System"); lvc.cx = 140;
            ListView_InsertColumn(hList, 0, &lvc);

            lvc.iSubItem = 1; lvc.pszText = (LPTSTR)TEXT("Partition Size"); lvc.cx = 90;
            ListView_InsertColumn(hList, 1, &lvc);

            lvc.iSubItem = 2; lvc.pszText = (LPTSTR)TEXT("Location"); lvc.cx = 110;
            ListView_InsertColumn(hList, 2, &lvc);

            vector<OSInfo> detectedOSList = ScanInstalledOSList();

            for (size_t i = 0; i < detectedOSList.size(); ++i) {
                LVITEM lvi = { 0 };
                lvi.mask = LVIF_TEXT;
                lvi.iItem = (int)i;
                lvi.iSubItem = 0;
                lvi.pszText = (LPTSTR)detectedOSList[i].name.c_str();
                ListView_InsertItem(hList, &lvi);

                ListView_SetItemText(hList, (int)i, 1, (LPTSTR)detectedOSList[i].sizeMB.c_str());

                ListView_SetItemText(hList, (int)i, 2, (LPTSTR)detectedOSList[i].driveLetter.c_str());
                //Data of OS
            }
        }

        if (ListView_GetItemCount(hList) == 0) {
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), FALSE);
        }

        UpdateNextButtonState(hDlg);

        CenterWindowOnScreen(hDlg);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        if (wmId == IDC_RADIO1 || wmId == IDC_RADIO2)
        {
            UpdateNextButtonState(hDlg);
            return (INT_PTR)TRUE;
        }

        if (wmId == IDC_BUTTON2 || wmId == IDOK) //Selection
        {
            HWND hList = GetDlgItem(hDlg, IDC_LIST3);

            int selectedIdx = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (selectedIdx == -1) selectedIdx = 0; 

            if (ListView_GetItemCount(hList) > 0) {
                wchar_t osName[256] = { 0 };
                wchar_t osLoc[256] = { 0 };

                ListView_GetItemText(hList, selectedIdx, 0, osName, 256);
                ListView_GetItemText(hList, selectedIdx, 2, osLoc, 256);

                wstring wFull = wstring(osName) + L" on " + wstring(osLoc);
                g_selectedOSInfo = string(wFull.begin(), wFull.end());
            }

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (wmId == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    case WM_DESTROY:
    {
        GetWindowRect(hDlg, &g_lastDlgRect);
        break;
    }
    }

    return (INT_PTR)FALSE;
}

#define OS_INDICATIONS_GUID L"{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}"
#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI 0x0000000000000001ULL

static BOOL EnableSystemEnvironmentPrivilege(void) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;

    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    if (!LookupPrivilegeValueW(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &tp.Privileges[0].Luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL ok = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    DWORD err = GetLastError();
    CloseHandle(hToken);
    return ok && (err == ERROR_SUCCESS);
}

static BOOL SetBootToFirmwareUI(void) {
    if (!EnableSystemEnvironmentPrivilege()) {
        return FALSE;
    }

    DWORD64 value = 0;
    GetFirmwareEnvironmentVariableExW(
        L"OsIndications", OS_INDICATIONS_GUID,
        &value, sizeof(value), NULL);

    value |= EFI_OS_INDICATIONS_BOOT_TO_FW_UI;

    DWORD attributes = VARIABLE_ATTRIBUTE_NON_VOLATILE |
        VARIABLE_ATTRIBUTE_BOOTSERVICE_ACCESS |
        VARIABLE_ATTRIBUTE_RUNTIME_ACCESS;

    return SetFirmwareEnvironmentVariableExW(
        L"OsIndications", OS_INDICATIONS_GUID,
        &value, sizeof(value), attributes);
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

        string osDetectedInfo = g_selectedOSInfo;
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

        CenterWindowOnScreen(hDlg);

        return (INT_PTR)TRUE;
    }

    case WM_NOTIFY:
    {
        
        LPNMHDR pnmh = (LPNMHDR)lParam;

        if (pnmh->idFrom == IDC_SPLIT1 && pnmh->code == BCN_DROPDOWN)
        {
            //Other boot options
            HMENU hPopupMenu = CreatePopupMenu();
            AppendMenu(hPopupMenu, MF_STRING, 1001, L"Startup Settings");
            AppendMenu(hPopupMenu, MF_STRING, 1002, L"UEFI Firmware Settings");
            AppendMenu(hPopupMenu, MF_SEPARATOR, 1003, L"");
            AppendMenu(hPopupMenu, MF_STRING, 1004, L"Change Boot Defaults");

         
            NMBCDROPDOWN* pDropDown = (NMBCDROPDOWN*)lParam;
            POINT pt = { pDropDown->rcButton.left, pDropDown->rcButton.bottom };
            ClientToScreen(pnmh->hwndFrom, &pt);
           
            TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hDlg, NULL);
           
            DestroyMenu(hPopupMenu);
            return (INT_PTR)TRUE;
        }

        if (pnmh->code == NM_CLICK || pnmh->code == NM_RETURN)
        {
            switch (pnmh->idFrom)
            {
            case IDC_SYSLINK1: // Change language
            case IDC_SYSLINK7:
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG3), hDlg, LangDlgProc);
                return (INT_PTR)TRUE;

            case IDC_SYSLINK2: // Startup Repair
                RunCommand("X:\\sources\\recovery\\StartRep.exe");
                return (INT_PTR)TRUE; 

            case IDC_SYSLINK3: // System Restore
            {
                char driveLetter = 'C';

                size_t pos = g_selectedOSInfo.find('(');
                if (pos != string::npos && pos + 1 < g_selectedOSInfo.length()) {
                    driveLetter = g_selectedOSInfo[pos + 1];
                }

                string cmd = "X:\\windows\\system32\\rstrui.exe /offline:" + string(1, driveLetter) + ":\\Windows";
                RunCommand(cmd.c_str());
                return (INT_PTR)TRUE;
            }

            case IDC_SYSLINK4: // System Image Recovery
                RunCommand("X:\\windows\\system32\\bmrui.exe");
                return (INT_PTR)TRUE;

            case IDC_SYSLINK5: // Windows Memory Diagnostic
                RunCommand("X:\\windows\\system32\\mdsched.exe");
                return (INT_PTR)TRUE;

            case IDC_SYSLINK6: // Command Prompt
                WinExec("cmd.exe", SW_SHOW);
                return (INT_PTR)TRUE;

            case IDC_SYSLINK8:
                RunCommand("C:\\windows\\system32\\bootim.exe");
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

        case IDC_SPLIT1: // Restart
            RunCommand("wpeutil reboot");
            EndDialog(hDlg, wmId);
            return (INT_PTR)TRUE;

        case 1001:
            
            RunCommand("bcdedit /set {default} onetimeadvancedoptions on");

            RunCommand("wpeutil reboot");
            EndDialog(hDlg, wmId);
            return (INT_PTR)TRUE;

        case 1002:

            if (SetBootToFirmwareUI()) {
                RunCommand("wpeutil reboot");
                EndDialog(hDlg, wmId);
            }
            else {
                MessageBoxA(hDlg,
                    "Cannot automatically enter BIOS settings."
                    "Please restart normally instead.",
                    "Error", MB_OK | MB_ICONWARNING);
            }

            EndDialog(hDlg, wmId);
            return (INT_PTR)TRUE;

        case 1004:
        {
            RunCommand("C:\\Windows\\system32\\SystemPropertiesAdvanced.exe");

            Sleep(100);

            HWND hwndParent = FindWindowA(NULL, "System Properties");

            if (hwndParent) {
                SetForegroundWindow(hwndParent);
                SetActiveWindow(hwndParent);
                keybd_event(VK_MENU, 0, 0, 0);
                keybd_event('T', 0, 0, 0);
                keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
            }

            return (INT_PTR)TRUE;
        }

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


// Sequence of opening different windows
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
    icex.dwICC = ICC_LINK_CLASS | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    INT_PTR langRet = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), NULL, LangDlgProc);

    if (langRet == IDOK) {
        INT_PTR osListRet = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG4), NULL, OSListDlgProc);

        if (osListRet == IDOK) {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MyDialogProc);
        }
    }

    return 0;
}