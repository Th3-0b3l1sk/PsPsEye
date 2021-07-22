#pragma once
#include <Windows.h>
#include <iostream>
#include <string>


#define FMT_HEADER_ONLY
#include "fmt\core.h"

/*
*
*	"""https://github.com/vmcall/loggr"""
*
*/


class Logger
{
public:
	Logger()
	{
		// instantiate a Console window
		auto console_window = GetConsoleWindow();
		if (console_window == 0x00) {

			AllocConsole();
	
			std::freopen("CONOUT$", "w", stdout);

			m_did_allocate_console = true;

		}
	}

	~Logger()
	{
		if (m_did_allocate_console)
		{
			FreeConsole();
		}
	}


	// LOGGING FUNCTIONS
	template<typename ... T>
	inline void log_raw(T... args) const
	{
		fmt::print(args...);

	}

	// string_view requires c++17
	inline void log(std::string_view message) const
	{
		fmt::print("[+] {}\n", message);
	}

	inline void log_error(std::string_view message) const
	{
		fmt::print("[!] {}\n", message);
	}


	inline void log_windows_error(std::string_view message) const
	{
		DWORD dwErrorCode = GetLastError();
		fmt::print("[!] {}, ErrorCode: {}\n", message, dwErrorCode);
	}


	template<bool hex = false, typename T>
	inline void log(std::string_view variable_name, const T& value) const
	{
		constexpr auto formate_string = hex ?
			"[=] {:<35} {:X}\n" :
			"[=] {:<35} {}\n";

		fmt::print(formate_string, variable_name, value);
	}

	template<std::size_t identation>
	inline void log_error_idented(std::string_view message) const
	{
		fmt::print("[!] {:<{}} {}\n", ' ', identation, message);
	}

	template<std::size_t identation>
	inline void log_idented(std::string_view message) const
	{
		fmt::print("[+] {:<{}} {}\n", ' ', identation, message);
	}

	template<std::size_t identation, bool hex = false, typename T>
	inline void log_idented(std::string_view variable_name, const T& value) const
	{
		constexpr auto format_string = hex ?
			"[=] {:<{}} {:.<20} {:02X}\n" :
			"[=] {:<{}} {:.<20} {}\n";

		fmt::print(format_string, ' ', identation, variable_name, value);
	}


	// CONSOLE MODIFICATION FUNCTIONS
	inline COORD get_position() const
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		{
			this->log_error("Failed to get cursor position.");
			return { 0, 0 };
		}

		return info.dwCursorPosition;
	}

	inline void set_position(const COORD cursor) const
	{
		if (!SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor))
		{

			this->log_error("Failed to set cursor position.");

		}
	}

	inline void clear_line() const
	{
		// GET CURSOR POSITION
		auto position = this->get_position();

		position.X = 0;

		// CLEAR LINE
		DWORD count = 0;
		const auto  handle = GetStdHandle(STD_OUTPUT_HANDLE);
		auto result = FillConsoleOutputCharacter(handle, ' ', 150, position, &count);

		// RESET POSITION
		this->set_position(position);

	}


private:

	bool m_did_allocate_console = false;



};


