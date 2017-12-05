#-------------------------------------------------
#
# Project created by QtCreator 2014-11-17T09:26:31
#
#-------------------------------------------------

message(*** WickrBot Parser Build)

DEPTH = ../../..

#
# Include the RabbitMQ API Library
#
include($${DEPTH}/libs/qamqpsrc/qamqpsrc.pri)

INCLUDEPATH += $${QAMQP_INCLUDEPATH}
LIBS += -L$${DEPTH}/libs/qamqpsrc $${QAMQP_LIBS}

linux-g++* {
    CONFIG(release,release|debug) {
        QMAKE_RPATHDIR += /usr/lib/wio_welcome_bot
    }
    else {
        wickr_blackout {
            QMAKE_RPATHDIR = /usr/lib/wio_welcome_bot-onprem
        }
        else:wickr_beta {
            QMAKE_RPATHDIR = /usr/lib/wio_welcome_bot-beta
        }
        else:wickr_qa {
            QMAKE_RPATHDIR = /usr/lib/wio_welcome_bot-qa
        }
        else {
            QMAKE_RPATHDIR = /usr/lib/wio_welcome_bot-alpha
        }
    }
}

COMMON = $${DEPTH}/shared/common

wickr_messenger {
    DEFINES += WICKR_MESSENGER=1
}
else:wickr_blackout {
    DEFINES += WICKR_BLACKOUT=1
}
else:wickr_enterprise {
    DEFINES += WICKR_ENTERPRISE=1
}
else:wickr_scif {
    DEFINES += WICKR_SCIF=1
}


CONFIG(release,release|debug) {
    wickr_beta {
        DEFINES += WICKR_BETA
    }
    else {
        DEFINES += WICKR_PRODUCTION
    }
}
else {
    wickr_beta {
        DEFINES += WICKR_BETA
    }
    else:wickr_qa {
        DEFINES += WICKR_QA
    }
    else {
        DEFINES += WICKR_ALPHA
    }

    DEFINES += WICKR_DEBUG
}

#
# Include the WickrBot library
#
include($${DEPTH}/libs/WickrIOLib/WickrIOLib.pri)

INCLUDEPATH += ../client

QT  += core sql
QT  += network
QT  -= gui

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG(release,debug|release) {
    TARGET = welcome_parser
    DEFINES += WICKR_PRODUCTION
}
else {
    wickr_beta {
        TARGET = welcome_parserBeta
    }
    else:wickr_production {
        TARGET = welcome_parser
        DEFINES += WICKR_PRODUCTION
    }
    else {
        TARGET = welcome_parserAlpha
    }

    DEFINES += VERSIONDEBUG
}

# sqlcipher
LIBPATH += $$DEPTH/wickr-sdk/libs/qsqlcipher_wickr/
LIBS += -lsqlcipher
LIBS += -lqsqlcipher_wickr
LIBS += -lsqlite3

SOURCES += \
    main.cpp \
    wbparse_qamqpqueue.cpp \
    wickrbotmain.cpp \
    parseroperationdata.cpp

HEADERS += \
    WickrBotContext.h \
    wbparse_qamqpqueue.h \
    wickrbotmain.h \
    parseroperationdata.h \
    clientconfigurationinfo.h
