#-------------------------------------------------
#
# Project created by QtCreator 2011-03-13T08:22:24
#
#-------------------------------------------------

QT += core gui
#QT += network
QT += multimedia widgets

TARGET = RecMasterNCutter
TEMPLATE = app

INCLUDEPATH += $$PWD/quazip
DEPENDPATH += $$PWD/quazip

DEFINES += QUAZIP_STATIC

SOURCES += main.cpp\
        mainwindow.cpp \
    waveform.cpp \
    wavedata.cpp \
    marks.cpp \
    markimages.cpp \
    icondelegate.cpp \
    savetracks.cpp \
    audiooutput.cpp \
    wavfile.cpp \
    optionsdialog.cpp \
    buttonstateitemdelegate.cpp \
    buttonstate.cpp \
    buttonstatewidget.cpp \
    quazip/zip.c \
    quazip/unzip.c \
    quazip/quazipnewinfo.cpp \
    quazip/quazipfile.cpp \
    quazip/quazip.cpp \
    quazip/quacrc32.cpp \
    quazip/quaadler32.cpp \
    quazip/qioapi.cpp \
    quazip/JlCompress.cpp \
    ziprw.cpp

HEADERS  += mainwindow.h \
    waveform.h \
    wavedata.h \
    marks.h \
    markimages.h \
    icondelegate.h \
    savetracks.h \
    audiooutput.h \
    wavfile.h \
    optionsdialog.h \
    buttonstateitemdelegate.h \
    buttonstate.h \
    buttonstatewidget.h \
    quazip/zip.h \
    quazip/unzip.h \
    quazip/quazipnewinfo.h \
    quazip/quazipfileinfo.h \
    quazip/quazipfile.h \
    quazip/quazip_global.h \
    quazip/quazip.h \
    quazip/quacrc32.h \
    quazip/quachecksum32.h \
    quazip/quaadler32.h \
    quazip/JlCompress.h \
    quazip/ioapi.h \
    quazip/crypt.h \
    quazip/zconf.h \
    quazip/zlib.h \
    ziprw.h

FORMS    += mainwindow.ui \
    optionsdialog.ui

OTHER_FILES += \
    icons/go-next-24.png \
    icons/go-last-24.png \
    icons/go-first-24.png \
    icons/go-previous-24.png \
    icons/save.png \
    icons/open.png \
    icons/delete.png \
    icons/standardbutton-closetab-16.png \
    icons/Start_Track.png \
    icons/Start_Silence.png \
    icons/Standard.png \
    icons/End_Track.png \
    icons/End_Silence.png \
    icons/Add_Actual.png \
    icons/Add_3.png \
    icons/Add_2.png \
    icons/Add_1.png \
    icons/go-pos1-24.png \
    icons/go-end-24.png \
    icons/saveWave.png \
    icons/Add_4.png \
    icons/zoom-out.svg \
    icons/zoom-in.svg \
    icons/media_playlist_refresh.png \
    icons/waveform.png \
    myapp.rc \
    icons/icon.png \
    icon.ico \
    icons/player_stop.png \
    icons/player_play.png \
    buttonstyleicons/WaveDiskette.png \
    buttonstyleicons/WaveDiscTick24.png \
    buttonstyleicons/WaveDiscGearShaddow24.png \
    buttonstyleicons/WaveDiscGear24.png \
    buttonstyleicons/notecompressTick24.png \
    buttonstyleicons/notecompressGearShaddow24.png \
    buttonstyleicons/notecompressGear24.png \
    buttonstyleicons/notecompress24.png \
    buttonstyleicons/fileopen_proc.png \
    buttonstyleicons/fileopen.png \
    icons/fadeIn.png \
    icons/fadeOut.png \
    lgpl-3.0.txt \
    icons/rec.png \
    icons/openrec.png \
    icons/player_play_atline.png \
    RecMasterNCutter_de.ts \
	RecMasterNCutter_de.qm \
    icons/Cd-Edit-32.png \
    icons/saveTxt.png \
    icons/newAudioCDProj.png \
    icons/HandautoPlaze.png \
    icons/End_FadeOut.png \
    icons/Start_FadeIn.png

RESOURCES += \
    icons.qrc
RC_FILE = myapp.rc

TRANSLATIONS = RecMasterNCutter_de.ts

unix:!symbian {
    LIBS += -lz
}
LIBS += -L$$PWD/quazip/ -lzlibstat
PRE_TARGETDEPS += $$PWD/quazip/zlibstat.lib

DEPENDPATH += $$PWD/quazip
