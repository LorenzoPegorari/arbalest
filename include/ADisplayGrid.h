
#ifndef RT3_DISPLAYGRID_H
#define RT3_DISPLAYGRID_H


#include <include/QVBoxWidget.h>
#include <include/Document.h>
#include <include/ADisplay.h>
#include <QSplitter>

class ADisplay;
class MouseAction;


class ADisplayGrid : public QVBoxWidget {
Q_OBJECT
public:
    explicit ADisplayGrid(Document*  document);

    void forceRerenderAllADisplays();

    Document *getDocument()  {
        return document;
    }

    QVector<ADisplay *> &getADisplays() {
        return displays;
    }

    ADisplay *getActiveADisplay(){
        return activeADisplay;
    }

    bool inQuadADisplayMode();

    int getActiveADisplayId();

    void setActiveADisplay(ADisplay *display);

    void singleADisplayMode(int displayId);
    void quadADisplayMode();

    void resetViewPort(int displayId);
    void resetAllViewPorts();

    void setMoveCameraMouseAction();
    void setSelectObjectMouseAction();

private:
    double defaultADisplayCameraRotation[4][3] = {
            {0, 0, 270},
            {270, 0, 180},
            {270, 0, 270},
            {295, 0, 235}
    };
    Document*  document;
    QVector<ADisplay *> displays;
    QVector<MouseAction *> mouseActions;
    ADisplay *activeADisplay;
    QSplitter *verticalSplitter;
    QSplitter *horizontalSplitter1;
    QSplitter *horizontalSplitter2;

};


#endif //RT3_DISPLAYGRID_H
