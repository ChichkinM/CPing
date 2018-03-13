INCLUDEPATH += $$PWD

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += $$PWD/cping.cpp
HEADERS += \
    $$PWD/cping.h \
    $$PWD/icpingos.h

win32 {
    LIBS += -liphlpapi -lws2_32
    SOURCES += $$PWD/cpingwindows.cpp
    HEADERS += $$PWD/cpingwindows.h
}
