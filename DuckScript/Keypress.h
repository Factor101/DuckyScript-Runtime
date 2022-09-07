#pragma once
#include <Windows.h>

template <class T>
class Keypress
{
	private:
		std::pair<INPUT, INPUT> key;
		bool shouldShift;
	public:
		static Keypress<T> shift;

		Keypress(T keyCode, bool _shouldShift = false) : shouldShift(_shouldShift)
		{
			key.first.type = INPUT_KEYBOARD;
			key.first.ki.wVk = (int)keyCode;
			key.first.ki.dwFlags = 0;
			key.second = key.first;
			key.second.ki.dwFlags = KEYEVENTF_KEYUP;
		}

		void keystroke(bool doShift = false) noexcept
		{
			if(!doShift && shouldShift) 
				doShift = true;

			if(doShift) 
				shift.press();

			SendInput(1, &key.first, sizeof(INPUT));
			SendInput(1, &key.second, sizeof(INPUT));

			if(doShift) 
				shift.release();
		}

		void press() noexcept
		{
			SendInput(1, &key.first, sizeof(INPUT));
		}

		void release() noexcept
		{
			SendInput(1, &key.second, sizeof(INPUT));
		}
};

template<class T>
Keypress<T> Keypress<T>::shift = Keypress<T>(VK_LSHIFT, false);