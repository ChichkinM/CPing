INCLUDEPATH += $$PWD

QMAKE_CXXFLAGS += -std=c++0x

win {
    LIBS += -liphlpapi -lws2_32
}

SOURCES += \
    $$PWD/cping.cpp \
    $$PWD/cpingwindows.cpp

HEADERS += \
    $$PWD/cping.h \
    $$PWD/cpingwindows.h \
    $$PWD/icpingos.h
