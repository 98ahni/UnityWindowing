//  This file is licenced under the GNU Affero General Public License. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once
#include "UnityAPI/IUnityLog.h"
#include <string>

#define LogInfo __FUNCTION__, __FILE__, __LINE__
#define UNITY_ASSERT(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (::Debug::ThrowException("!Native assert! " #expression), 0) \
        )
#define UNITY_LOG_CODE(message, expression) expression \
		::Debug::Log(std::string(message + \
			std::string("\nCODE") + #expression "\n").c_str()\
		)

class Debug
{
public:
	static void Init(IUnityLog* aLogger)
	{
		myLogger = aLogger;
	}
	static void Log(const char* message, const char* funcName = __builtin_FUNCTION(), const char* fileName = __builtin_FILE(), const int fileLine = __builtin_LINE())
	{
		myLogger->Log(kUnityLogTypeLog, (message + std::string("\n(") + funcName + ") " + std::string(fileName).substr(std::string(fileName).find_last_of('\\')) + " : " + std::to_string(fileLine)).c_str(), fileName, fileLine);
	}
	static void LogWarning(const char* message, const char* funcName = __builtin_FUNCTION(), const char* fileName = __builtin_FILE(), const int fileLine = __builtin_LINE())
	{
		myLogger->Log(kUnityLogTypeWarning, (message + std::string("\n(") + funcName + ") " + std::string(fileName).substr(std::string(fileName).find_last_of('\\')) + " : " + std::to_string(fileLine)).c_str(), fileName, fileLine);
	}
	static void LogError(const char* message, const char* funcName = __builtin_FUNCTION(), const char* fileName = __builtin_FILE(), const int fileLine = __builtin_LINE())
	{
		myLogger->Log(kUnityLogTypeError, (message + std::string("\n(") + funcName + ") " + std::string(fileName).substr(std::string(fileName).find_last_of('\\')) + " : " + std::to_string(fileLine)).c_str(), fileName, fileLine);
	}
	static void LogException(const char* message, const char* funcName = __builtin_FUNCTION(), const char* fileName = __builtin_FILE(), const int fileLine = __builtin_LINE())
	{
		myLogger->Log(kUnityLogTypeException, (message + std::string("\n(") + funcName + ") " + std::string(fileName).substr(std::string(fileName).find_last_of('\\')) + " : " + std::to_string(fileLine)).c_str(), fileName, fileLine);
	}
	static void ThrowException(const char* message, const char* funcName = __builtin_FUNCTION(), const char* fileName = __builtin_FILE(), const int fileLine = __builtin_LINE())
	{
		std::string msg = message + std::string("\n(") + funcName + ") " + std::string(fileName).substr(std::string(fileName).find_last_of('\\')) + " : " + std::to_string(fileLine);
		myLogger->Log(kUnityLogTypeException, msg.c_str(), fileName, fileLine);
		Throw(msg);
	}

private:
	static inline IUnityLog* myLogger;

	static void Throw(std::string& message)
	{
		__try { throw std::exception(message.c_str()); }
		__except (1) {}
	}
};