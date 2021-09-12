#include <iostream>
#include <Windows.h>
#include <thread>
#include <list>
#include <WinUser.h>
using namespace std;

struct Win {
	HWND hwnd;
	wstring OG_name, NEW_name;
};

HWND curWin;
wchar_t titleBuf[255];
list<Win> wins;

bool matchHWND(HWND a, HWND b) {
	return a == b;
}

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

int main() {
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_F7);

	auto rest = std::chrono::milliseconds(50);
	int loopCnt = 0;
	MSG msg;
	while (true) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | (QS_HOTKEY << 16)) != 0) {
			curWin = GetForegroundWindow();

			auto i = find_if(wins.begin(), wins.end(), [&](Win w) {return w.hwnd == curWin; });

			if (i != wins.end()) { // existing window
				i->NEW_name = L"existing";
			}
			else { // new window
				Win w;
				w.hwnd = curWin;
				w.NEW_name = L"test win";
				wins.emplace_back(w);
			}
		}

		if (++loopCnt >= 10) { // every 10 * 50 ms
			loopCnt = 0;
			processWins();
		}
		this_thread::sleep_for(rest);
	}
}