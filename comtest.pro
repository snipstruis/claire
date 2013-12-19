TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
	main.cpp

QMAKE_CXXFLAGS += -g --std=c++11 -pthread

LIBS += -pthread


HEADERS += \
	connection.hpp \
	job.hpp \
	mandelbrot.hpp \
	utils.hpp

OTHER_FILES +=

