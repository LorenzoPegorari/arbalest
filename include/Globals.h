#ifndef GLOBALS_H
#define GLOBALS_H

#include "Viewport.h"

class MainWindow;
class QSSPreprocessor;

class Globals{
public:
    static QSSPreprocessor *theme;
    static MainWindow *mainWindow;
};


#endif