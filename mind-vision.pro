QT += gui
CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:INCLUDEPATH += "C:\Program Files (x86)\MindVision\Demo\VC++\Include"

unix:LIBS += -lMVSDK
win32:LIBS += -L"C:\Program Files (x86)\MindVision\SDK\X64" -lMVCAMSDK_X64

HEADERS += \
    brightness.h \
    defectpixelalg.h \
    mindvision.h \
    recordthread.h \
    snapshotthread.h

SOURCES += \
        main.cpp \
        mindvision.cpp \
        recordthread.cpp \
        snapshotthread.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
