#include "savetracks.h"

#include <QtDebug>

SaveTracks::SaveTracks(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&proc, SIGNAL(readyRead()), this, SLOT(canread()));
    QObject::connect(&proc, SIGNAL(finished(int)), this, SLOT(finished()));
    proc.setProcessChannelMode(QProcess::MergedChannels);
}

void SaveTracks::canread()
{
    QString out = proc.readAll();
    qDebug() << out;
}

void SaveTracks::SaveTrack(int TrackNr)
{
    list.append(TrackNr);
}

void SaveTracks::finished()
{
    emit Finished(list.takeFirst());

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
        for (int i = list.first(); i < marks->Count() && ignore == 0; i++) {
            if (marks->Type(i) == Marks::StartTrack)
                ignore++;
            else if (marks->Type(i) == Marks::EndTrack)
                ignore--;
            last = i;
        }
        if (last > 0) {
            if (list.first() == last - 1) { // No Marks between Start and End of Track
                //
            }
            else {
                int silent = 0;
                int firstsilent = 0;
                bool havesilent = false;
                for (int i = list.first() + 1; i < last - 1; i++) {
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
                if (!havesilent && silent = 0) {
                    // Same as NoMarks
                }
                else if (!havesilent && silent > 0) { // silent starts, but not end
                    last = firstsilent; // set Track-end to silent start
                    // then same as nomarks
                }
                else if (havesilent) {
                    // multi places to one track
                }
            }
        }
        else {
            list.removeFirst();
            Start();
        }
    }
}
