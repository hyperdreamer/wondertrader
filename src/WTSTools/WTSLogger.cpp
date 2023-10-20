/*!
 * \file WTSLogger.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#include <stdio.h>
#include <iostream>
#include <sys/timeb.h>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "WTSLogger.h"
#include "../WTSUtils/WTSCfgLoader.h"
#include "../Includes/ILogHandler.h"
#include "../Includes/WTSVariant.hpp"
#include "../Share/StdUtils.hpp"
#include "../Share/StrUtil.hpp"
#include "../Share/TimeUtils.hpp"

#include <boost/filesystem.hpp>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/async.h>

static const char* DYN_PATTERN = "dyn_pattern";

ILogHandler*		WTSLogger::m_logHandler	= NULL;
WTSLogLevel			WTSLogger::m_logLevel	= LL_NONE;
bool				WTSLogger::m_bStopped = false;
bool				WTSLogger::m_bInited = false;
bool				WTSLogger::m_bTpInited = false;
SpdLoggerPtr		WTSLogger::m_rootLogger = NULL;
WTSLogger::LogPatterns*	WTSLogger::m_mapPatterns = NULL;
thread_local char	WTSLogger::m_buffer[];
std::set<std::string>	WTSLogger::m_setDynLoggers;

inline spdlog::level::level_enum str_to_level(const char* slvl)
{
    if (!wt_stricmp(slvl, "debug")) return spdlog::level::debug;
    if (!wt_stricmp(slvl, "info")) return spdlog::level::info;
    if (!wt_stricmp(slvl, "warn")) return spdlog::level::warn;
    if (!wt_stricmp(slvl, "error")) return spdlog::level::err;
    if (!wt_stricmp(slvl, "fatal")) return spdlog::level::critical;
    return spdlog::level::off;
}

inline WTSLogLevel str_to_ll(const char* slvl)
{ // case-insentive cmp, return 0 if match
    if (!wt_stricmp(slvl, "debug")) return LL_DEBUG;
    if (!wt_stricmp(slvl, "info")) return LL_INFO;
    if (!wt_stricmp(slvl, "warn")) return LL_WARN;
    if (!wt_stricmp(slvl, "error")) return LL_ERROR;
    if (!wt_stricmp(slvl, "fatal")) return LL_FATAL;
    return LL_NONE;
}

inline void checkDirs(const char* filename)
{
    std::string s = StrUtil::standardisePath(filename, false);
    std::size_t pos = s.find_last_of('/');
    if (pos == std::string::npos) return;   // no '/' found

    ++pos;
    if (!StdFile::exists(s.substr(0, pos).c_str()))
        boost::filesystem::create_directories(s.substr(0, pos).c_str());
}

inline void print_timetag(bool bWithSpace = true)
{
    uint64_t now = TimeUtils::getLocalTimeNow();
    time_t t = now / 1000;

    tm* tNow = localtime(&t);
    fmt::print("[{}.{:02d}.{:02d} {:02d}:{:02d}:{:02d}]", 
               tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, 
               tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
    if (bWithSpace) fmt::print(" ");
}

void WTSLogger::print_message(const char* buffer)
{
	print_timetag(true);
	fmt::print(buffer);
	fmt::print("\r\n");
}

void WTSLogger::initLogger(const char* catName, WTSVariant* cfgLogger)
{
    /*
     * @cfgLogger: map
     * entries:
     * (key: "async", value: boolean value)
     * (key: "level", value: string in {"debug", "info", "warn", "error", "fatal"})
     * (key: "sinks", value: array of sinks
     */
    bool bAsync = cfgLogger->getBoolean("async");
    /***************************************************************/
    const char* level = cfgLogger->getCString("level");
    /***************************************************************/
    WTSVariant* cfgSinks = cfgLogger->get("sinks"); // array of sinks
    std::vector<spdlog::sink_ptr> sinks;
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
    for (uint32_t idx = 0; idx < cfgSinks->size(); ++idx) {
        /*
         * @cfgSink: map
         * entries:
         * (key: "type", value: string of type)
         * (key: "filename", value: string of filename with %s)
         * (key: "pattern", value: format of output), Check the example
         */
        WTSVariant* cfgSink = cfgSinks->get(idx);
        /***************************************************************/
        const char* type = cfgSink->getCString("type");
        if (!strcmp(type, "daily_file_sink")) { // == 0
            std::string filename = cfgSink->getString("filename");
            StrUtil::replace(filename, "%s", catName);
            checkDirs(filename.c_str());
            /***************************************************************/
            // rotate every daily at 00:00
            auto sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, 0, 0);
            // example of sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v"); 
            sink->set_pattern(cfgSink->getCString("pattern"));
            sinks.emplace_back(sink);
        }
        else if (!strcmp(type, "basic_file_sink")) { // == 0
            std::string filename = cfgSink->getString("filename");
            StrUtil::replace(filename, "%s", catName);
            checkDirs(filename.c_str());
            /***************************************************************/
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, cfgSink->getBoolean("truncate"));
            sink->set_pattern(cfgSink->getCString("pattern"));
            sinks.emplace_back(sink);
        }
        else if (!strcmp(type, "console_sink")) { // == 0
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sink->set_pattern(cfgSink->getCString("pattern"));
            sinks.emplace_back(sink);
        }
        else if (!strcmp(type, "ostream_sink")) { // == 0
            auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
            sink->set_pattern(cfgSink->getCString("pattern"));
            sinks.emplace_back(sink);
        }
    }
    //////////////////////////////////////////////////////////////////////////
    if (!bAsync) {
        auto logger = std::make_shared<spdlog::logger>(catName, sinks.begin(), sinks.end());
        logger->set_level(str_to_level(cfgLogger->getCString("level")));
        spdlog::register_logger(logger);
    }
    else {
        if(!m_bTpInited) {
            spdlog::init_thread_pool(8192, 2);
            m_bTpInited = true;
        }
     
        auto logger = std::make_shared<spdlog::async_logger>(catName, sinks.begin(), sinks.end(), 
                                                             spdlog::thread_pool(), 
                                                             spdlog::async_overflow_policy::block);
        logger->set_level(str_to_level(cfgLogger->getCString("level")));
        spdlog::register_logger(logger);
    }
    //////////////////////////////////////////////////////////////////////////
    if(!strcmp(catName, "root")) m_logLevel = str_to_ll(cfgLogger->getCString("level"));
}

/*
 * default values defined by API
 * @propFile: logcfg.json
 * @isFile: true
 * @ILogHandler: NULL
 */
void WTSLogger::init(const char* propFile, bool isFile, ILogHandler* handler)
{
    if (m_bInited) return;
    if (isFile && !StdFile::exists(propFile)) return;
    /***************************************************************/
    /*
     * map in propFile:
     * entries:
     * TYPE I:  (key: "static logger name", value: map of static logger cfg)
     * TYPE II: (key: DYN_PATTERN, value: map of dynamic loggers)
     */
    WTSVariant* cfg = isFile ? WTSCfgLoader::load_from_file(propFile) 
                             : WTSCfgLoader::load_from_content(propFile, false);
    if (!cfg) return;
    //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
    auto keys = cfg->memberNames();
    for (std::string& key : keys) {
        WTSVariant* cfgItem = cfg->get(key.c_str());
        if (key == DYN_PATTERN) {
            // Type II entry: (key: DYN_PATTERN, value: map of dynamic loggers)
            auto pkeys = cfgItem->memberNames();
            for(std::string& pkey : pkeys) {
                WTSVariant* cfgPattern = cfgItem->get(pkey.c_str());
                if (!m_mapPatterns) m_mapPatterns = LogPatterns::create();
                /*
                 * @m_mapPatterns: map of dynamic loggers
                 * entry: (key: "dynamic logger name", value: map of dynamic logger cfg)
                 */
                m_mapPatterns->add(pkey.c_str(), cfgPattern, true);
            }
            continue;
        }
        /***************************************************************/
        initLogger(key.c_str(), cfgItem);
    }
    //////////////////////////////////////////////////////////////////////////
    m_rootLogger = getLogger("root"); // the root logger must be configed as a static logger
    spdlog::set_default_logger(m_rootLogger);
    spdlog::flush_every(std::chrono::seconds(2));
    //////////////////////////////////////////////////////////////////////////
    m_logHandler = handler;
    //////////////////////////////////////////////////////////////////////////
    m_bInited = true;
}

void WTSLogger::stop()
{
	m_bStopped = true;
	if (m_mapPatterns) m_mapPatterns->release();
	spdlog::shutdown();
}

void WTSLogger::debug_imp(SpdLoggerPtr logger, const char* message)
{
    if (logger) logger->debug(message);
    if (logger != m_rootLogger) m_rootLogger->debug(message);   // TO-FIX
    if (m_logHandler) m_logHandler->handleLogAppend(LL_DEBUG, message);
}

void WTSLogger::info_imp(SpdLoggerPtr logger, const char* message)
{
	if (logger)
		logger->info(message);

	if (logger != m_rootLogger)
		m_rootLogger->info(message);

	if (m_logHandler)
		m_logHandler->handleLogAppend(LL_INFO, message);
}

void WTSLogger::warn_imp(SpdLoggerPtr logger, const char* message)
{
	if (logger)
		logger->warn(message);

	if (logger != m_rootLogger)
		m_rootLogger->warn(message);

	if (m_logHandler)
		m_logHandler->handleLogAppend(LL_WARN, message);
}

void WTSLogger::error_imp(SpdLoggerPtr logger, const char* message)
{
	if (logger)
		logger->error(message);

	if (logger != m_rootLogger)
		m_rootLogger->error(message);

	if (m_logHandler)
		m_logHandler->handleLogAppend(LL_ERROR, message);
}

void WTSLogger::fatal_imp(SpdLoggerPtr logger, const char* message)
{
	if (logger)
		logger->critical(message);

	if (logger != m_rootLogger)
		m_rootLogger->critical(message);

	if (m_logHandler)
		m_logHandler->handleLogAppend(LL_FATAL, message);
}

void WTSLogger::log_raw(WTSLogLevel ll, const char* message)
{
	if (m_logLevel > ll || m_bStopped)
		return;

	if (!m_bInited)
	{
		print_message(message);
		return;
	}

	auto logger = m_rootLogger;

	if (logger)
	{
		switch (ll)
		{
		case LL_DEBUG:
			debug_imp(logger, message); break;
		case LL_INFO:
			info_imp(logger, message); break;
		case LL_WARN:
			warn_imp(logger, message); break;
		case LL_ERROR:
			error_imp(logger, message); break;
		case LL_FATAL:
			fatal_imp(logger, message); break;
		default:
			break;
		}
	}
}

void WTSLogger::log_raw_by_cat(const char* catName, WTSLogLevel ll, const char* message)
{
	if (m_logLevel > ll || m_bStopped)
		return;

	auto logger = getLogger(catName);
	if (logger == NULL)
		logger = m_rootLogger;

	if (!m_bInited)
	{
		print_timetag(true);
		printf(message);
		printf("\r\n");
		return;
	}

	if (logger)
	{
		switch (ll)
		{
		case LL_DEBUG:
			debug_imp(logger, message);
			break;
		case LL_INFO:
			info_imp(logger, message);
			break;
		case LL_WARN:
			warn_imp(logger, message);
			break;
		case LL_ERROR:
			error_imp(logger, message);
			break;
		case LL_FATAL:
			fatal_imp(logger, message);
			break;
		default:
			break;
		}
	}	
}

void WTSLogger::log_dyn_raw(const char* patttern, const char* catName, WTSLogLevel ll, const char* message)
{
	if (m_logLevel > ll || m_bStopped)
		return;

	auto logger = getLogger(catName, patttern);
	if (logger == NULL)
		logger = m_rootLogger;

	if (!m_bInited)
	{
		print_timetag(true);
		printf(m_buffer);
		printf("\r\n");
		return;
	}

	switch (ll)
	{
	case LL_DEBUG:
		debug_imp(logger, message);
		break;
	case LL_INFO:
		info_imp(logger, message);
		break;
	case LL_WARN:
		warn_imp(logger, message);// each sink cfg is a map
		break;
	case LL_ERROR:
		error_imp(logger, message);
		break;// each sink cfg is a map
	case LL_FATAL:
		fatal_imp(logger, message);
		break;
	default:
		break;
	}
}

/*
 * default value defined by API
 * @pattern: ""
 */
SpdLoggerPtr WTSLogger::getLogger(const char* logger, const char* pattern)
{
    SpdLoggerPtr ret = spdlog::get(logger); // first check if it already exists
    if (!ret && strlen(pattern) > 0) { // if not, treat it as as dynamic logger
        if (!m_mapPatterns) return SpdLoggerPtr();  // if == NULL
     
        WTSVariant* cfg = (WTSVariant*) m_mapPatterns->get(pattern);
        if (!cfg) return SpdLoggerPtr(); // if == NULL
     
        initLogger(logger, cfg);
        m_setDynLoggers.insert(logger);
        return spdlog::get(logger);
    }
    return ret;
}

void WTSLogger::freeAllDynLoggers()
{
    for(const std::string& logger : m_setDynLoggers) {
        auto loggerPtr = spdlog::get(logger);
        if(!loggerPtr) continue;
        spdlog::drop(logger);
    }
}
