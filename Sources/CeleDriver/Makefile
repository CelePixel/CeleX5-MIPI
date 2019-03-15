#############################################################################
# Makefile for building: libCeleDriver.so
#############################################################################

MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DCELEDRIVER_LIBRARY
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -std=gnu++0x -Wall -W -D_REENTRANT -fPIC $(DEFINES)
INCPATH       = -I../CeleDriver -I. -I/usr/include/libusb-1.0
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
DISTDIR = 
LINK          = g++
LFLAGS        = -Wl,-O1 -shared -Wl,-soname,libCeleDriver.so
LIBS          = $(SUBLIBS) /usr/lib/x86_64-linux-gnu/libusb-1.0.so -lpthread 
AR            = ar cqs
RANLIB        = 
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = ../CeleDriver/VideoThread.cpp \
		../CeleDriver/USBIR.cpp \
		../CeleDriver/Package.cpp \
		../CeleDriver/Cypress.cpp \
		../CeleDriver/CeleDriver.cpp \
		../CeleDriver/BulkTransfer.cpp 
OBJECTS       = VideoThread.o \
		USBIR.o \
		Package.o \
		Cypress.o \
		CeleDriver.o \
		BulkTransfer.o
DIST          = ../CeleDriver/CeleDriver.pro VideoThread.h \
		USBIR.h \
		Package.h \
		Cypress.h \
		CeleDriver.h \
		BulkTransfer.h ../CeleDriver/VideoThread.cpp \
		../CeleDriver/USBIR.cpp \
		../CeleDriver/Package.cpp \
		../CeleDriver/Cypress.cpp \
		../CeleDriver/CeleDriver.cpp \
		../CeleDriver/BulkTransfer.cpp

DESTDIR       = 
TARGET        = libCeleDriver.so
TARGETA       = libCeleDriver.a


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


####### Sub-libraries

compiler_clean: 

####### Compile

VideoThread.o: ../CeleDriver/VideoThread.cpp ../CeleDriver/VideoThread.h \
		../CeleDriver/BulkTransfer.h \
		/usr/include/libusb-1.0/libusb.h \
		../CeleDriver/Package.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o VideoThread.o ../CeleDriver/VideoThread.cpp

USBIR.o: ../CeleDriver/USBIR.cpp ../CeleDriver/USBIR.h \
		/usr/include/libusb-1.0/libusb.h \
		../CeleDriver/BulkTransfer.h \
		../CeleDriver/Package.h \
		../CeleDriver/VideoThread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o USBIR.o ../CeleDriver/USBIR.cpp

Package.o: ../CeleDriver/Package.cpp ../CeleDriver/Package.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o Package.o ../CeleDriver/Package.cpp

Cypress.o: ../CeleDriver/Cypress.cpp ../CeleDriver/Cypress.h \
		../CeleDriver/USBIR.h \
		/usr/include/libusb-1.0/libusb.h \
		../CeleDriver/BulkTransfer.h \
		../CeleDriver/Package.h \
		../CeleDriver/VideoThread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o Cypress.o ../CeleDriver/Cypress.cpp

CeleDriver.o: ../CeleDriver/CeleDriver.cpp ../CeleDriver/CeleDriver.h \
		../CeleDriver/Cypress.h \
		../CeleDriver/USBIR.h \
		/usr/include/libusb-1.0/libusb.h \
		../CeleDriver/BulkTransfer.h \
		../CeleDriver/Package.h \
		../CeleDriver/VideoThread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CeleDriver.o ../CeleDriver/CeleDriver.cpp

BulkTransfer.o: ../CeleDriver/BulkTransfer.cpp ../CeleDriver/BulkTransfer.h \
		/usr/include/libusb-1.0/libusb.h \
		../CeleDriver/Package.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o BulkTransfer.o ../CeleDriver/BulkTransfer.cpp


