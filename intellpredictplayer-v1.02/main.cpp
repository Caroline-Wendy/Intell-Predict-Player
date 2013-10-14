#include <QtGui>
#include "mediaplayer.h"
#include "NoPornVideo.h"

const qreal DefaultVolume = -1.0;

int main (int argc, char *argv[])
{

    Q_INIT_RESOURCE(mediaplayer);
    QApplication app(argc, argv);

    QFont serifFont("Times", 10);
    app.setFont(serifFont);
    app.setWindowIcon(QIcon(":/images/icon.ico"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

    app.setApplicationName("Intelligent Prediction Player");
    app.setOrganizationName("C.L.Wang");
    app.setQuitOnLastWindowClosed(true);

    QString fileName;
    qreal volume = DefaultVolume;
    bool smallScreen = false;

    QStringList args(app.arguments());
    args.removeFirst(); // remove name of executable
    while (!args.empty()) {
        const QString &arg = args.first();
        if (QLatin1String("-small-screen") == arg || QLatin1String("--small-screen") == arg) {
            smallScreen = true;
        } else if (QLatin1String("-volume") == arg || QLatin1String("--volume") == arg) {
            if (!args.empty()) {
                args.removeFirst();
                volume = qMax(qMin(args.first().toFloat(), float(1.0)), float(0.0));
            }
        } else if (fileName.isNull()) {
            fileName = arg;
        }
        args.removeFirst();
    }

    MediaPlayer player;
    player.setSmallScreen(smallScreen);
    if (DefaultVolume != volume)
        player.setVolume(volume);
    if (!fileName.isNull())
        player.setFile(fileName);

    if (smallScreen)
        player.showMaximized();
    else
        player.show();

    return app.exec();
}

