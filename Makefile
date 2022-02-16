OUTPUT = pet-test

# Test code

ifeq (,_)
endif

SOURCES += TestAlgoCharLookup.cpp
SOURCES += TestAlgoCharTraits.cpp
SOURCES += TestAlgoCobs.cpp
SOURCES += TestAlgoStr.cpp
SOURCES += TestAlgoMinMaxHeap.cpp
SOURCES += TestAlgoIntegerDivision.cpp
SOURCES += TestAlgoUnalignedAccess.cpp
SOURCES += TestDataExtremeSetFilter.cpp
SOURCES += TestDataFifo.cpp
SOURCES += TestDataUnion.cpp
SOURCES += TestDataBinaryTree.cpp
SOURCES += TestDataBinaryHeap.cpp
SOURCES += TestDataDoubleList.cpp
SOURCES += TestDataLinkedList.cpp
SOURCES += TestDataLinkedListSpecial.cpp
SOURCES += TestDataLinRegFilter.cpp
SOURCES += TestDataMaybe.cpp
SOURCES += TestDataMedianFilter.cpp
SOURCES += TestDataMutableStorage.cpp
SOURCES += TestDataPriorityQueue.cpp
SOURCES += TestDataCircularBuffer.cpp
SOURCES += TestDataOrderedDoubleList.cpp
SOURCES += TestIntegrationRefCntDelegate.cpp
SOURCES += TestIntegrationSmartPtrLinkedList.cpp
SOURCES += TestManagedTreeMap.cpp
SOURCES += TestManagedRefCnt.cpp
SOURCES += TestManagedUnique.cpp
SOURCES += TestMetaExpLog.cpp
SOURCES += TestMetaExpRange.cpp
SOURCES += TestMetaMetaString.cpp
SOURCES += TestMetaPacks.cpp
SOURCES += TestMetaRange.cpp
SOURCES += TestMetaSequence.cpp
SOURCES += TestMetaSfinae.cpp
SOURCES += TestMetaStateChange.cpp
SOURCES += TestMetaTuple.cpp
SOURCES += TestMetaTypePackInfo.cpp
SOURCES += TestPlatformAtomic.cpp
SOURCES += TestUbiqDelegate.cpp
SOURCES += TestHeapTlsf.cpp
SOURCES += TestHeapBuddy.cpp
SOURCES += TestHeapHost1.cpp
SOURCES += TestHeapHost2.cpp
SOURCES += TestHeapHost3.cpp
SOURCES += TestHeapHost4.cpp
SOURCES += TestHeapHost5.cpp
SOURCES += TestHeapStress.cpp
SOURCES += TestUbiqTrace.cpp

# Test support

SOURCES += ../1test/TestRunnerExperimental.cpp
SOURCES += ../ubiquitous/PrintfWriter.cpp
SOURCES += MockWriter.cpp
SOURCES += DebugConfig.cpp
SOURCES += TestMain.cpp

# Includes

INCLUDE_DIRS += .
INCLUDE_DIRS += failure-injector

# Flags

CXXFLAGS += -std=c++17								# Use the c++ standard released in 2017
CXXFLAGS += -O0 									# Avoid optimizer screwing up coverage data
CXXFLAGS += -g3										# Include all available debug info
CXXFLAGS += -fno-inline								# Do not inline (coverage again)
CXXFLAGS += -fmax-errors=5							# Avoid blowing up the console
CXXFLAGS += -rdynamic								# Allows meaningful backtraces 
CXXFLAGS += --coverage								# Generate program flow data and inject profiling code
CXXFLAGS += -fno-exceptions -fno-inline
CXXFLAGS += -fdelete-null-pointer-checks

LIBS += gcov

LD=$(CXX) 

ifeq (_,)
CXXFLAGS += -m32
LDFLAGS += -m32
endif

include ../mod.mk
include ultimate-makefile/Makefile.ultimate	
