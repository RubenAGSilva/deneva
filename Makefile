CC=g++ -g -Wall 
CFLAGS=-Wall -g -gdwarf-3 -std=c++0x 
#CFLAGS += -fsanitize=address -fno-omit-frame-pointer 
JEMALLOC=./jemalloc-4.0.3
NNMSG=./nanomsg-0.5-beta/.libs
BOOST=/usr/lib/x86_64-linux-gnu/

.SUFFIXES: .o .cpp .h

SRC_DIRS = ./ ./framework/utils/ ./framework/ ./benchmarks/ ./client/ ./concurrency_control/ ./storage/ ./transport/ ./system/ ./statistics/ #./unit_tests/
DEPS = -I. -I./framework/utils -I./framework -I./benchmarks -I./client/ -I./concurrency_control -I./storage -I./transport -I./system -I./statistics -I$(JEMALLOC)/include #-I./unit_tests 

CFLAGS += $(DEPS) -D NOGRAPHITE=1 -Werror -Wno-sizeof-pointer-memaccess -Wno-unused-function -Wno-unused-but-set-variable -DBOOST_SYSTEM_NO_DEPRECATED
LDFLAGS = -Wall -L. -L$(NNMSG) -L$(JEMALLOC)/lib -L$(BOOST) -Wl,-rpath,$(JEMALLOC)/lib -pthread -gdwarf-3 -lrt -std=c++0x
#LDFLAGS = -Wall -L. -L$(NNMSG) -L$(JEMALLOC)/lib -Wl,-rpath,$(JEMALLOC)/lib -pthread -gdwarf-3 -lrt -std=c++11
LDFLAGS += $(CFLAGS) --verbose
LIBS = -lnanomsg -lanl -ljemalloc -lboost_system -lboost_serialization

DB_MAINS = ./client/client_main.cpp ./system/sequencer_main.cpp ./unit_tests/unit_main.cpp
CL_MAINS = ./system/main.cpp ./system/sequencer_main.cpp ./unit_tests/unit_main.cpp
UNIT_MAINS = ./system/main.cpp ./client/client_main.cpp ./system/sequencer_main.cpp

CPPS_DB = $(foreach dir,$(SRC_DIRS),$(filter-out $(DB_MAINS), $(wildcard $(dir)*.cpp))) 
CPPS_CL = $(foreach dir,$(SRC_DIRS),$(filter-out $(CL_MAINS), $(wildcard $(dir)*.cpp))) 
CPPS_UNIT = $(foreach dir,$(SRC_DIRS),$(filter-out $(UNIT_MAINS), $(wildcard $(dir)*.cpp))) 

#CPPS = $(wildcard *.cpp)
OBJS_DB = $(addprefix obj/, $(notdir $(CPPS_DB:.cpp=.o)))
OBJS_CL = $(addprefix obj/, $(notdir $(CPPS_CL:.cpp=.o)))
OBJS_UNIT = $(addprefix obj/, $(notdir $(CPPS_UNIT:.cpp=.o)))

#NOGRAPHITE=1

all:rundb runcl 
#unit_test

.PHONY: deps_db
deps:$(CPPS_DB)
	$(CC) $(CFLAGS) -MM $^ > obj/deps
	sed '/^[^ ]/s/^/obj\//g' obj/deps > obj/deps.tmp
	mv obj/deps.tmp obj/deps
-include obj/deps

unit_test :  $(OBJS_UNIT)
#	$(CC) -static -o $@ $^ $(LDFLAGS) $(LIBS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
./obj/%.o: transport/%.cpp
#	$(CC) -static -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
./obj/%.o: unit_tests/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: benchmarks/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: storage/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: framework/utils/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: framework/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: system/%.cpp
	$(CC) -c -DSTATS_ENABLE=false $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: concurrency_control/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: client/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
	


rundb : $(OBJS_DB)
	$(CC) -static -o $@ $^ $(LDFLAGS) $(LIBS)
#	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
./obj/%.o: transport/%.cpp
	$(CC) -static -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#./deps/%.d: %.cpp
#	$(CC) -MM -MT $*.o -MF $@ $(CFLAGS) $<
./obj/%.o: benchmarks/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: storage/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: framework/utils/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: framework/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: system/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: statistics/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: concurrency_control/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: client/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<


runcl : $(OBJS_CL)
	$(CC) -static -o $@ $^ $(LDFLAGS) $(LIBS)
#	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
./obj/%.o: transport/%.cpp
	$(CC) -static -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#./deps/%.d: %.cpp
#	$(CC) -MM -MT $*.o -MF $@ $(CFLAGS) $<
./obj/%.o: benchmarks/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: storage/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: framework/utils/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: framework/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: system/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: statistics/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: concurrency_control/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: client/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

.PHONY: clean
clean:
	rm -f obj/*.o obj/.depend rundb runcl runsq unit_test

#g++ -o $@ $^ -Wall -L. -L./nanomsg-1.1.5/build -L./jemalloc-5.2.1/lib -Wl,-rpath,./jemalloc-5.2.1/lib -pthread -gdwarf-3 -lrt -std=c++0x -lnanomsg ./*.h ./*.cpp system/*.cpp transport/*.cpp benchmarks/*.cpp storage/*.cpp statistics/*.cpp concurrency_control/*.cpp client/*.cpp
