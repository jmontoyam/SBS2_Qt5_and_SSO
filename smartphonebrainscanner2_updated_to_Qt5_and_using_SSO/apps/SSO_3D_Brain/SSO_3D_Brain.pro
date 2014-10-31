SOURCES += \
    main.cpp \
    openglwindow.cpp \
    glm.cpp \
    glwidget.cpp \
    model.cpp \
    mainwindow.cpp \
    mycallback.cpp \
    mycallback_mf.cpp \
    sparsereconstruction_mf.cpp

HEADERS += \
    openglwindow.h \
    colordata.h \
    glm.h \
    glwidget.h \
    model.h \
    mainwindow.h \
    mycallback.h \
    mycallback_mf.h \
    sparsereconstruction_mf.h

QT += widgets concurrent

QMAKE_CXXFLAGS += -DNDEBUG

QMAKE_CXXFLAGS += -std=c++11

include(../../core/src/sbs2_binary_decryptor.pri)

RESOURCES += \
    resources.qrc

#Path to Eigen Library
INCLUDEPATH += "/home/user/Documents/ProgrammingLibraries/Cplusplus/eigen_3_2_2"

#Path to SSO folder
INCLUDEPATH += "/home/user/Desktop/SSO_Git"

#Include the sbs2-emotiv-decryptor library (
#please read:
#https://github.com/SmartphoneBrainScanner/smartphonebrainscanner2-core/wiki/Emotiv-EPOC)
