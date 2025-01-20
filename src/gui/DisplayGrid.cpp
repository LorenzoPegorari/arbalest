
#include <QtOpenGL/QtOpenGL>
#include <include/ADisplay.h>
#include "ADisplayGrid.h"
#include "MoveCameraMouseAction.h"
#include "SelectMouseAction.h"

ADisplayGrid::ADisplayGrid(Document*  document) : document(document) {
    verticalSplitter = new QSplitter(this);
    verticalSplitter->setOrientation(Qt::Vertical);
    horizontalSplitter1 = new QSplitter(this);
    horizontalSplitter2 = new QSplitter(this);

    verticalSplitter->addWidget(horizontalSplitter1);
    verticalSplitter->addWidget(horizontalSplitter2);

    displays.append(new ADisplay(document));
    displays.append(new ADisplay(document));
    displays.append(new ADisplay(document));
    displays.append(new ADisplay(document));

    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);
    mouseActions.append(nullptr);

    horizontalSplitter1->addWidget(displays[0]);
    horizontalSplitter1->addWidget(displays[1]);
    horizontalSplitter2->addWidget(displays[2]);
    horizontalSplitter2->addWidget(displays[3]);

    for(int i=0;i<4;i++){
        displays[i]->getCamera()->setAnglesAroundAxes(defaultADisplayCameraRotation[i][0],
                                                      defaultADisplayCameraRotation[i][1],
                                                      defaultADisplayCameraRotation[i][2]);
    }

    addWidget(verticalSplitter);
    activeADisplay = displays[3];

    singleADisplayMode(3);
}

void ADisplayGrid::forceRerenderAllADisplays() {
    for (ADisplay *display : displays){
        display->forceRerenderFrame();
    }
}

void ADisplayGrid::setActiveADisplay(ADisplay *display) {
    activeADisplay = display;
}

void ADisplayGrid::resetViewPort(int displayId) {
    displays[displayId]->getCamera()->setAnglesAroundAxes(defaultADisplayCameraRotation[displayId][0],
                                                          defaultADisplayCameraRotation[displayId][1],
                                                          defaultADisplayCameraRotation[displayId][2]);

    displays[displayId]->getCamera()->autoview();
    displays[displayId]->forceRerenderFrame();
}

void ADisplayGrid::resetAllViewPorts() {
    for(int i=0;i<4;i++)resetViewPort(i);
}

void ADisplayGrid::singleADisplayMode(int displayId) {
    for(int i=0;i<4;i++){
        if (i != displayId)displays[i]->hide();
    }
    displays[displayId]->show();
    activeADisplay = displays[displayId];
    verticalSplitter->setHandleWidth(0);

    forceRerenderAllADisplays();
}
void ADisplayGrid::quadADisplayMode() {
    for(int i=0;i<4;i++){
        displays[i]->show();
    }

    verticalSplitter->setHandleWidth(5);
    verticalSplitter->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter1->setSizes(QList<int>({INT_MAX, INT_MAX}));
    horizontalSplitter2->setSizes(QList<int>({INT_MAX, INT_MAX}));

    forceRerenderAllADisplays();
}

int ADisplayGrid::getActiveADisplayId() {
    for(int i=0;i<4;i++)if(displays[i]==activeADisplay)return i;
    return 3;
}

bool ADisplayGrid::inQuadADisplayMode() {
    return !displays[0]->isHidden() && !displays[1]->isHidden();
}

void ADisplayGrid::setMoveCameraMouseAction() {
    int displaysSize = displays.size();
    for (int index = 0; index < displaysSize; ++index) {
        if (mouseActions[index] != nullptr) {
            delete mouseActions[index];
        }

        mouseActions[index] = new MoveCameraMouseAction(this, displays[index]);
    }
}

void ADisplayGrid::setSelectObjectMouseAction() {
    int displaysSize = displays.size();
    for (int index = 0; index < displaysSize; ++index) {
        if (mouseActions[index] != nullptr) {
            delete mouseActions[index];
        }

        mouseActions[index] = new SelectMouseAction(this, displays[index]);
        connect(mouseActions[index], &MouseAction::Done, this, [this](MouseAction* mouseAction) {
            SelectMouseAction* selectMouseAction = dynamic_cast<SelectMouseAction*>(mouseAction);

            if (selectMouseAction != nullptr) {
                QString regionName = selectMouseAction->getSelected();
                document->getObjectTreeWidget()->select(regionName);
            }
        });
    }
}
