######################################################################
# Automatically generated by qmake (3.1) Wed Feb 15 18:06:31 2017
######################################################################

TEMPLATE = app
TARGET = test
INCLUDEPATH += . ../include ../../ /usr/local/include/

QT += testlib network
QT -= gui widgets
CONFIG += c++11 debug
CONFIG -= app_bundle

LIBS += -L../lib/ -lblds-client

mac {
	QMAKE_RPATHDIR += $$(PWD)/../lib
}

# Input
HEADERS += test-libblds-client.h
SOURCES += test-libblds-client.cc