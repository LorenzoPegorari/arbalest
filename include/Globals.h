#ifndef GLOBALS_H
#define GLOBALS_H

#include "ADisplay.h"

class MainWindow;
class QSSPreprocessor;

class Globals{
public:
    static QSSPreprocessor *theme;
    static MainWindow *mainWindow;
};


#endif