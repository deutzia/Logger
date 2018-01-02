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

std::mutex Logger::logger_is_writing;

Logger::Logger(std::ostream * _out)
{
	writing_to_file = false;
	logger_out = _out;
	logger_level = Logger::LogLevel::Info;
	spaces_at_the_begining = 0;
	message_level = Logger::LogLevel::Info;
}

Logger::Logger(std::string _file)
{
	logger_out = nullptr;
	file.open(_file, std::ios::out | std::ios::app);
	writing_to_file = true;
	logger_level = Logger::LogLevel::Info;
	spaces_at_the_begining = 0;
	message_level = Logger::LogLevel::Info;
}

Logger::~Logger()
{
	if (writing_to_file)
		file.close();
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
	s = new std::ostringstream;
	StartLogging();
}

Logger::StreamWrapper::StreamWrapper(Logger::StreamWrapper && other)
{
	parent = other.parent;
	message_level = other.message_level;
	s = other.s;
	other.s = nullptr;
}

Logger::StreamWrapper::~StreamWrapper()
{
	if (s != nullptr)
	{
		if (message_level == Logger::LogLevel::Error)
			(*s) << "\033[0m";
		(*s) << "\n";
		logger_is_writing.lock();
		if (parent->logger_level >= message_level)
		{
			if (parent->writing_to_file)
			{
				parent->file << s->str();
				parent->file <<std::flush;
			}
			else
			{
				(*parent->logger_out) << s->str();
				(*parent->logger_out) << std::flush;
			}
		}
		logger_is_writing.unlock();
		delete s;
	}
}

Logger::StreamWrapper Logger::Log()
{
	Logger::StreamWrapper l(this, this->message_level);
	return std::move(l);
}

Logger::StreamWrapper Logger::Log(Logger::LogLevel _level)
{
	Logger::StreamWrapper l(this, _level);
	return std::move(l);
}

Logger::StreamWrapper Logger::LogE()
{
	Logger::StreamWrapper l(this, Logger::LogLevel::Error);
	return std::move(l);
}
Logger::StreamWrapper Logger::LogW()
{
	Logger::StreamWrapper l(this, Logger::LogLevel::Warning);
	return std::move(l);
}
Logger::StreamWrapper Logger::LogI()
{
	Logger::StreamWrapper l(this, Logger::LogLevel::Info);
	return std::move(l);
}
Logger::StreamWrapper Logger::LogV()
{
	Logger::StreamWrapper l(this, Logger::LogLevel::Verbose);
	return std::move(l);
}
Logger::StreamWrapper Logger::LogD()
{
	Logger::StreamWrapper l(this, Logger::LogLevel::Debug);
	return std::move(l);
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
	return Logger::Progress(_start, _end, _step, std::min(_updates, (_end-_start+_step-1)/_step), _start-1, this, this->message_level);
}

Logger::Progress Logger::InitializeProgress(Logger::LogLevel _level, int _start, int _end, int _step, int _updates)
{
	spaces_at_the_begining += 4;
	return Logger::Progress(_start, _end, _step, std::min(_updates, (_end-_start+_step-1)/_step), _start-1, this, _level);
}

void Logger::Progress::Count()
{
	if (!finished)
	{
		count += step;
		if (count%(((end-start+step-1)/step+number_of_updates-1)/number_of_updates) == 0)
		{
			std::string spaces;
			for (int i = std::to_string(end).size() - std::to_string(count).size(); i > 0; --i)
				spaces += " ";
			parent -> Log(message_level) << spaces << count <<" / " <<end;
		}
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

void Logger::SetNewOutput(std::ostream * _out)
{
	if (writing_to_file)
		file.close();
	logger_out = _out;
	writing_to_file = false;
}

void Logger::SetNewOutput(std::string _file)
{
	if (writing_to_file)
		file.close();
	else
		logger_out = nullptr;
	file.open(_file, std::ios::out | std::ios::app);
	writing_to_file = true;
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
