#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include "mp3gnormalizer.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("mp3ggain");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("ReplayGain processor for karaoke files (mp3+g zip files)");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("karaoke file", QCoreApplication::translate("main", "File to perform ReplayGain processing on"));

    QCommandLineOption forceOption(QStringList() << "f" << "force", QCoreApplication::translate("main", "Force processing even if marker file is present"));
    parser.addOption(forceOption);

    parser.process(a);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)
    QString path = args.at(0);
    bool force = parser.isSet(forceOption);


    Mp3gNormalizer mp3gNormalizer;
    mp3gNormalizer.setFilename(path);
    mp3gNormalizer.setForce(force);
    QObject::connect(&mp3gNormalizer, SIGNAL(finished()),
             &a, SLOT(quit()));
    QObject::connect(&a, SIGNAL(aboutToQuit()),
             &mp3gNormalizer, SLOT(aboutToQuitApp()));

    QTimer::singleShot(10, &mp3gNormalizer, SLOT(run()));
    return a.exec();
}
