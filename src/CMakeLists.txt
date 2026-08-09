CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(WonderTrader)

# Always export compile_commands.json for clangd/LSP (nvim)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#3. 环境判断
IF (WIN32)
	MESSAGE(STATUS "Operation System is windows")
ELSEIF (UNIX)
	MESSAGE(STATUS "Operation System is UNIX-like OS's")
ENDIF ()

MESSAGE(STATUS "Generator is ${CMAKE_GENERATOR}")

IF (MSVC)
	ADD_DEFINITIONS (-D_CRT_SECURE_NO_WARNINGS)
	IF(CMAKE_CL_64)
		SET(INCS $ENV{MyDepends141}/include)
		SET(LNKS $ENV{MyDepends141}/lib/x64)

		MESSAGE(STATUS "MyDepends is at $ENV{MyDepends141}")
		MESSAGE(STATUS "Platform is x64")
		SET(PLATFORM x64)
	ELSE()
		SET(INCS $ENV{MyDepends141}/include)
		SET(LNKS $ENV{MyDepends141}/lib/x86)

		MESSAGE(STATUS "MyDepends is at $ENV{MyDepends141}")
		MESSAGE(STATUS "Platform is x86")
		SET(PLATFORM x86)
	ENDIF()
	SET(PREFIX )
	SET(SUFFIX .dll)
ELSE (GNUCC)
	IF(WIN32)
		SET (MyDeps $ENV{MyDependsGcc})
		SET(SUFFIX .dll)

		MESSAGE(STATUS "MyDepends is at $ENV{MyDependsGcc}")
	ELSE(UNIX)
		SET (MyDeps "/home/mydeps")
		SET(SUFFIX .so)

		MESSAGE(STATUS "MyDepends is at /home/mydeps")
	ENDIF()

	SET(INCS ${MyDeps}/include)
	SET(LNKS ${MyDeps}/lib)

	MESSAGE(STATUS "Platform is x64")
	SET(PLATFORM x64)
	SET(PREFIX lib)
ENDIF()

SET(CMAKE_CXX_STANDARD 17)

#basic libraries
ADD_SUBDIRECTORY(WTSUtils)
ADD_SUBDIRECTORY(WTSTools)

#contract Loade through ctp channel
ADD_SUBDIRECTORY(CTPLoader)
ADD_SUBDIRECTORY(CTPOptLoader)
ADD_SUBDIRECTORY(LoaderRunner)

#parser for quote date from ctp channel
ADD_SUBDIRECTORY(ParserCTP)
ADD_SUBDIRECTORY(ParserCTPMini)
ADD_SUBDIRECTORY(ParserCTPOpt)
ADD_SUBDIRECTORY(ParserFemas)
ADD_SUBDIRECTORY(ParserXTP)
ADD_SUBDIRECTORY(ParserShm)
ADD_SUBDIRECTORY(ParserXeleSkt)
ADD_SUBDIRECTORY(WtDataStorage)
ADD_SUBDIRECTORY(WtDataStorageAD)
ADD_SUBDIRECTORY(WtIndexFactory)
ADD_SUBDIRECTORY(WtDtCore)
ADD_SUBDIRECTORY(WtDtHelper)
ADD_SUBDIRECTORY(WtDtPorter)
ADD_SUBDIRECTORY(WtDtServo)
#quote date processor
ADD_SUBDIRECTORY(QuoteFactory)

#back test core library
ADD_SUBDIRECTORY(WtBtCore)
#back test c porter for outter libraries
ADD_SUBDIRECTORY(WtBtPorter)
ADD_SUBDIRECTORY(WtBtRunner)

ADD_SUBDIRECTORY(ParserUDP)
ADD_SUBDIRECTORY(TraderCTP)
ADD_SUBDIRECTORY(TraderCTPMini)
ADD_SUBDIRECTORY(TraderCTPOpt)
ADD_SUBDIRECTORY(TraderFemas)
ADD_SUBDIRECTORY(TraderMocker)
ADD_SUBDIRECTORY(TraderXTP)
ADD_SUBDIRECTORY(TraderYD)
ADD_SUBDIRECTORY(WtExeFact)
ADD_SUBDIRECTORY(WtRiskMonFact)

ADD_SUBDIRECTORY(WtMsgQue)
ADD_SUBDIRECTORY(WtCore)
ADD_SUBDIRECTORY(WtPorter)
ADD_SUBDIRECTORY(WtExecMon)
ADD_SUBDIRECTORY(WtRunner)

ADD_SUBDIRECTORY(TraderDumper)

ADD_SUBDIRECTORY(WtShareHelper)

#uft
ADD_SUBDIRECTORY(WtUftCore)
ADD_SUBDIRECTORY(WtUftStraFact)
ADD_SUBDIRECTORY(WtUftRunner)

ADD_SUBDIRECTORY(WtLatencyHFT)
ADD_SUBDIRECTORY(WtLatencyUFT)

#test projects
ADD_SUBDIRECTORY(TestBtPorter)
ADD_SUBDIRECTORY(TestDtPorter)
ADD_SUBDIRECTORY(TestExecPorter)
ADD_SUBDIRECTORY(TestPorter)
ADD_SUBDIRECTORY(TestTrader)
ADD_SUBDIRECTORY(TestParser)
ADD_SUBDIRECTORY(TestUnits)

# ------------------------------------------------------------------
# LSP / clangd index-only plugins (default OFF)
# ------------------------------------------------------------------
# These plugin directories are NOT part of the regular Linux build
# (official release builds them on Windows only, or they need SDKs that
# are not available on this platform). Configuring with -DWT_LSP_INDEX=ON
# registers them so CMAKE_EXPORT_COMPILE_COMMANDS covers every source
# file and clangd can index them. All targets are EXCLUDE_FROM_ALL and
# must NOT be built - this is for IDE/indexing only.
OPTION(WT_LSP_INDEX "Add index-only plugin targets for clangd/LSP (do not build)" OFF)
IF (WT_LSP_INDEX)
	INCLUDE_DIRECTORIES(${INCS}
		${CMAKE_CURRENT_SOURCE_DIR}/API/oesApi0.17.5.8/include
		${CMAKE_CURRENT_SOURCE_DIR}/API/AtpTradeApi/include)
	ADD_SUBDIRECTORY(WtCtaStraFact)
	ADD_SUBDIRECTORY(WtHftStraFact)
	ADD_SUBDIRECTORY(WtSelStraFact)
	ADD_SUBDIRECTORY(ParserOES)
	ADD_SUBDIRECTORY(ParserYD)
	ADD_SUBDIRECTORY(TraderATP)
	ADD_SUBDIRECTORY(TraderDD)
	ADD_SUBDIRECTORY(TraderOES)

	# dirs with broken/missing CMakeLists.txt (upstream copy-paste bugs):
	# TraderXTPXAlgo defines target TraderXTP with wrong sources,
	# TraderHTS defines target TraderDD with missing sources,
	# ParserHuaX/TraderHuaX have no CMakeLists.txt at all
	ADD_LIBRARY(ParserHuaX SHARED EXCLUDE_FROM_ALL ParserHuaX/ParserHuaX.cpp)
	ADD_LIBRARY(TraderHuaX SHARED EXCLUDE_FROM_ALL TraderHuaX/TraderHuaX.cpp)
	ADD_LIBRARY(TraderHTS SHARED EXCLUDE_FROM_ALL TraderHTS/TraderHTS.cpp)
	ADD_LIBRARY(TraderXTPXAlgo SHARED EXCLUDE_FROM_ALL TraderXTPXAlgo/TraderXTPXAlgo.cpp)

	FOREACH(t WtCtaStraFact WtHftStraFact WtSelStraFact ParserOES ParserYD TraderATP TraderDD TraderOES)
		SET_TARGET_PROPERTIES(${t} PROPERTIES EXCLUDE_FROM_ALL TRUE)
	ENDFOREACH()
ENDIF()

