#include <QApplication>

#include "zmainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("ZMessenger");

    ZMainWindow *zm = new ZMainWindow;
    for (int i = 1;i < argc;i++) {
        if (strcmp(argv[i], "--record") == 0)
            zm->setRecord(true);
        else if (strcmp(argv[i], "--test") == 0)
            zm->setTesting(true);
        else
            fprintf(stderr, "zmessenger: Unknown option '%s'.\n", argv[i]);
    }

    if (zm->start())
        return app.exec();
    else
        return EXIT_FAILURE; 
}
