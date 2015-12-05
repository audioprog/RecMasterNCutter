#include "savetracks.h"

//#include <QtDebug>
#include <QStringList>
#include <QSettings>
#include "wavfile.h"

SaveTracks::SaveTracks(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&proc, SIGNAL(readyRead()), this, SLOT(canread()));
    QObject::connect(&proc, SIGNAL(finished(int)), this, SLOT(finished()));
    proc.setProcessChannelMode(QProcess::MergedChannels);
    isworking = false;
    ReadSettings();
}

void SaveTracks::canread()
{
    QString out = proc.readAll();
    emit Debug(out);
    //qDebug() << out;
}

void SaveTracks::SaveTrack(int TrackNr, const QString& fileTitle)
{
    //qDebug() << "SaveTrack" << TrackNr << isworking;
    proclist[TrackNr] = QStringList();
    fileTitles[TrackNr] = fileTitle;
    if (!isworking)
        Start();
}

void SaveTracks::ReadSettings()
{
    QSettings settings;
    soxpath = settings.value("soxpath", "").toString();
}

void SaveTracks::finished()
{
    if (proclist.count() > 0 && proclist[proclist.keys().first()].count() > 0)
    {
        QString command = proclist[proclist.keys().first()].takeFirst();
        if (command.contains("%"))
        {
            QString commandend = command.section('%', -1, -1);
            /*QString nr = commandend.section('s', 0, 0);
            bool ok;
            int snr = nr.toInt(&ok);
            if (ok)
            {*/
                int len = WavFile(command.section('\"', 3, 3)).SampleCount();
                command = command.section('%', 0, 0) + QString::number(len) + "s" + commandend.section('s', 1, -1);
            /*}
            else
            {
                command = command.replace("%", "");
            }*/
        }
        if (command.startsWith("del"))
        {
            QFile(command.section('\"', 1, 1)).remove();
            //proc.execute(command);

            if (proclist[proclist.keys().first()].count() == 0)
            {
                int tracknr = proclist.keys().first();
                proclist.remove(tracknr);
                emit Finished(tracknr);
                Start();

                command = "";
            }
            else
                finished();
        }
        emit Debug(command);
        if (command != "")
            proc.start(command);
    }
    else
    {
        int tracknr = proclist.keys().first();
        proclist.remove(tracknr);
        emit Finished(tracknr);
        Start();
    }
}

void SaveTracks::Start()
{
    if (proclist.count() > 0) {
        int ignore = 0;
        int fadein = -1;
        int fadeout = -1;
        for (int i = 0; i < proclist.keys().first(); ++i) {
            if (marks->Type(i) == Marks::StartTrack)
                ignore++;
            else if (marks->Type(i) == Marks::EndTrack)
                ignore--;
            else if (marks->Type(i) == Marks::FadeIn)
                fadein = i;
        }
        int last = 0;
        ignore++;
        for (int i = proclist.keys().first() + 1; i < marks->Count() && ignore > 0; i++) {
            if (marks->Type(i) == Marks::StartTrack)
                ignore++;
            else if (marks->Type(i) == Marks::EndTrack)
                ignore--;
            else if (marks->Type(i) == Marks::FadeIn)
                fadein = i;
            last = i;
        }
        if (last > 0) {
            if (proclist.keys().first() == last - 1) { // No Marks between Start and End of Track
                Save(proclist.keys().first(), -1, -1, last, fileTitles[proclist.keys().first()]);
            }
            else {
                int silent = 0;
                int firstsilent = 0;
                bool havesilent = false;
                for (int i = proclist.keys().first() + 1; i < last; i++) {
                    if (marks->Type(i) == Marks::StartSilence) {
                        silent++;
                        if (firstsilent == 0)
                            firstsilent = i;
                    }
                    else if (marks->Type(i) == Marks::EndSilence) {
                        silent--;
                        havesilent = true;
                    }
                    else if (fadeout == -1 && marks->Type(i) == Marks::FadeOut)
                        fadeout = i;
                }
                if (!havesilent && silent == 0) {
                    Save(proclist.keys().first(), fadein, fadeout, last, fileTitles[proclist.keys().first()]);
                }
                else if (!havesilent && silent > 0) { // silent starts, but not end
                    last = firstsilent; // set Track-end to silent start
                    Save(proclist.keys().first(), fadein, fadeout, last, fileTitles[proclist.keys().first()]);
                }
                else if (havesilent) {
                    SaveMerged(proclist.keys().first(), fadein, fadeout, last, fileTitles[proclist.keys().first()]);
                }
            }
        }
        else {
            int rem = proclist.keys().first();
            proclist.remove(rem);
            Start();
        }
    }
    else
        isworking = false;
}

void SaveTracks::Save(int startmark, int faddin, int faddout, int endmark, const QString& fileTitle)
{
    isworking = true;
    QStringList strlist;
    strlist << "-V3" << "-r" << "44100" << "-s" << "-3" << "-c" << "2" << file->fileName() << "-t" << "wavpcm" << path + fileTitle + ".wav"
                              << "trim" << QString::number(marks->Pos(startmark)) + "s" << QString::number(marks->Pos(endmark) - marks->Pos(startmark)) + "s";
    if (faddin > -1 && faddout > -1)
        strlist << "fade" << "t" << QString::number(marks->Pos(faddin) - marks->Pos(startmark)) + "s" << QString::number(marks->Pos(endmark) - marks->Pos(startmark)) + "s"
                << QString::number(marks->Pos(endmark) - marks->Pos(faddout)) + "s";
                //<< QString::number(marks->Pos(endmark) - marks->Pos(startmark)) + "s"
                //<< QString::number(marks->Pos(endmark) - marks->Pos(faddout)) + "s";
    else if (faddin > -1 && faddout == -1)
        strlist << "fade" << "t" << QString::number(marks->Pos(faddin) - marks->Pos(startmark)) + "s";
    else if (faddout > -1) {
        //strlist << "fade" << "t" << "0" << QString::number(marks->Pos(endmark) - marks->Pos(startmark)) + "s"
        strlist << "fade" << "t" << "0" << "0"
                << QString::number(marks->Pos(endmark) - marks->Pos(faddout)) + "s";
    }
    strlist << "gain" << "-n";

    emit Debug(soxpath + "sox " + strlist.join(" "));
    //Profen und erstellen Dir
    proc.start(soxpath + "sox", strlist);
    //qDebug() << strlist;
}

void SaveTracks::SaveMerged(int startmark, int fadein, int fadeout, int endmark, const QString& fileTitle)
{
    QStringList parts;
    int silence = 0;
    int last = startmark;
    int nr = 0;
    qint64 fadeinlen = 0;
    qint64 fadeoutlen = 0;
    qint64 fadeoutlast = -1;
    qint64 fillen = 0;

    QString mainfilecmd = "\"" + soxpath + "sox\" -r 44100 -s -3 -c 2 \"" + file->fileName() + "\" -t wavpcm \"" + path + fileTitle + ".wav\" trim "
            + QString::number(marks->Pos(startmark)) + "s " + QString::number(marks->Pos(endmark) - marks->Pos(startmark)) + "s";
    parts << mainfilecmd;
    for (int i = startmark + 1; i < endmark; i++) {
        if (marks->Type(i) == Marks::StartSilence) {
            silence++;
            if (silence == 1) {
                if (fadein > -1 && i < fadein)
                    fadeinlen += (marks->Pos(i) - marks->Pos(last));
                if (i > fadeout && fadeoutlast > -1)
                    fadeoutlen += marks->Pos(i) - marks->Pos(fadeoutlast);
                QString part = "\"" + soxpath + "sox\" -t wavpcm \"" + path + fileTitle + ".wav\" -t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + "." + QString::number(++nr) + ".wav\" trim "
                        + QString::number(marks->Pos(last) - marks->Pos(startmark)) + "s " + QString::number(marks->Pos(i) - marks->Pos(last)) + "s";
                fillen += marks->Pos(i) - marks->Pos(last) - 4410;
                parts.append(part);
                last = 0;
            }
        }
        else if (marks->Type(i) == Marks::EndSilence) {
            silence--;
            if (silence == 0) {
                last = i;
                fadeoutlast = i;
            }
        }
        else if (marks->Type(i) == Marks::FadeIn && i <= fadein)
            fadeinlen += (marks->Pos(i) - marks->Pos(last));
        else if (marks->Type(i) == Marks::FadeOut && fadeout == i)
            fadeoutlast = marks->Pos(i);
    }
    if (last > 0) {
        parts << "\"" + soxpath + "sox\" -t wavpcm \"" + path + fileTitle + ".wav\" -t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + "." + QString::number(++nr) + ".wav\" trim "
                + QString::number(marks->Pos(last) - marks->Pos(startmark)) + "s " + QString::number(marks->Pos(endmark) - marks->Pos(last)) + "s";
        fillen += marks->Pos(endmark) - marks->Pos(last);
    }
    fadeoutlen = marks->Pos(endmark) - fadeoutlast;
    parts.append("del \"" + path + fileTitle + ".wav\"");
    if (parts.length() > 0) {

        QString command = "\"" + soxpath + "sox\" ";
        for (int i = 1; i <= nr; i++)
            command += "\"" + path + QString::number(marks->Pos(startmark)) + "." + QString::number(i) + ".wav\" ";
        if (fadein == -1 && fadeout == -1)
            command += "-t wavpcm \"" + path + fileTitle + ".wav\" splice 4410s gain -n silence 1 0.5 -67d -1 1 -71d";
        else
            command += "-t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\" splice 4410s gain -n silence 1 0.5 -67d -1 1 -71d";
        parts << command;

        for (int i = 1; i <= nr; i++)
            parts << "del \"" + path.replace('/', '\\') + QString::number(marks->Pos(startmark)) + "." + QString::number(i) + ".wav\"";

        if (fadein > -1 && fadeout > -1) {
            parts << "\"" + soxpath + "sox\" -t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\" -t wavpcm \"" + path + fileTitle + ".wav\" fade t " + QString::number(fadeinlen) + "s %" + QString::number(fillen) + "s " + QString::number(fadeoutlen) + "s";
            parts << "del \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\"";
            //parts << "fade t " + QString::number(fadeinlen) + "s 0 " + QString::number(fadeoutlen) + "s";
        }
        else if (fadein > -1)
        {
            parts << "\"" + soxpath + "sox\" -t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\" -t wavpcm \"" + path + fileTitle + ".wav\" fade t " + QString::number(fadeinlen) + "s";
            parts << "del \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\"";
        }
        else if (fadeout > -1)
        {
            parts << "\"" + soxpath + "sox\" -t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\" -t wavpcm \"" + path + fileTitle + ".wav\" fade t 0 %" + QString::number(fillen) + "s " + QString::number(fadeoutlen) + "s";
            parts << "del \"" + path + QString::number(marks->Pos(startmark)) + ".tofade.wav\"";
        }
            //parts << "fade t 0 0 " + QString::number(fadeoutlen) + "s";
        //if (fadein > -1 || fadeout > -1)
        //    parts << "& del \"" + path + QString::number(marks->Pos(startmark)).replace('/', '\\') + ".bf.wav\"";

        QString cmd = parts.takeFirst();
        proclist[startmark] = parts;
        proc.start(cmd);
/*#ifdef Q_OS_WIN32
        emit Debug("cmd /c " + parts.join(" "));
        proc.start("cmd /c " + parts.join(" "));
#else
        emit Debug(parts.join(" "));
        proc.start(parts.join(" "));
#endif*/
    }
}
