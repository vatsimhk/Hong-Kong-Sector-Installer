QT       += core gui
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    colourthemepicker.cpp \
    libgit2_callbacks.cpp \
    main.cpp \
    mainwindow.cpp \
    mergeconflictdialog.cpp \
    migratedialog.cpp \
    optionsdialog.cpp \
    repairdialog.cpp

HEADERS += \
    colourthemepicker.h \
    libgit2_callbacks.h \
    mainwindow.h \
    mergeconflictdialog.h \
    migratedialog.h \
    optionsdialog.h \
    repairdialog.h

FORMS += \
    colourthemepicker.ui \
    mainwindow.ui \
    mergeconflictdialog.ui \
    migratedialog.ui \
    optionsdialog.ui \
    repairdialog.ui

INCLUDEPATH += "C:/Users/Justin Wai/source/repos/libgit2-1.7.1/build/include"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../source/repos/libgit2-1.7.1/build/debug/ -lgit2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../source/repos/libgit2-1.7.1/build/debug/ -lgit2

INCLUDEPATH += $$PWD/../../../source/repos/libgit2-1.7.1/build/Debug
DEPENDPATH += $$PWD/../../../source/repos/libgit2-1.7.1/build/Debug

RESOURCES += \
    resources.qrc
