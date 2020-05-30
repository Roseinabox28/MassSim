
SOURCES += masses.cc MassWindow.cc Mass.cc

HEADERS += MassWindow.h Mass.h

TARGET = masses
TEMPLATE = app

FORMS += MassWindow.ui

QMAKE_CXXFLAGS += -std=c++11
