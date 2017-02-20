#include "Logger.hh"

std::map<Logger::LogLevel, std::string> Logger::message_colors = {
		{LogLevel::Error,	"\033[31m"},
		{LogLevel::Warning, "\033[33m"},
		{LogLevel::Info, 	"\033[32m"},
		{LogLevel::Verbose,	"\033[34m"},
		{LogLevel::Debug,	"\033[35m"}
	};

std::map<Logger::LogLevel, std::string> Logger::message_headers = {
		{LogLevel::Error,	"\033[1m[  Error  ]\033[0m "},
		{LogLevel::Warning, "\033[1m[ Warning ]\033[0m "},
		{LogLevel::Info, 	"\033[1m[  Info   ]\033[0m "},
		{LogLevel::Verbose,	"\033[1m[ Verbose ]\033[0m "},
		{LogLevel::Debug,	"\033[1m[  Debug  ]\033[0m "}
	};


Logger::Logger(std::ostream * _out)
{
	logger_out = _out;
	logger_level = Logger::LogLevel::Info;
	spaces_at_the_begining = 0;
}

Logger::Logger(std::string _file)
{
	logger_out = nullptr;
	logger_file = _file;
	logger_level = Logger::LogLevel::Info;
	spaces_at_the_begining = 0;
}

void Logger::StreamWrapper::StartLogging()
{
	(*s) <<Logger::message_colors[message_level];
	time_t rawtime;
	time(&rawtime);
	auto timehere = std::localtime(&rawtime);
	char buffer[20];
	strftime(buffer, 20, "[ %X ] ", timehere);
	(*s) <<buffer;
	(*s) <<Logger::message_headers[message_level];
	if (message_level == Logger::LogLevel::Error)
		(*s) << "\033[1;31m";
	for (int i = 0; i < parent->spaces_at_the_begining; ++i)
		(*s) <<" ";
}

Logger::StreamWrapper::StreamWrapper(Logger * _parent, Logger::LogLevel _level)
{
	parent = _parent;
	message_level = _level;
	if (parent->logger_out != nullptr)
	{
		out = parent->logger_out;
	}
	else
	{
		if (parent->logger_file != "")
		{
			out = nullptr;
			file = parent->logger_file;
		}
		else
			out = &std::cout;
	}
	s = new std::ostringstream;
	StartLogging();
}

Logger::StreamWrapper::StreamWrapper(std::ostream * _out, Logger * _parent, Logger::LogLevel _level)
{
	parent = _parent;
	message_level = _level;
	out = _out;
	s = new std::ostringstream;
	StartLogging();
}

Logger::StreamWrapper::StreamWrapper(std::string _file, Logger * _parent, Logger::LogLevel _level)
{
	out = nullptr;
	message_level = _level;
	parent = _parent;
	file = _file;
	s = new std::ostringstream;
	StartLogging();
}

Logger::StreamWrapper::~StreamWrapper()
{
	if (message_level == Logger::LogLevel::Error)
		(*s) << "\033[0m";
	(*s) << "\n";
	if (parent->logger_level >= message_level)
	{
		if (out == nullptr)
		{
			std::ofstream fstr;
			fstr.open(file, std::ios::out | std::ios::app);
			fstr << s->str();
			fstr.close();
		}
		else
		{
			(*out) << s->str();
		}
	}
	delete s;
}

Logger::StreamWrapper Logger::Log()
{
	Logger::StreamWrapper l(this, this->message_level);
	return l;
}

Logger::StreamWrapper Logger::Log(Logger::LogLevel _level)
{
	Logger::StreamWrapper l(this, _level);
	return l;
}

Logger::StreamWrapper Logger::Log(std::ostream * _out)
{
	Logger::StreamWrapper l(_out, this, this->message_level);
	return l;
}

Logger::StreamWrapper Logger::Log(std::ostream * _out, Logger::LogLevel _level)
{
	Logger::StreamWrapper l(_out, this, _level);
	return l;
}

Logger::StreamWrapper Logger::Log(std::string _file)
{
	Logger::StreamWrapper l(_file, this, this->message_level);
	return l;
}

Logger::StreamWrapper Logger::Log(std::string _file, Logger::LogLevel _level)
{
	Logger::StreamWrapper l(_file, this, _level);
	return l;
}
Logger::Progress::Progress (int _start, int _end, int _step, int _number_of_updates, int _count, Logger * _parent, Logger::LogLevel _level)
: parent(_parent)
, start(_start)
, end(_end)
, step(_step)
, number_of_updates(_number_of_updates)
, count(_count)
, finished(false)
, message_level(_level)
{}

Logger::Progress Logger::InitializeProgress(int _start, int _end, int _step, int _updates)
{
	spaces_at_the_begining += 4;
	return Logger::Progress(_start, _end, _step, _updates, _start-1, this, this->message_level);
}

Logger::Progress Logger::InitializeProgress(Logger::LogLevel _level, int _start, int _end, int _step, int _updates)
{
	spaces_at_the_begining += 4;
	return Logger::Progress(_start, _end, _step, _updates, _start-1, this, _level);
}

void Logger::Progress::Count()
{
	if (!finished)
	{
		count += step;
		if (count%((end-start)/step/number_of_updates) == 0)
			parent -> Log() << count <<" \033[36m/\033[0m " <<end;
		if (count + step >= end)
		{
			parent->spaces_at_the_begining -= 4;
			finished = true;
		}
	}
}

void Logger::SetLoggerLevel(Logger::LogLevel _level)
{
	logger_level = _level;
}

void Logger::SetDefaultMessageLevel(Logger::LogLevel _level)
{
	message_level = _level;
}

void Logger::Enter(std::string _info)
{
	Log()<<"\033[1mEntering " <<_info <<"\033[0m";
	entered_stuff.push_back(_info);
	spaces_at_the_begining += 4;
}

void Logger::Exit()
{
	if (entered_stuff.size() > 0)
	{
		auto info = entered_stuff[entered_stuff.size()-1];
		entered_stuff.pop_back();
		spaces_at_the_begining -= 4;
		Log()<<"\033[1mExiting " <<info <<"\033[0m";
		return;
	}
	Log(Logger::LogLevel::Error) <<"LOGGER ERROR: Exiting without entering\n";
	throw "Exiting without entering\n";
}
