#include "mp3gnormalizer.h"
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
#include <QProcess>
#include <QFileInfo>

#include "ziphandler.h"

Mp3gNormalizer::Mp3gNormalizer(QObject *parent) : QObject(parent)
{
    force = false;
    zipLvl = 9;
}

void Mp3gNormalizer::setFilename(QString fname)
{
    this->fname = fname;
}

void Mp3gNormalizer::setForce(bool force)
{
    this->force = force;
}

void Mp3gNormalizer::run()
{
    qWarning() << "Processing file: " << fname;
    qWarning() << "Force: " << force;
    if (!QFile::exists(fname))
    {
        qWarning() << "Specified file does not exist!";
        emit finished();
        return;
    }
    processZipFileRg(fname);
    emit finished();
}

void Mp3gNormalizer::aboutToQuitApp()
{

}

void Mp3gNormalizer::processZipFileRg(QString fileName)
{
    QString mp3gainPath = "mp3gain";
    qWarning() << "Processing - File: " << fileName;
    QFileInfo info(fileName);
    QString baseName = info.completeBaseName();
    QTemporaryDir tmpDir;
    ZipHandler zipper;
    zipper.setZipFile(fileName);
    if ((zipper.containsRGMarkerFile()) && (!force))
    {
        qWarning() << "File contains marker, already processed.  Skipping";
        zipper.close();
        return;
    }
    qWarning() << "Processing - Extracting mp3 file";
    if (!zipper.extractAudio(QDir(tmpDir.path())))
    {
        qWarning() << "Bad file contents - error extracting mp3";
        zipper.close();
        return;
    }
    QFile::copy(tmpDir.path() + QDir::separator() + "tmp.mp3", "/storage/KaraokeRGTest/PreRG.mp3");
    qWarning() << "Processing - Extracting cdg file";
    if (!zipper.extractCdg(QDir(tmpDir.path())))
    {
        qWarning() << "Processing - Bad file contents - error extracting cdg";
        zipper.close();
        return;
    }
    zipper.close();
    QString program = mp3gainPath;
    QStringList arguments;
    arguments << "-c";
    arguments << "-r";
    arguments << "-T";
    arguments << "-s" << "r";
    arguments << tmpDir.path() + QDir::separator() + "tmp.mp3";
    QProcess process;
    qWarning() << "Doing mp3gain processing...";
    process.start(program, arguments);
    process.waitForFinished();
    int exitCode = process.exitCode();
    QFile::copy(tmpDir.path() + QDir::separator() + "tmp.mp3", "/storage/KaraokeRGTest/PostRG.mp3");
    //qWarning() << process.readAllStandardOutput();
    //qWarning() << process.readAllStandardError();
    if (exitCode != 0)
    {
        qWarning() << "Error occurred while running mp3gain, aborting";
        qWarning() << "mp3gain error output: " << process.readAllStandardError();
        return;
    }
    qWarning() << "Processing - Creating zip file";
    QFile::rename(tmpDir.path() + QDir::separator() + "tmp.mp3", tmpDir.path() + QDir::separator() + baseName + ".mp3");
    QFile::rename(tmpDir.path() + QDir::separator() + "tmp.cdg", tmpDir.path() + QDir::separator() + baseName + ".cdg");
    zipper.setReplaceFnsWithZipFn(false);
    zipper.setCompressionLevel(zipLvl);
    if (zipper.createZip(tmpDir.path() + QDir::separator() + info.fileName(), tmpDir.path() + QDir::separator() + baseName + ".cdg", tmpDir.path() + QDir::separator() + baseName + ".mp3", true))
    {
        qWarning () << "Processing - Replacing original file";
        if (QFile(tmpDir.path() + QDir::separator() + info.fileName()).exists())
        {
            //qWarning() << "Doing QFile::rename(" << info.absoluteFilePath() << ", " << info.absoluteFilePath() + ".tmp" << ");";
            if (QFile::rename(info.absoluteFilePath(), info.absoluteFilePath() + ".tmp"))
            {
                if (QFile::copy(tmpDir.path() + QDir::separator() + info.fileName(), info.absoluteFilePath()))
                {
                    qWarning() << "New file copied into place, deleting old one";
                    if (!QFile::remove(info.absoluteFilePath() + ".tmp"))
                    {
                        qWarning() << "Error deleting old file";
                    }
                }
                else
                {
                    qWarning() << "Unable to move new file into place, restoring old one";
                    QFile::rename(info.absoluteFilePath() + ".tmp", info.absoluteFilePath());
                }

            }
            else
            {
                qWarning() << "Unable to move existing file to tmp file";
                return;
            }
        }
        else
        {
            qWarning() << "Unexpected error, new processed zip file missing!";
        }
        qWarning() << "Processing - Complete for file: " << fileName;
    }
    else
    {
        qWarning() << "Failed to create new archive, leaving original file in place";
    }
}
