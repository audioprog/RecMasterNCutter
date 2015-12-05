INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/zlib-1.2.8
HEADERS += \
        $$PWD/crypt.h \
        $$PWD/ioapi.h \
        $$PWD/JlCompress.h \
        $$PWD/quaadler32.h \
        $$PWD/quachecksum32.h \
        $$PWD/quacrc32.h \
        $$PWD/quagzipfile.h \
        $$PWD/quaziodevice.h \
        $$PWD/quazipdir.h \
        $$PWD/quazipfile.h \
        $$PWD/quazipfileinfo.h \
        $$PWD/quazip_global.h \
        $$PWD/quazip.h \
        $$PWD/quazipnewinfo.h \
        $$PWD/unzip.h \
        $$PWD/zip.h \
    $$PWD/zlib-1.2.8/crc32.h \
    $$PWD/zlib-1.2.8/deflate.h \
    $$PWD/zlib-1.2.8/gzguts.h \
    $$PWD/zlib-1.2.8/inffast.h \
    $$PWD/zlib-1.2.8/inffixed.h \
    $$PWD/zlib-1.2.8/inflate.h \
    $$PWD/zlib-1.2.8/inftrees.h \
    $$PWD/zlib-1.2.8/trees.h \
    $$PWD/zlib-1.2.8/zconf.h \
    $$PWD/zlib-1.2.8/zlib.h \
    $$PWD/zlib-1.2.8/zutil.h

SOURCES += $$PWD/qioapi.cpp \
           $$PWD/JlCompress.cpp \
           $$PWD/quaadler32.cpp \
           $$PWD/quacrc32.cpp \
           $$PWD/quagzipfile.cpp \
           $$PWD/quaziodevice.cpp \
           $$PWD/quazip.cpp \
           $$PWD/quazipdir.cpp \
           $$PWD/quazipfile.cpp \
           $$PWD/quazipfileinfo.cpp \
           $$PWD/quazipnewinfo.cpp \
           $$PWD/unzip.c \
           $$PWD/zip.c \
    $$PWD/zlib-1.2.8/adler32.c \
    $$PWD/zlib-1.2.8/compress.c \
    $$PWD/zlib-1.2.8/crc32.c \
    $$PWD/zlib-1.2.8/deflate.c \
    $$PWD/zlib-1.2.8/gzclose.c \
    $$PWD/zlib-1.2.8/gzlib.c \
    $$PWD/zlib-1.2.8/gzread.c \
    $$PWD/zlib-1.2.8/gzwrite.c \
    $$PWD/zlib-1.2.8/infback.c \
    $$PWD/zlib-1.2.8/inffast.c \
    $$PWD/zlib-1.2.8/inflate.c \
    $$PWD/zlib-1.2.8/inftrees.c \
    $$PWD/zlib-1.2.8/trees.c \
    $$PWD/zlib-1.2.8/uncompr.c \
    $$PWD/zlib-1.2.8/zutil.c

win32:DEFINES += NOMINMAX
