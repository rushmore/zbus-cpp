#ifndef __ZBUS_LOG_H__
#define __ZBUS_LOG_H__

#include "Platform.h"    

 
#define	LOG_ERROR	0	/* error conditions */
#define	LOG_WARN	1	/* warning conditions */ 
#define	LOG_INFO	2	/* informational */
#define	LOG_DEBUG	3	/* debug-level messages */ 
 

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


class ZBUS_API Logger { 
public: 
	inline static void configDefaultLogger(char* logDir=NULL, int level = LOG_INFO) {
		Logger* defaultLogger = getLogger();
		defaultLogger->level = level;
		if (logDir) {
			strcpy(defaultLogger->logDir, logDir);
			mkdirIfNeeded(defaultLogger->logDir);
			defaultLogger->createLogFile();
		}
	}
	inline static Logger* getLogger() {
		static Logger defaultLogger; 
		return &defaultLogger;
	}

public:
	Logger(char* logDir=NULL, int level=LOG_INFO) {
		this->level = level;
		if (logDir == NULL) {
			this->logFile = stdout;
		}
		else {
			strcpy(this->logDir, logDir);
			mkdirIfNeeded(this->logDir);
		} 
	}

	~Logger() {
		if (this->logFile != stdout) {
			fclose(this->logFile);
		} 
	}

	void setLevel(int level) {
		this->level = level;
	}
	int getLevel() {
		return this->level;
	}

	bool isDebugEnabled() {
		return level >= LOG_DEBUG;
	}

	void info(const char *format, ...) {
		int level = LOG_INFO;
		_DO_LOG(level);
	}

	void debug(const char *format, ...) {
		int level = LOG_DEBUG;
		_DO_LOG(level);
	}

	void warn(const char *format, ...) {
		int level = LOG_WARN;
		_DO_LOG(level);
	}

	void error(const char *format, ...) {
		int level = LOG_ERROR;
		_DO_LOG(level);
	}

	void debug(void* data, int len) {
		int level = LOG_DEBUG;
		_DO_LOG2(level, data, len);
	}

	void info(void* data, int len) {
		int level = LOG_INFO;
		_DO_LOG2(level, data, len);
	}

	void warn(void* data, int len) {
		int level = LOG_WARN;
		_DO_LOG2(level, data, len);
	}

	void error(void* data, int len) {
		int level = LOG_ERROR;
		_DO_LOG2(level, data, len);
	}

	void logHead(const int level) {
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

	void logBody(void* data, int len, const int level) {
		if (this->level >= (level)) {
			std::lock_guard<std::mutex> lock(this->mutex); 
			FILE* file = this->getLogFile();
			fwrite(data, 1, len, file);
			fflush(file); 
		}
	}

private:
	FILE* getLogFile() {
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

	void createLogFile() {
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

private:
	inline static int fileExists(const char* path) { 
		#if defined (__UNIX__)
		return access(path, 0) == 0;
		#elif (defined (__WINDOWS__))
		return _access(path, 0) == 0;
		#endif 
	}
	
	inline static int mkdirIfNeeded(const char *base_path) {
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

	inline static int64_t currentMillis(void) {
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


private:
	char  logDir[256];
	FILE* logFile;
	int level; 
	int logDate; 
	std::mutex mutex;
};

#endif

