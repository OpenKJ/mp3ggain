#ifndef MP3GNORMALIZER_H
#define MP3GNORMALIZER_H

#include <QObject>

class Mp3gNormalizer : public QObject
{
    Q_OBJECT
private:
    QString fname;
    bool force;
    void processZipFileRg(QString fileName);
public:
    explicit Mp3gNormalizer(QObject *parent = nullptr);
    void setFilename(QString fname);
    void setForce(bool force);
    int zipLvl;

signals:
    void finished();

public slots:
    void run();
    void aboutToQuitApp();
};

#endif // MP3GNORMALIZER_H
