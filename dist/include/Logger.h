#ifndef __ZBUS_LOG_H__
#define __ZBUS_LOG_H__

#include "Platform.h"    

 
#define	LOG_ERROR	0	/* error conditions */
#define	LOG_WARN	1	/* warning conditions */ 
#define	LOG_INFO	2	/* informational */
#define	LOG_DEBUG	3	/* debug-level messages */ 
 
 
namespace zbus { 
	class ZBUS_API Logger {
	public:
		static void configDefaultLogger(char* logDir = NULL, int level = LOG_INFO);
		static Logger* getLogger();

	public:
		Logger(char* logDir = NULL, int level = LOG_INFO); 
		~Logger();

		void setLevel(int level);
		int getLevel(); 
		bool isDebugEnabled(); 

		void info(const char *format, ...); 
		void debug(const char *format, ...); 
		void warn(const char *format, ...); 
		void error(const char *format, ...); 

		void debug(void* data, int len); 
		void info(void* data, int len); 
		void warn(void* data, int len); 
		void error(void* data, int len); 

		void logHead(const int level); 
		void logBody(void* data, int len, const int level);

	private:
		FILE* getLogFile(); 
		void createLogFile();

	private:
		static int fileExists(const char* path);
		static int mkdirIfNeeded(const char *base_path);
		static int64_t currentMillis(void); 

	private:
		char  logDir[256];
		FILE* logFile;
		int level;
		int logDate;
		std::mutex mutex;
	};

}//namespace
#endif

