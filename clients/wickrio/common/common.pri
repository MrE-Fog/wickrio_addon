CONFIG(release,release|debug) {
    wickr_prod:DEFINES += WICKR_TARGET_PROD
    else:DEFINES += WICKR_TARGET_PREVIEW
}
else {
    wickr_beta:DEFINES += WICKR_TARGET_BETA
    else:DEFINES += WICKR_TARGET_ALPHA
}

HEADERS += \
    $${PWD}/wbio_common.h \
    $${PWD}/wickrioappsettings.h \
    $${PWD}/wickriodatabase.h \
    $$PWD/wickrioconsoleuser.h \
    $$PWD/wickrioclients.h \
    $$PWD/wickrioapi.h \
    $$PWD/wickriotokens.h \
    $$PWD/wickriomessage.h \
    $$PWD/wickrbotclientdatabase.h

SOURCES += \
    $${PWD}/wbio_common.cpp \
    $${PWD}/wickrioappsettings.cpp \
    $${PWD}/wickriodatabase.cpp \
    $$PWD/wickrioconsoleuser.cpp \
    $$PWD/wickriotokens.cpp \
    $$PWD/wickriomessage.cpp \
    $$PWD/wickrbotclientdatabase.cpp

INCLUDEPATH += $${PWD}
INCLUDEPATH += $${PWD}/../../common
INCLUDEPATH += $${PWD}/$${DEPTH}/src
INCLUDEPATH += $${PWD}/$${DEPTH}/export
INCLUDEPATH += $$PWD/$${DEPTH}/libs/WickrProto

linux-g++* {
    INCLUDEPATH += $${PWD}/$${DEPTH}/platforms/linux/include
}

win32 {
    INCLUDEPATH += $${PWD}/$${DEPTH}/platforms/win/include
}

linux-g++* {
    equals(QT_MAJOR_VERSION, 5) {
        equals(QT_MINOR_VERSION, 7) {
            QMAKE_RPATHDIR = /usr/local/wickr/Qt-5.7
            QMAKE_RPATHDIR += /usr/local/wickr/Qt-5.7/lib
        }
        equals(QT_MINOR_VERSION, 8) {
            QMAKE_RPATHDIR = /usr/local/wickr/Qt-5.8
            QMAKE_RPATHDIR += /usr/local/wickr/Qt-5.8/lib
        }
    }
    CONFIG(release,release|debug) {
        QMAKE_RPATHDIR += /usr/lib/wickrio
    }
    else {
        wickr_beta:QMAKE_RPATHDIR += /usr/lib/wickrio-beta
        else:wickr_qa:QMAKE_RPATHDIR += /usr/lib/wickrio-qa
        else:QMAKE_RPATHDIR += /usr/lib/wickrio-alpha
    }

    message(*** QMAKE_RPATHDIR = $$QMAKE_RPATHDIR)
}
