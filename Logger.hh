// Warning: this library uses Return Value Optimization
// (https://en.wikipedia.org/wiki/Return_value_optimization)
// to work properly. Basically add -felide-constructors to
// complier's flags.

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <ctime>
#include <mutex>

class Logger
{
public:
	enum class LogLevel
	{
		Error,
		Warning,
		Info,
		Verbose,
		Debug,
	};
private:
	class StreamWrapper
	{
	private:
		std::ostringstream * s;
		Logger * parent;
		void StartLogging();
		LogLevel message_level;
	public:
		StreamWrapper(Logger * _parent, LogLevel _level);
		template <typename Type> std::ostringstream & operator<<(Type x)
		{
			(*s) << x;
			return *s;
		}
		~StreamWrapper();
	};
	class Progress
	{
	private:
		Logger * parent;
		int start;
		int end;
		int step;
		int number_of_updates;
		int count;
		bool finished;
		LogLevel message_level;
	public:
		Progress(int _start, int _end, int _step, int _number_of_updates, int _count, Logger * _parent, LogLevel _level);
		void Count();
	};
	bool writing_to_file;
	std::ostream * logger_out;
	std::ofstream file;
	int spaces_at_the_begining;
	LogLevel logger_level;
	LogLevel message_level;
	static std::map<LogLevel, std::string> message_colors;
	static std::map<LogLevel, std::string> message_headers;
	static std::mutex logger_is_writing;
	std::vector<std::string> entered_stuff;
public:
	Logger(std::ostream *_out = &std::cout);
	Logger(std::string _file);
	~Logger();
	StreamWrapper Log();
	StreamWrapper Log(LogLevel _level);
	Progress InitializeProgress(int _start, int _end, int _step = 1, int _updates = 10);
	Progress InitializeProgress(LogLevel _level, int _start, int _end, int _step = 1, int _updates = 10);
	void SetLoggerLevel(LogLevel _level);
	void SetDefaultMessageLevel(LogLevel _level);
	void SetNewOutput(std::ostream * _out);
	void SetNewOutput(std::string _file);
	void Enter(std::string _info);
	void Exit();
};
