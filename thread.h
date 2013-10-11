#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QString>

class HorrThread : public QThread
{
    Q_OBJECT

public:
    HorrThread();
    void HorrThread::initHorr(const QString& fileName);

protected:
    void run();

private:
    bool processHorr(int &result);

signals:
    void passHorrValue(const QString& resultString);

private:
    QString m_fileName;
    volatile bool horrEnd;
     int horrResult;
     bool m_stop;
};

#endif
