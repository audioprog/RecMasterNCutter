#include "savetracks.h"

//#include <QtDebug>
#include <QStringList>

SaveTracks::SaveTracks(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&proc, SIGNAL(readyRead()), this, SLOT(canread()));
    QObject::connect(&proc, SIGNAL(finished(int)), this, SLOT(finished()));
    proc.setProcessChannelMode(QProcess::MergedChannels);
    isworking = false;
}

void SaveTracks::canread()
{
    QString out = proc.readAll();
    emit Debug(out);
    //qDebug() << out;
}

void SaveTracks::SaveTrack(int TrackNr)
{
    //qDebug() << "SaveTrack" << TrackNr << isworking;
    list.append(TrackNr);
    if (!isworking)
        Start();
}

void SaveTracks::finished()
{
    emit Finished(list.takeFirst());
    Start();
}

void SaveTracks::Start()
{
    if (list.count() > 0) {
        int ignore = 0;
        for (int i = 0; i < list.first(); ++i) {
            if (marks->Type(i) == Marks::StartTrack)
                ignore++;
            else if (marks->Type(i) == Marks::EndTrack)
                ignore--;
        }
        int last = 0;
        ignore++;
        for (int i = list.first() + 1; i < marks->Count() && ignore > 0; i++) {
            if (marks->Type(i) == Marks::StartTrack)
                ignore++;
            else if (marks->Type(i) == Marks::EndTrack)
                ignore--;
            last = i;
        }
        if (last > 0) {
            if (list.first() == last - 1) { // No Marks between Start and End of Track
                Save(list.first(), last);
            }
            else {
                int silent = 0;
                int firstsilent = 0;
                bool havesilent = false;
                for (int i = list.first() + 1; i < last; i++) {
                    if (marks->Type(i) == Marks::StartSilence) {
                        silent++;
                        if (firstsilent == 0)
                            firstsilent = i;
                    }
                    else if (marks->Type(i) == Marks::EndSilence) {
                        silent--;
                        havesilent = true;
                    }
                }
                if (!havesilent && silent == 0) {
                    Save(list.first(), last);
                }
                else if (!havesilent && silent > 0) { // silent starts, but not end
                    last = firstsilent; // set Track-end to silent start
                    Save(list.first(), last);
                }
                else if (havesilent) {
                    SaveMerged(list.first(), last);
                }
            }
        }
        else {
            list.removeFirst();
            Start();
        }
    }
    else
        isworking = false;
}

void SaveTracks::Save(int startmark, int endmark)
{
    isworking = true;
    QStringList strlist;
    strlist << "-V3" << "-r" << "44100" << "-s" << "-3" << "-c" << "2" << file->fileName() << "-t" << "wavpcm" << path + QString::number(marks->Pos(startmark)) + ".wav"
                              << "trim" << QString::number(marks->Pos(startmark)) + "s" << QString::number(marks->Pos(endmark) - marks->Pos(startmark)) + "s"
                              << "gain" << "-n";
    //Prüfen und erstellen Dir
    proc.start(soxpath + "sox", strlist);
    //qDebug() << strlist;
}

void SaveTracks::SaveMerged(int startmark, int endmark)
{
    QStringList parts;
    int silence = 0;
    int last = startmark;
    int nr = 0;
    for (int i = startmark + 1; i < endmark; i++) {
        if (marks->Type(i) == Marks::StartSilence) {
            silence++;
            if (silence == 1) {
                QString part = soxpath + "sox -r 44100 -s -3 -c 2 \"" + file->fileName() + "\" -t wavpcm \"" + file->fileName() + "." + QString::number(++nr) + ".wav\" trim "
                        + QString::number(marks->Pos(last)) + "s " + QString::number(marks->Pos(i) - marks->Pos(last)) + "s &";
                parts.append(part);
                last = 0;
            }
        }
        else if (marks->Type(i) == Marks::EndSilence) {
            silence--;
            if (silence == 0)
                last = i;
        }
    }
    if (last > 0) {
        parts << soxpath + "sox -r 44100 -s -3 -c 2 \"" + file->fileName() + "\" -t wavpcm \"" + file->fileName() + "." + QString::number(++nr) + ".wav\" trim "
                + QString::number(marks->Pos(last)) + "s " + QString::number(marks->Pos(endmark) - marks->Pos(last)) + "s &";
    }
    if (parts.length() > 0) {
        parts << soxpath + "sox";
        for (int i = 1; i <= nr; i++)
            parts << "\"" + file->fileName() + "." + QString::number(i) + ".wav\"";
        parts << "-t wavpcm \"" + path + QString::number(marks->Pos(startmark)) + ".wav\" splice 0.1 gain -n";
        for (int i = 1; i <= nr; i++)
            parts << "& del \"" + file->fileName().replace('/', '\\') + "." + QString::number(i) + ".wav\"";

        emit Debug("cmd /c " + parts.join(" "));
        proc.start("cmd /c " + parts.join(" "));
    }
}
