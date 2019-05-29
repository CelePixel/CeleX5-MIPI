#############################################################################
# Makefile for building: libCeleX.so
#############################################################################

MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DCELEX_LIBRARY
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -std=gnu++0x -Wall -W -D_REENTRANT -fPIC $(DEFINES)
INCPATH       = -I../CeleX -I.
DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
COPY          = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
TAR           = tar -cf
COMPRESS      = gzip -9f
LINK          = g++
LFLAGS        = -Wl,-O1 -Wl,-rpath, -shared -Wl,-soname,libCeleX.so
LIBS          = $(SUBLIBS) -lpthread `pkg-config --cflags --libs opencv`
AR            = ar cqs
RANLIB        = 
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files


SOURCES       = ./base/xthread.cpp \
		./base/xbase.cpp \
		./base/dataqueue.cpp \
		./configproc/tinyxml/tinyxmlparser.cpp \
		./configproc/tinyxml/tinyxmlerror.cpp \
		./configproc/tinyxml/tinyxml.cpp \
		./configproc/tinyxml/tinystr.cpp \
		./eventproc/eventprocessing.cpp \
		./eventproc/datarecorder.cpp \
		./eventproc/dataprocessthread.cpp \
		./eventproc/celex5processeddata.cpp \
		./eventproc/celex5dataprocessor.cpp \
		./eventproc/celex5.cpp \
		./configproc/hhxmlreader.cpp \
		./configproc/hhwireincommand.cpp \
		./configproc/hhsequencemgr.cpp \
		./configproc/hhcommand.cpp 
OBJECTS       = xthread.o \
		xbase.o \
		dataqueue.o \
		tinyxmlparser.o \
		tinyxmlerror.o \
		tinyxml.o \
		tinystr.o \
		eventprocessing.o \
		datarecorder.o \
		dataprocessthread.o \
		celex5processeddata.o \
		celex5dataprocessor.o \
		celex5.o \
		hhxmlreader.o \
		hhwireincommand.o \
		hhsequencemgr.o \
		hhcommand.o

TARGET        = libCeleX.so
TARGETA       = libCeleX.a


first: all
####### Build rules

$(TARGET):  $(OBJECTS) $(SUBLIBS) $(OBJCOMP)  
	-$(DEL_FILE) $(TARGET)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(OBJCOMP)


staticlib: $(TARGETA)

$(TARGETA):  $(OBJECTS) $(OBJCOMP) 
	-$(DEL_FILE) $(TARGETA) 
	$(AR) $(TARGETA) $(OBJECTS)

all: Makefile $(TARGET)

clean: compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


distclean: clean 
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) $(TARGET0) $(TARGET1) $(TARGET2) $(TARGETA)
	-$(DEL_FILE) Makefile


####### Sub-libraries
compiler_clean: 

####### Compile

xthread.o: ./base/xthread.cpp ./base/xthread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o xthread.o ./base/xthread.cpp

xbase.o: ./base/xbase.cpp ./base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o xbase.o ./base/xbase.cpp

dataqueue.o: ./base/dataqueue.cpp ./base/dataqueue.h \
		./include/celextypes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o dataqueue.o ./base/dataqueue.cpp

tinyxmlparser.o: ./configproc/tinyxml/tinyxmlparser.cpp ./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlparser.o ./configproc/tinyxml/tinyxmlparser.cpp

tinyxmlerror.o: ./configproc/tinyxml/tinyxmlerror.cpp ./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlerror.o ./configproc/tinyxml/tinyxmlerror.cpp

tinyxml.o: ./configproc/tinyxml/tinyxml.cpp ./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxml.o ./configproc/tinyxml/tinyxml.cpp

tinystr.o: ./configproc/tinyxml/tinystr.cpp ./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinystr.o ./configproc/tinyxml/tinystr.cpp

eventprocessing.o: ./eventproc/eventprocessing.cpp ./include/celextypes.h \
		./include/dvslib/eventproc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o eventprocessing.o ./eventproc/eventprocessing.cpp

datarecorder.o: ./eventproc/datarecorder.cpp ./eventproc/datarecorder.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o datarecorder.o ./eventproc/datarecorder.cpp

dataprocessthread.o: ./eventproc/dataprocessthread.cpp ./eventproc/dataprocessthread.h \
		./base/dataqueue.h \
		./base/xthread.h \
		./include/celextypes.h \
		./eventproc/celex5dataprocessor.h \
		./include/celex5/celex5.h \
		./include/celex5/celex5processeddata.h \
		./include/celex5/celex5datamanager.h \
		./eventproc/celex5datareader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o dataprocessthread.o ./eventproc/dataprocessthread.cpp

celex5processeddata.o: ./eventproc/celex5processeddata.cpp ./include/celex5/celex5processeddata.h \
		./include/celex5/celex5.h \
		./include/celextypes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5processeddata.o ./eventproc/celex5processeddata.cpp

celex5dataprocessor.o: ./eventproc/celex5dataprocessor.cpp ./eventproc/celex5dataprocessor.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./include/celex5/celex5processeddata.h \
		./include/celex5/celex5datamanager.h \
		./eventproc/celex5datareader.h \
		./base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5dataprocessor.o ./eventproc/celex5dataprocessor.cpp

celex5.o: ./eventproc/celex5.cpp ./include/celex5/celex5.h \
		./include/celextypes.h \
		./base/xbase.h \
		./driver/CeleDriver.h \
		./configproc/hhsequencemgr.h \
		./configproc/hhwireincommand.h \
		./configproc/hhcommand.h \
		./eventproc/dataprocessthread.h \
		./base/dataqueue.h \
		./base/xthread.h \
		./eventproc/celex5dataprocessor.h \
		./include/celex5/celex5processeddata.h \
		./include/celex5/celex5datamanager.h \
		./eventproc/celex5datareader.h \
		./eventproc/datarecorder.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5.o ./eventproc/celex5.cpp

hhxmlreader.o: ./configproc/hhxmlreader.cpp ./configproc/hhxmlreader.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./base/xbase.h \
		./configproc/hhwireincommand.h \
		./configproc/hhcommand.h \
		./configproc/hhsequencemgr.h \
		./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhxmlreader.o ./configproc/hhxmlreader.cpp

hhwireincommand.o: ./configproc/hhwireincommand.cpp ./configproc/hhwireincommand.h \
		./configproc/hhcommand.h \
		./base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhwireincommand.o ./configproc/hhwireincommand.cpp

hhsequencemgr.o: ./configproc/hhsequencemgr.cpp ./configproc/hhsequencemgr.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./configproc/hhcommand.h \
		./configproc/hhxmlreader.h \
		./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhsequencemgr.o ./configproc/hhsequencemgr.cpp

hhcommand.o: ./configproc/hhcommand.cpp ./configproc/hhcommand.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhcommand.o ./configproc/hhcommand.cpp



