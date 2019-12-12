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
LIBS          = $(SUBLIBS) -lpthread -L/usr/lib/x86_64-linux-gnu/ -lusb-1.0 `pkg-config --cflags --libs opencv`
AR            = ar cqs
RANLIB        = 
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files


SOURCES       = ./base/xthread.cpp \
		./base/filedirectory.cpp\
		./base/dataqueue.cpp \
		./configproc/tinyxml/tinyxmlparser.cpp \
		./configproc/tinyxml/tinyxmlerror.cpp \
		./configproc/tinyxml/tinyxml.cpp \
		./configproc/tinyxml/tinystr.cpp \
		./eventproc/datarecorder.cpp \
		./eventproc/dataprocessthread.cpp \
		./eventproc/celex5processeddata.cpp \
		./eventproc/celex5dataprocessor.cpp \
		./eventproc/celex5.cpp \
		./configproc/xmlreader.cpp \
		./configproc/wireincommand.cpp \
		./configproc/celex5cfgmgr.cpp \
		./cx3driver/usbinterface.cpp \
		./cx3driver/package.cpp \
		./cx3driver/cypress.cpp \
		./cx3driver/celedriver.cpp \
		./cx3driver/bulktransfer.cpp 
OBJECTS       = xthread.o \
		filedirectory.o \
		dataqueue.o \
		tinyxmlparser.o \
		tinyxmlerror.o \
		tinyxml.o \
		tinystr.o \
		datarecorder.o \
		dataprocessthread.o \
		celex5processeddata.o \
		celex5dataprocessor.o \
		celex5.o \
		xmlreader.o \
		wireincommand.o \
		celex5cfgmgr.o \
		usbinterface.o \
		package.o \
		cypress.o \
		celedriver.o \
		bulktransfer.o

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

filedirectory.o: ./base/filedirectory.cpp ./base/filedirectory.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o filedirectory.o ./base/filedirectory.cpp

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

datarecorder.o: ./eventproc/datarecorder.cpp ./eventproc/datarecorder.h \
		./include/celex5/celex5.h \
		./include/celextypes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o datarecorder.o ./eventproc/datarecorder.cpp

dataprocessthread.o: ./eventproc/dataprocessthread.cpp ./eventproc/dataprocessthread.h \
		./base/dataqueue.h \
		./base/xthread.h \
		./include/celextypes.h \
		./eventproc/celex5dataprocessor.h \
		./include/celex5/celex5.h \
		./include/celex5/celex5processeddata.h \
		./include/celex5/celex5datamanager.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o dataprocessthread.o ./eventproc/dataprocessthread.cpp

celex5processeddata.o: ./eventproc/celex5processeddata.cpp ./include/celex5/celex5processeddata.h \
		./include/celex5/celex5.h \
		./include/celextypes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5processeddata.o ./eventproc/celex5processeddata.cpp

celex5dataprocessor.o: ./eventproc/celex5dataprocessor.cpp ./eventproc/celex5dataprocessor.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./include/celex5/celex5processeddata.h \
		./include/celex5/celex5datamanager.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5dataprocessor.o ./eventproc/celex5dataprocessor.cpp

celex5.o: ./eventproc/celex5.cpp ./include/celex5/celex5.h \
		./include/celextypes.h \
		./cx3driver/celedriver.h \
		./configproc/celex5cfgmgr.h \
		./configproc/wireincommand.h \
		./eventproc/dataprocessthread.h \
		./base/dataqueue.h \
		./base/xthread.h \
		./eventproc/celex5dataprocessor.h \
		./include/celex5/celex5processeddata.h \
		./include/celex5/celex5datamanager.h \
		./eventproc/datarecorder.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5.o ./eventproc/celex5.cpp

xmlreader.o: ./configproc/xmlreader.cpp ./configproc/xmlreader.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./configproc/wireincommand.h \
		./configproc/celex5cfgmgr.h \
		./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o xmlreader.o ./configproc/xmlreader.cpp

wireincommand.o: ./configproc/wireincommand.cpp ./configproc/wireincommand.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o wireincommand.o ./configproc/wireincommand.cpp

celex5cfgmgr.o: ./configproc/celex5cfgmgr.cpp ./configproc/celex5cfgmgr.h \
		./include/celex5/celex5.h \
		./include/celextypes.h \
		./configproc/xmlreader.h \
		./configproc/tinyxml/tinyxml.h \
		./configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5cfgmgr.o ./configproc/celex5cfgmgr.cpp


usbinterface.o: ./cx3driver/usbinterface.cpp ./cx3driver/usbinterface.h \
		./cx3driver/bulktransfer.h \
		./cx3driver/include/libusb.h \
		./cx3driver/package.h \
		./cx3driver/celedriver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o usbinterface.o ./cx3driver/usbinterface.cpp

package.o: ./cx3driver/package.cpp ./cx3driver/package.h \
		./cx3driver/celedriver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o package.o ./cx3driver/package.cpp

cypress.o: ./cx3driver/cypress.cpp ./cx3driver/cypress.h \
		./cx3driver/usbinterface.h \
		./cx3driver/bulktransfer.h \
		./cx3driver/include/libusb.h \
		./cx3driver/package.h \
		./cx3driver/celedriver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o cypress.o ./cx3driver/cypress.cpp

celedriver.o: ./cx3driver/celedriver.cpp ./cx3driver/celedriver.h \
		./cx3driver/cypress.h \
		./cx3driver/usbinterface.h \
		./cx3driver/bulktransfer.h \
		./cx3driver/include/libusb.h \
		./cx3driver/package.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celedriver.o ./cx3driver/celedriver.cpp

bulktransfer.o: ./cx3driver/bulktransfer.cpp ./cx3driver/bulktransfer.h \
		./cx3driver/include/libusb.h \
		./cx3driver/package.h \
		./cx3driver/celedriver.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o bulktransfer.o ./cx3driver/bulktransfer.cpp

