#include <iostream>
#include <Windows.h>
#include <thread>
#include <list>

#include <commctrl.h>
#include <commoncontrols.h>
#include <dpa_dsa.h>
#include <prsht.h>
#include <richedit.h>
#include <richole.h>
#include <shlobj.h>
#include <textserv.h>
#include <tom.h>
#include <uxtheme.h>
#include <windowsx.h>
#include <winuser.h>

#include "resource.h"

#ifdef UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

using namespace std;

struct Win {
	HWND hwnd;
	wstring OG_name, NEW_name;
};

HWND curWin;
wchar_t titleBuf[255];
list<Win> wins;

void processWins() {
	auto i = wins.begin();
	while (i != wins.end()) {
		if (IsWindow(i->hwnd)) {
			GetWindowText(i->hwnd, titleBuf, 255);
			if (i->NEW_name.compare(titleBuf) != 0) { // name changed
				i->OG_name = titleBuf;
				SetWindowText(i->hwnd, i->NEW_name.c_str());
			}
			++i;
		}
		else { // window closed
			i = wins.erase(i);
		}
	}

}

BOOL CALLBACK DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_INITDIALOG:
		GetWindowText(curWin, titleBuf, 255);
		SetWindowText(GetDlgItem(hwndDlg, IDC_RICHEDIT21), titleBuf);
		SendMessage(GetDlgItem(hwndDlg, IDC_RICHEDIT21), EM_SETSEL, 0, -1);
		SetFocus(GetDlgItem(hwndDlg, IDC_RICHEDIT21));
		SendMessage(GetDlgItem(hwndDlg, IDC_RICHEDIT21), EM_SETBKGNDCOLOR, 0, RGB(240, 240, 240));
		return false;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_RICHEDIT21), titleBuf, 255);
			EndDialog(hwndDlg, 1);
			return true;
		case IDNO: // reset name
			EndDialog(hwndDlg, 2);
			return true;
		case IDHELP: // recreate window (move right in taskbar)
			EndDialog(hwndDlg, 3);
			return true;
		case IDCANCEL:
		case IDC_SPLIT1:
			EndDialog(hwndDlg, 0);
			return true;
		}
		return false;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == BCN_DROPDOWN) {
			NMBCDROPDOWN* pDropDown = (NMBCDROPDOWN*)lParam;
			POINT pt;
			pt.x = pDropDown->rcButton.left;
			pt.y = pDropDown->rcButton.bottom;
			ClientToScreen(pDropDown->hdr.hwndFrom, &pt);
			HMENU hMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1));
			hMenu = GetSubMenu(hMenu, 0);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwndDlg, NULL);
			return true;
		}
		return false;
	case WM_NCACTIVATE:
		if (wParam == 0 && IsWindowVisible(hwndDlg)) {
			EndDialog(hwndDlg, 0);
		}
		return true;
	default:
		return false;
	}
}

int main() {
	LoadLibrary(L"RichEd20.dll");
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_F7);

	auto rest = chrono::milliseconds(50);
	int loopCnt = 0;
	MSG msg;
	while (true) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			if (msg.message == WM_HOTKEY) {
				curWin = GetForegroundWindow();

				switch (DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), curWin, DlgProc)) {
				case 1: //ok
				{
					auto i = find_if(wins.begin(), wins.end(), [&](Win w) {return w.hwnd == curWin; });

					if (i != wins.end()) { // existing window
						i->NEW_name = titleBuf;
					}
					else { // new window
						Win w;
						w.hwnd = curWin;
						w.NEW_name = titleBuf;
						wins.emplace_back(w);
					}
				}
				break;
				case 2: // reset name
				{
					auto i = find_if(wins.begin(), wins.end(), [&](Win w) {return w.hwnd == curWin; });

					if (i != wins.end()) { // exists
						if (IsWindow(i->hwnd)) SetWindowText(i->hwnd, i->OG_name.c_str()); // in case window closed during
						wins.erase(i);
					}
				}
				break;
				case 3: // recreate window (move right in taskbar)
					curWin = GetForegroundWindow();

					if (IsWindow(curWin)) {
						ShowWindow(curWin, SW_HIDE);
						ShowWindow(curWin, SW_SHOW);
					}
					break;
				}
			}
		}

		if (++loopCnt >= 10) { // every 10 * 50 ms
			loopCnt = 0;
			processWins();
		}
		this_thread::sleep_for(rest);
	}
}