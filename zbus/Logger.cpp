#include "Logger.h"

#define _DO_LOG(level) do{\
	if(this->level >= (level)){\
		std::lock_guard<std::mutex> lock(this->mutex);\
		this->logHead((level));\
		FILE* file = this->getLogFile();\
		va_list argptr;\
		va_start (argptr, format);\
		vfprintf ((file), format, argptr);\
		va_end (argptr);\
		fprintf (file, "\n");\
		fflush (file);\
	}\
}while(0)

#define _DO_LOG2(level, data, len) do{\
	if(this->level >= (level)){\
		std::lock_guard<std::mutex> lock(this->mutex);\
		this->logHead((level));\
		FILE* file = this->getLogFile();\
		fwrite(data, 1, len, file);\
		fflush (file);\
		fprintf (file, "\n");\
	}\
}while(0)

namespace zbus {
	 
	void Logger::configDefaultLogger(char* logDir, int level) {
		Logger* defaultLogger = getLogger();
		defaultLogger->level = level;
		if (logDir) {
			strcpy(defaultLogger->logDir, logDir);
			mkdirIfNeeded(defaultLogger->logDir);
			defaultLogger->createLogFile();
		}
	}
	Logger* Logger::getLogger() {
		static Logger defaultLogger;
		return &defaultLogger;
	}
	 
	Logger::Logger(char* logDir, int level) {
		this->level = level;
		if (logDir == NULL) {
			this->logFile = stdout;
		}
		else {
			strcpy(this->logDir, logDir);
			mkdirIfNeeded(this->logDir);
		}
	}

	Logger::~Logger() {
		if (this->logFile != stdout) {
			fclose(this->logFile);
		}
	}

	void Logger::setLevel(int level) {
		this->level = level;
	}

	int Logger::getLevel() {
		return this->level;
	}

	bool Logger::isDebugEnabled() {
		return level >= LOG_DEBUG;
	}

	void Logger::info(const char *format, ...) {
		int level = LOG_INFO;
		_DO_LOG(level);
	}

	void Logger::debug(const char *format, ...) {
		int level = LOG_DEBUG;
		_DO_LOG(level);
	}

	void Logger::warn(const char *format, ...) {
		int level = LOG_WARN;
		_DO_LOG(level);
	}

	void Logger::error(const char *format, ...) {
		int level = LOG_ERROR;
		_DO_LOG(level);
	}

	void Logger::debug(void* data, int len) {
		int level = LOG_DEBUG;
		_DO_LOG2(level, data, len);
	}

	void Logger::info(void* data, int len) {
		int level = LOG_INFO;
		_DO_LOG2(level, data, len);
	}

	void Logger::warn(void* data, int len) {
		int level = LOG_WARN;
		_DO_LOG2(level, data, len);
	}

	void Logger::error(void* data, int len) {
		int level = LOG_ERROR;
		_DO_LOG2(level, data, len);
	}

	void Logger::logHead(const int level) {
		FILE* file;
		time_t curtime = time(NULL);
		struct tm *loctime;
		char formatted[32];
		char *caption;

		loctime = localtime(&curtime);
		file = getLogFile();
		strftime(formatted, 32, "[%Y-%m-%d %H:%M:%S", loctime);
		fprintf(file, "%s.%03d] ", formatted, currentMillis() % 1000);

		switch (level)
		{
		case LOG_DEBUG:
			caption = "DEBUG";
			break;
		case LOG_INFO:
			caption = "INFO";
			break;
			break;
		case LOG_WARN:
			caption = "WARNING";
			break;
		case LOG_ERROR:
			caption = "ERROR";
			break;
		default:
			caption = "UNKOWN";
			break;
		}

		fprintf(file, "%s - ", caption);
	}

	void Logger::logBody(void* data, int len, const int level) {
		if (this->level >= (level)) {
			std::lock_guard<std::mutex> lock(this->mutex);
			FILE* file = this->getLogFile();
			fwrite(data, 1, len, file);
			fflush(file);
		}
	}
	 
	FILE* Logger::getLogFile() {
		int date;
		char fdate[32];
		if (this->logFile == stdout) {
			return this->logFile;
		}
		time_t curtime = time(NULL);
		struct tm *loctime;
		loctime = localtime(&curtime);
		strftime(fdate, 32, "%Y%m%d", loctime);
		date = atoi(fdate);

		if (date > this->logDate) {
			this->createLogFile();
		}
		return this->logFile;
	}

	void Logger::createLogFile() {
		char fdate[32];
		time_t curtime = time(NULL);
		struct tm *loctime;
		loctime = localtime(&curtime);
		strftime(fdate, 32, "%Y%m%d", loctime);
		int date = atoi(fdate);

		char newfile[256];
		this->logDate = date;
		snprintf(newfile, sizeof(newfile), "%s/%s.log", this->logDir, fdate);
		if (this->logFile != stdout && this->logFile) {
			fclose(this->logFile);
		}
		this->logFile = fopen(newfile, "a+");
		if (!this->logFile) {
			printf("create log file error[%s],using stdout instead\n", newfile);
			this->logFile = stdout;
		}
	}
	 
	int Logger::fileExists(const char* path) {
#if defined (__UNIX__)
		return access(path, 0) == 0;
#elif (defined (__WINDOWS__))
		return _access(path, 0) == 0;
#endif 
	}


	int Logger::mkdirIfNeeded(const char *base_path) {
		if (!fileExists(base_path)) {
#if defined (__UNIX__)
			if (mkdir(base_path, 0755) != 0) {
#elif (defined (__WINDOWS__))
			if (_mkdir(base_path) != 0) {
#endif
				return errno != 0 ? errno : EPERM;
			}
			}
		return 0;
		}

	int64_t Logger::currentMillis(void) {
#if defined (__UNIX__)
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (int64_t)((int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000);

#elif (defined (__WINDOWS__))
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		return (int64_t)(*((int64_t *)(&ft)) / 10000);
#endif
	} 
}//namespace