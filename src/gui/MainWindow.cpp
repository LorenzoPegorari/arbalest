#include <QtWidgets/QFileDialog>
#include "MainWindow.h"
#include "ArbDisplayGrid.h"
#include <Document.h>
#include <QtWidgets/QLabel>
#include <include/QSSPreprocessor.h>
#include <include/Properties.h>
#include <include/Globals.h>
#include <iostream>
#include <QtGui/QtGui>
#include <include/QHBoxWidget.h>
#include <QApplication>
#include <QMessageBox>
#include <QComboBox>
#include <include/RaytraceView.h>
#include <include/AboutWindow.h>
#include <include/HelpWidget.h>
#include <brlcad/Database/Arb8.h>
#include <QtWidgets/QtWidgets>
#include <brlcad/Database/Torus.h>
#include <brlcad/Database/Cone.h>
#include <brlcad/Database/Particle.h>
#include <brlcad/Database/Paraboloid.h>
#include <brlcad/Database/Hyperboloid.h>
#include <brlcad/Database/Halfspace.h>
#include <brlcad/Database/EllipticalTorus.h>
#include <brlcad/Database/Ellipsoid.h>
#include <brlcad/Database/HyperbolicCylinder.h>
#include <brlcad/Database/ParabolicCylinder.h>
#include <include/MatrixTransformWidget.h>
#include "MoveCameraMouseAction.h"
#include "SelectMouseAction.h"

using namespace BRLCAD;
using namespace std;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_mouseAction{nullptr}
{
    loadTheme();
    prepareUi();
    prepareDockables();

    documentArea->addTab(new HelpWidget(this), "Quick Start");
    if (QCoreApplication::arguments().length() > 1) openFile(QString(QCoreApplication::arguments().at(1)));
    Globals::mainWindow = this;
}

MainWindow::~MainWindow()
{
    for (const std::pair<const int, Document*>& pair : documents) {
        Document* document = pair.second;
        delete document;
    }
}

void MainWindow::loadTheme()
{
    QSettings settings("BRLCAD", "arbalest");  // Sets organizationName and applicationName

    // Sets theme
    int themeIndex = settings.value("themeIndex", 0).toInt();
    QStringList themes = {":themes/arbalest_light.theme",":themes/arbalest_dark.theme"};
    QFile themeFile(themes[themeIndex] );
    themeFile.open( QFile::ReadOnly );
    QString themeStr( themeFile.readAll() );
    Globals::theme = new QSSPreprocessor(themeStr);
    themeFile.close();

    // Sets style
    QFile styleFile( ":styles/arbalest_simple.qss" );
    styleFile.open( QFile::ReadOnly );
    QString styleStr(styleFile.readAll() );
    qApp->setStyleSheet(Globals::theme->process(styleStr));
    styleFile.close();
}

void MainWindow::prepareUi()
{
    setWindowTitle("Arbalest");
    setWindowFlags(Qt::FramelessWindowHint);  // Hide window title bar

    // ---------------------------------------- MENU BAR ----------------------------------------
    
    menuTitleBar = new QMenuBar(this);
    menuTitleBar->installEventFilter(this);
    setMenuBar(menuTitleBar);

    // "FILE" MENU
    QMenu *fileMenu = menuTitleBar->addMenu(tr("&File"));
    connect(fileMenu, &QMenu::aboutToShow, this, &MainWindow::increaseMenusOpen);
    connect(fileMenu, &QMenu::aboutToHide, this, &MainWindow::decreaseMenusOpen);

    // "New" action
    QIcon newActIcon;
    newActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-MenuIconFile")), QIcon::Normal);
    newActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-Menu")), QIcon::Active);
    QAction* newAct = new QAction(newActIcon, tr("New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("New .g file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);  // Send signal to call MainWindow::newFile
    fileMenu->addAction(newAct);

    // "Open" action
    QIcon openActIcon;
    openActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-MenuIconFile")), QIcon::Normal);
    openActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-Menu")), QIcon::Active);
    QAction* openAct = new QAction(openActIcon, tr("Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Opens a .g file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFileDialog);  // Send signal to call MainWindow::openFileDialog
    fileMenu->addAction(openAct);

    // "Save" action
    QIcon saveActIcon;
    saveActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-MenuIconFile")), QIcon::Normal);
    saveActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-Menu")), QIcon::Active);
    QAction* saveAct = new QAction(saveActIcon, tr("Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save database"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFileDefaultPath);  // Send signal to call MainWindow::saveFileDefaultPath
    fileMenu->addAction(saveAct);

    // "Save as" action
    QIcon saveAsActIcon;
    saveAsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-MenuIconFile")), QIcon::Normal);
    saveAsActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-Menu")), QIcon::Active);
    QAction* saveAsAct = new QAction(saveAsActIcon, tr("Save As..."), this);
    saveAsAct->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    saveAsAct->setStatusTip(tr("Save database as"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFileDialog);  // Send signal to call MainWindow::saveAsFileDialog
    fileMenu->addAction(saveAsAct);
    
    fileMenu->addSeparator();

    // "Quit" action
    QIcon quitActIcon;
    quitActIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/quitIcon.png", "$Color-MenuIconQuit")), QIcon::Normal);
    QAction* quitAct = new QAction(quitActIcon, tr("Quit"), this);
    quitAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
    quitAct->setStatusTip(tr("Quit"));
    connect(quitAct, &QAction::triggered, this, [this]() {
        QCoreApplication::quit();
    });
    fileMenu->addAction(quitAct);

    // "CREATE" MENU
    QMenu* createMenu = menuTitleBar->addMenu(tr("&Create"));
    connect(createMenu, &QMenu::aboutToShow, this, &MainWindow::increaseMenusOpen);
    connect(createMenu, &QMenu::aboutToHide, this, &MainWindow::decreaseMenusOpen);

    // "Arb8" action
    QAction* createArb8Act = new QAction(tr("Arb8"), this);
    connect(createArb8Act, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Arb8 * object = new BRLCAD::Arb8();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createArb8Act);

    // "Cone" action
    QAction* createConeAct = new QAction(tr("Cone"), this);
    connect(createConeAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Cone * object = new BRLCAD::Cone();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createConeAct);

    // "Ellipsoid" action
    QAction* createEllipsoidAct = new QAction(tr("Ellipsoid"), this);
    connect(createEllipsoidAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Ellipsoid * object = new BRLCAD::Ellipsoid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createEllipsoidAct);

    // "Elliptical Torus" action
    QAction* createEllipticalTorusAct = new QAction(tr("Elliptical Torus"), this);
    connect(createEllipticalTorusAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::EllipticalTorus * object = new BRLCAD::EllipticalTorus();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createEllipticalTorusAct);

    // "Halfspace" action
    QAction* createHalfspaceAct = new QAction(tr("Halfspace"), this);
    connect(createHalfspaceAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Halfspace * object = new BRLCAD::Halfspace();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createHalfspaceAct);

    // "Hyperbolic Cylinder" action
    QAction* createHyperbolicCylinderAct = new QAction(tr("Hyperbolic Cylinder"), this);
    connect(createHyperbolicCylinderAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::HyperbolicCylinder * object = new BRLCAD::HyperbolicCylinder();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createHyperbolicCylinderAct);

    // "Hyperboloid" action
    QAction* createHyperboloidAct = new QAction(tr("Hyperboloid"), this);
    connect(createHyperboloidAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Hyperboloid * object = new BRLCAD::Hyperboloid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createHyperboloidAct);

    // "Parabolic Cylinder" action
    QAction* createParabolicCylinderAct = new QAction(tr("Parabolic Cylinder"), this);
    connect(createParabolicCylinderAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::ParabolicCylinder * object = new BRLCAD::ParabolicCylinder();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createParabolicCylinderAct);

    // "Paraboloid" action
    QAction* createParaboloidAct = new QAction(tr("Paraboloid"), this);
    connect(createParaboloidAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Paraboloid * object = new BRLCAD::Paraboloid();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createParaboloidAct);

    // "Particle" action
    QAction* createParticleAct = new QAction(tr("Particle"), this);
    connect(createParticleAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Particle * object = new BRLCAD::Particle();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createParticleAct);

    // "Torus" action
    QAction* createTorusAct = new QAction(tr("Torus"), this);
    connect(createTorusAct, &QAction::triggered, this,[this]() {
        if (activeDocumentId == -1) return;

        QString name;
        if (!getObjectNameFromUser(this, *documents[activeDocumentId], name)) return;
        BRLCAD::Torus * object = new BRLCAD::Torus();
        object->SetName(name.toUtf8());
        documents[activeDocumentId]->Add(*object);
        int objectId = documents[activeDocumentId]->getObjectTree()->addTopObject(name);
        documents[activeDocumentId]->getObjectTree()->changeVisibilityState(objectId,true);
        documents[activeDocumentId]->getObjectTreeWidget()->build(objectId);
        documents[activeDocumentId]->getObjectTreeWidget()->refreshItemTextColors();
        documents[activeDocumentId]->getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        documents[activeDocumentId]->getArbDisplayGrid()->forceRerenderAllArbDisplays();
    });
    createMenu->addAction(createTorusAct);

    // "EDIT" MENU
    QMenu* editMenu = menuTitleBar->addMenu(tr("&Edit"));
    connect(editMenu, &QMenu::aboutToShow, this, &MainWindow::increaseMenusOpen);
    connect(editMenu, &QMenu::aboutToHide, this, &MainWindow::decreaseMenusOpen);

    // "Relative move selected object" action
    QAction* relativeMoveAct = new QAction("Relative move selected object", this);
    relativeMoveAct->setStatusTip(tr("Relative move selected object. Top objects cannot be moved."));
    connect(relativeMoveAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Translate);
    });
    editMenu->addAction(relativeMoveAct);

    // "Relative scale selected object" action
    QAction* relativeScaleAct = new QAction("Relative scale selected object", this);
    relativeScaleAct->setStatusTip(tr("Relative scale selected object. Top objects cannot be scaled."));
    connect(relativeScaleAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Scale);
    });
    editMenu->addAction(relativeScaleAct);

    // "Relative rotate selected object" action
    QAction* relativeRotateAct = new QAction("Relative rotate selected object", this);
    relativeRotateAct->setStatusTip(tr("Relative rotate selected object. Top objects cannot be rotated."));
    connect(relativeRotateAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        MatrixTransformWidget * matrixTransformWidget = new MatrixTransformWidget(documents[activeDocumentId],objectId, MatrixTransformWidget::Rotate);
    });
    editMenu->addAction(relativeRotateAct);

    // "Select object" action
    selectObjectAct = new QAction(tr("Select object"), this);
    selectObjectAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/select_object.png", "$Color-MenuIconEdit")));
    selectObjectAct->setStatusTip(tr("Select object."));
    selectObjectAct->setCheckable(true);
    connect(selectObjectAct, &QAction::toggled, this, &MainWindow::updateMouseButtonObjectState);
    editMenu->addAction(selectObjectAct);

    // "VIEW" MENU
    QMenu* viewMenu = menuTitleBar->addMenu(tr("&View"));
    connect(viewMenu, &QMenu::aboutToShow, this, &MainWindow::increaseMenusOpen);
    connect(viewMenu, &QMenu::aboutToHide, this, &MainWindow::decreaseMenusOpen);

    // "Reset current viewport" action
    QAction* resetViewportAct = new QAction("Reset current viewport", this);
    resetViewportAct->setStatusTip(tr("Reset to default camera orientation for the viewport and autoview to currently visible objects"));
    connect(resetViewportAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getArbDisplayGrid()->resetViewPort(documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplayId());
    });
    viewMenu->addAction(resetViewportAct);

    // "Reset all viewports" action
    QAction* resetAllViewportsAct = new QAction("Reset all viewports", this);
    resetAllViewportsAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_refresh_black_48dp.png", "$Color-MenuIconView")));
    resetAllViewportsAct->setStatusTip(tr("Reset to default camera orientation for each viewport and autoview to visible objects"));
    connect(resetAllViewportsAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getArbDisplayGrid()->resetAllViewPorts();
    });
    viewMenu->addAction(resetAllViewportsAct);

    viewMenu->addSeparator();

    // "Focus visible objects (all viewports)" action
    QAction* autoViewAct = new QAction(tr("Focus visible objects (all viewports)"), this);
    autoViewAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_free_black_48dp.png", "$Color-MenuIconView")));
    autoViewAct->setShortcut(Qt::Key_F|Qt::CTRL);
    autoViewAct->setStatusTip(tr("Resize and center the view based on the current visible objects"));
    connect(autoViewAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        for (ArbDisplay * display : documents[activeDocumentId]->getArbDisplayGrid()->getArbDisplays()) {
            display->getCamera()->autoview();
            display->forceRerenderFrame();
        }
    });
    viewMenu->addAction(autoViewAct);

    // "Focus visible objects (current viewport)" action
    QAction* autoViewSingleAct = new QAction(tr("Focus visible objects (current viewport)"), this);
    autoViewSingleAct->setStatusTip(tr("Resize and center the view based on the current visible objects"));
    connect(autoViewSingleAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getArbDisplay()->getCamera()->autoview();
        documents[activeDocumentId]->getArbDisplay()->forceRerenderFrame();
    });
    viewMenu->addAction(autoViewSingleAct);

    // "Focus selected object" action
    QAction* centerViewAct = new QAction(tr("Focus selected object"), this);
    centerViewAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_center_focus_strong_black_48dp.png", "$Color-MenuIconView")));
    centerViewAct->setStatusTip(tr("Resize and center the view based on the selected objects"));
    centerViewAct->setShortcut(Qt::Key_F);
    connect(centerViewAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        if (documents[activeDocumentId]->getObjectTreeWidget()->currentItem() == nullptr) return;
        int objectId = documents[activeDocumentId]->getObjectTreeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        documents[activeDocumentId]->getArbDisplay()->getCamera()->centerView(objectId);
    });
    viewMenu->addAction(centerViewAct);
    
    viewMenu->addSeparator();

    // "Single View" submenu
    currentViewport = new QComboBox();
    QMenu* singleView = viewMenu->addMenu(tr("&Single View"));
    QActionGroup *viewportMenuGroup = new QActionGroup(this);
    for(int i = 0; i < 4; i++) {
        singleViewAct[i] = new QAction("Viewport " + QString::number(i+1), this);
        viewportMenuGroup->addAction(singleViewAct[i]);
        singleViewAct[i]->setCheckable(true);
        singleViewAct[i]->setStatusTip("ArbDisplay viewport " + QString::number(i+1));
        connect(singleViewAct[i], &QAction::triggered, this, [this,i]() {
            if (activeDocumentId == -1) return;
            documents[activeDocumentId]->getArbDisplayGrid()->singleArbDisplayMode(i);
            currentViewport->setCurrentIndex(i);
        });
        singleView->addAction(singleViewAct[i]);
    }
    singleViewAct[0]->setShortcut(Qt::Key_1);
    singleViewAct[1]->setShortcut(Qt::Key_2);
    singleViewAct[2]->setShortcut(Qt::Key_3);
    singleViewAct[3]->setShortcut(Qt::Key_4);
    singleViewAct[3]->setChecked(true);

    // "All Viewports" action
    QAction* quadViewAct = new QAction(tr("All Viewports"), this);
    quadViewAct->setStatusTip(tr("ArbDisplay 4 viewports"));
    connect(quadViewAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        documents[activeDocumentId]->getArbDisplayGrid()->quadArbDisplayMode();
        for(QAction *i : singleViewAct) i->setChecked(false);
        currentViewport->setCurrentIndex(4);
    });
    quadViewAct->setShortcut(Qt::Key_5);
    viewMenu->addAction(quadViewAct);

    viewMenu->addSeparator();
    
    // "Toggle grid on/off" action
    QAction* toggleGridAct = new QAction(tr("Toggle grid on/off"), this);
    toggleGridAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sharp_grid_on_black_48dp.png", "$Color-MenuIconView")));
    toggleGridAct->setCheckable(true);
    toggleGridAct->setShortcut(Qt::Key_G);
    connect(toggleGridAct, &QAction::toggled, this, [=]() {
        if (activeDocumentId == -1) {
            toggleGridAct->setChecked(false);
            return;
        }

        if (documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->gridEnabled == false) {
            documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->gridEnabled = true;
            toggleGridAct->setToolTip("Toggle grid OFF (G)");
        } else {
            documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->gridEnabled = false;
            toggleGridAct->setToolTip("Toggle grid ON (G)");
        }

        documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->forceRerenderFrame();
    });
    viewMenu->addAction(toggleGridAct);

    // "Select theme" submenu
    QMenu* selectThemeAct = viewMenu->addMenu(tr("Select theme"));
    QActionGroup *selectThemeActGroup = new QActionGroup(this);

    themeAct[0] = new QAction(tr("Arbalest Light"), this);
    themeAct[0]->setCheckable(true);
    connect(themeAct[0], &QAction::triggered, this, [this]() {
        QSettings settings("BRLCAD", "arbalest");
        settings.setValue("themeIndex", 0);
        QMessageBox::information(this, "Application Restart Needed", "Selected theme will be set after next restart of Arbalest", QMessageBox::Ok);
    });
    selectThemeActGroup->addAction(themeAct[0]);
    selectThemeAct->addAction(themeAct[0]);

    themeAct[1] = new QAction(tr("Arbalest Dark"), this);
    themeAct[1]->setCheckable(true);
    connect(themeAct[1], &QAction::triggered, this, [this]() {
        QSettings settings("BRLCAD", "arbalest");
        settings.setValue("themeIndex", 1);
        QMessageBox::information(this, "Application Restart Needed", "Selected theme will be set after next restart of Arbalest", QMessageBox::Ok);
    });
    selectThemeActGroup->addAction(themeAct[1]);
    selectThemeAct->addAction(themeAct[1]);
    QSettings settings("BRLCAD", "arbalest");
    themeAct[settings.value("themeIndex",0).toInt()]->setChecked(true);

    // "RAYTRACE" MENU
    QMenu* raytrace = menuTitleBar->addMenu(tr("&Raytrace"));
    connect(raytrace, &QMenu::aboutToShow, this, &MainWindow::increaseMenusOpen);
    connect(raytrace, &QMenu::aboutToHide, this, &MainWindow::decreaseMenusOpen);

    // "Raytrace current viewport" action
    QAction* raytraceAct = new QAction(tr("Raytrace current viewport"), this);
    raytraceAct->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_filter_vintage_black_48dp.png", "$Color-MenuIconRaytrace")));
    raytraceAct->setStatusTip(tr("Raytrace current viewport"));
    raytraceAct->setShortcut(Qt::CTRL|Qt::Key_R);
    connect(raytraceAct, &QAction::triggered, this, [this]() {
        if (activeDocumentId == -1) return;
        statusBar->showMessage("Raytracing current viewport...", statusBarShortMessageDuration);
        QCoreApplication::processEvents();
        documents[activeDocumentId]->getRaytraceWidget()->raytrace();
        statusBar->showMessage("Raytracing completed.", statusBarShortMessageDuration);
    });
    raytrace->addAction(raytraceAct);

    // "Set raytrace background color..." action
    QAction* setRaytraceBackgroundColorAct = new QAction(tr("Set raytrace background color..."), this);
    connect(setRaytraceBackgroundColorAct, &QAction::triggered, this, [this]() {
        QSettings settings("BRLCAD", "arbalest");
        QColor color=settings.value("raytraceBackground").value<QColor>();
        QColor selectedColor = QColorDialog::getColor(color);
        settings.setValue("raytraceBackground", selectedColor);
    });
    raytrace->addAction(setRaytraceBackgroundColorAct);

    // "HELP" MENU
    QMenu* help = menuTitleBar->addMenu(tr("&Help"));
    connect(help, &QMenu::aboutToShow, this, &MainWindow::increaseMenusOpen);
    connect(help, &QMenu::aboutToHide, this, &MainWindow::decreaseMenusOpen);

    // "About" action
    QAction* aboutAct = new QAction(tr("About"), this);
    connect(aboutAct, &QAction::triggered, this, [this]() {
        (new AboutWindow())->show();
    });
    help->addAction(aboutAct);

    // "Help" action
    QAction* helpAct = new QAction(tr("Help"), this);
    helpAct->setShortcut(Qt::Key_F1);
    connect(helpAct, &QAction::triggered, this, [this]() {
        HelpWidget * helpWidget = dynamic_cast<HelpWidget*>(documentArea->widget(0));
        if (helpWidget== nullptr) {
            documentArea->insertTab(0,new HelpWidget(this), "Quick Start");
        }
        documentArea->setCurrentIndex(0);
    });
    help->addAction(helpAct);

    // ARBALEST ICON
    applicationIcon = new QPushButton(menuTitleBar);
    applicationIcon->setIcon(QIcon(":/icons/arbalest_icon.png"));
    applicationIcon->setObjectName("topLeftAppIcon");
    menuTitleBar->setCornerWidget(applicationIcon, Qt::TopLeftCorner);
    setWindowIcon(*new QIcon(*new QBitmap(":/icons/arbalest_icon.png")));
    applicationIcon->installEventFilter(this);

    // MINIMIZE/MAXIMIZE/CLOSE BUTTONS
    QHBoxLayout* layoutTopRightWidget = new QHBoxLayout;
    layoutTopRightWidget->setContentsMargins(0, 0, 0, 0);
    QWidget* topRightWidget = new QWidget;
    topRightWidget->setLayout(layoutTopRightWidget);
    menuTitleBar->setCornerWidget(topRightWidget, Qt::TopRightCorner);
    layoutTopRightWidget->setSpacing(0);

    // Minimize button
    minimizeButton = new QPushButton(topRightWidget);
    minimizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_minimize_white_36dp","$Color-WindowTitleButtons")));
    minimizeButton->setObjectName("minimizeButton");
    connect(minimizeButton, &QPushButton::clicked, this, &MainWindow::minimizeButtonPressed);
    layoutTopRightWidget->addWidget(minimizeButton);
    minimizeButton->installEventFilter(this);

    // Maximize button
    maximizeButton = new QPushButton(topRightWidget);
    if (this->windowState() == Qt::WindowMaximized) {
        maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sadeep_edited_baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
    }
    else {
        maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
    }
    maximizeButton->setObjectName("maximizeButton");
    connect(maximizeButton, &QPushButton::clicked, this, &MainWindow::maximizeButtonPressed);
    layoutTopRightWidget->addWidget(maximizeButton);
    maximizeButton->installEventFilter(this);

    // Close button
    closeButton = new QPushButton(topRightWidget);
    closeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sadeep_edited_baseline_close_white_36dp","$Color-WindowTitleButtons")));
    closeButton->setObjectName("closeButton");
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::closeButtonPressed);
    layoutTopRightWidget->addWidget(closeButton);
    closeButton->installEventFilter(this);


    // ---------------------------------------- STATUS BAR ----------------------------------------

    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBarPathLabel = new QLabel("No document open");
    statusBarPathLabel->setObjectName("statusBarPathLabel");
    statusBar->addWidget(statusBarPathLabel);
    

    // ---------------------------------------- DOCUMENT AREA ----------------------------------------

    documentArea = new QTabWidget(this);
    documentArea->setObjectName("documentArea");
    documentArea->setMovable(true);
    documentArea->setTabsClosable(true);
    setCentralWidget(documentArea);
    documentArea->tabBar()->setObjectName("documentAreaTabBar");
    connect(documentArea, &QTabWidget::currentChanged, this, &MainWindow::onActiveDocumentChanged);
    connect(documentArea, &QTabWidget::tabCloseRequested, this, &MainWindow::tabCloseRequested);
    connect(documentArea, &QTabWidget::currentChanged, this, &MainWindow::updateMouseButtonObjectState);
    
    // TOOLBAR

    QHBoxWidget * mainTabBarCornerWidget = new QHBoxWidget();
    mainTabBarCornerWidget->setObjectName("mainTabBarCornerWidget");

    // "New" button
    QToolButton* newButton = new QToolButton(menuTitleBar);
    newButton->setDefaultAction(newAct);
    newButton->setObjectName("toolbarButton");
    QIcon newButtonIcon;
    newButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_note_add_black_48dp.png", "$Color-IconFile")), QIcon::Normal);
    newButton->setIcon(newButtonIcon);
    newButton->setToolTip("New (Ctrl+N)");
    mainTabBarCornerWidget->addWidget(newButton);

    // "Open" button
    QToolButton* openButton = new QToolButton(menuTitleBar);
    openButton->setDefaultAction(openAct);
    openButton->setObjectName("toolbarButton");
    QIcon openButtonIcon;
    openButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_folder_black_48dp.png", "$Color-IconFile")), QIcon::Normal);
    openButton->setIcon(openButtonIcon);
    openButton->setToolTip("Open (Ctrl+O)");
    mainTabBarCornerWidget->addWidget(openButton);

    // "Save" button
    QToolButton* saveButton = new QToolButton(menuTitleBar);
    saveButton->setDefaultAction(saveAct);
    saveButton->setObjectName("toolbarButton");
    QIcon saveButtonIcon;
    saveButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_save_black_48dp.png", "$Color-IconFile")), QIcon::Normal);
    saveButton->setIcon(saveButtonIcon);
    saveButton->setToolTip("Save (Ctrl+S)");
    mainTabBarCornerWidget->addWidget(saveButton);

    // "Save as" button
    QToolButton* saveAsButton = new QToolButton(menuTitleBar);
    saveAsButton->setDefaultAction(saveAsAct);
    saveAsButton->setObjectName("toolbarButton");
    QIcon saveAsButtonIcon;
    saveAsButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/saveAsIcon.png", "$Color-IconFile")), QIcon::Normal);
    saveAsButton->setIcon(saveAsButtonIcon);
    saveAsButton->setToolTip("Save as..");
    mainTabBarCornerWidget->addWidget(saveAsButton);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    // "Focus all" button
    QToolButton* focusAll = new QToolButton(menuTitleBar);
    focusAll->setDefaultAction(autoViewAct);
    focusAll->setObjectName("toolbarButton");
    QIcon focusAllIcon;
    focusAllIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_free_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    focusAll->setIcon(focusAllIcon);
    focusAll->setToolTip("Focus on all visible objects (Ctrl+F)");
    mainTabBarCornerWidget->addWidget(focusAll);

    // "Focus selected" button
    QToolButton* focusCurrent = new QToolButton(menuTitleBar);
    focusCurrent->setDefaultAction(centerViewAct);
    focusCurrent->setObjectName("toolbarButton");
    QIcon focusCurrentIcon;
    focusCurrentIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_center_focus_strong_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    focusCurrent->setIcon(focusCurrentIcon);
    focusCurrent->setToolTip("Focus on selected object (F)");
    mainTabBarCornerWidget->addWidget(focusCurrent);

    // "Reset viewports" button
    QToolButton* resetViewports = new QToolButton(menuTitleBar);
    resetViewports->setDefaultAction(resetAllViewportsAct);
    resetViewports->setObjectName("toolbarButton");
     QIcon resetViewportsIcon;
    resetViewportsIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_refresh_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    resetViewports->setIcon(resetViewportsIcon);
    resetViewports->setToolTip("Reset the viewports and focus on the visible");
    mainTabBarCornerWidget->addWidget(resetViewports);
    
    currentViewport->setToolTip("Change viewport");
    currentViewport->addItem("Viewport 1");
    currentViewport->addItem("Viewport 2");
    currentViewport->addItem("Viewport 3");
    currentViewport->addItem("Viewport 4");
    currentViewport->addItem("All Viewports");
    currentViewport->setCurrentIndex(3);
    connect(currentViewport, QOverload<int>::of(&QComboBox::activated),[=](int index) {
        if (activeDocumentId == -1) return;
        if (index <4) documents[activeDocumentId]->getArbDisplayGrid()->singleArbDisplayMode(index);
        else documents[activeDocumentId]->getArbDisplayGrid()->quadArbDisplayMode();
        for(QAction *i : singleViewAct) i->setChecked(false);
        if(index != 4) singleViewAct[index]->setChecked(true);
    });
    mainTabBarCornerWidget->addWidget(currentViewport);

    // "Toggle grid" button
    QToolButton* toggleGrid = new QToolButton(menuTitleBar);
    toggleGrid->setDefaultAction(toggleGridAct);
    toggleGrid->setObjectName("toolbarButton");
    QIcon toggleGridIcon;
    toggleGridIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/sharp_grid_on_black_48dp.png", "$Color-IconView")), QIcon::Normal);
    toggleGrid->setIcon(toggleGridIcon);
    mainTabBarCornerWidget->addWidget(toggleGrid);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    // "Select object" button
    QToolButton* selectObjectButton = new QToolButton(menuTitleBar);
    selectObjectButton->setDefaultAction(selectObjectAct);
    selectObjectButton->setObjectName("toolbarButton");
    QIcon selectObjectButtonIcon;
    selectObjectButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/select_object.png", "$Color-IconSelectObject")), QIcon::Normal);
    selectObjectButton->setIcon(selectObjectButtonIcon);
    selectObjectButton->setToolTip("Select object");
    mainTabBarCornerWidget->addWidget(selectObjectButton);

    mainTabBarCornerWidget->addWidget(toolbarSeparator(false));

    // "Raytrace" button
    QToolButton* raytraceButton = new QToolButton(menuTitleBar);
    raytraceButton->setDefaultAction(raytraceAct);
    raytraceButton->setObjectName("toolbarButton");
    QIcon raytraceButtonIcon;
    raytraceButtonIcon.addPixmap(QPixmap::fromImage(coloredIcon(":/icons/baseline_filter_vintage_black_48dp.png", "$Color-IconRaytrace")), QIcon::Normal);
    raytraceButton->setIcon(raytraceButtonIcon);
    raytraceButton->setToolTip("Raytrace current viewport (Ctrl+R)");
    mainTabBarCornerWidget->addWidget(raytraceButton);

    documentArea->setCornerWidget(mainTabBarCornerWidget,Qt::Corner::TopRightCorner);
}

void MainWindow::prepareDockables()
{
    // OBJECTS SIDE MENU
    objectTreeWidgetDockable = new Dockable("Objects", this, false, 200);
    addDockWidget(Qt::LeftDockWidgetArea, objectTreeWidgetDockable);

    // PROPERTIES SIDE MENU
    objectPropertiesDockable = new Dockable("Properties", this, true, 300);
    addDockWidget(Qt::RightDockWidgetArea, objectPropertiesDockable);

    // Toolbox
//    toolbokxDockable = new Dockable("Make", this,true,30);
//    toolboxDockable->hideHeader();
//    addDockWidget(Qt::LeftDockWidgetArea, toolboxDockable);
}

void MainWindow::newFile() {
    Document* document = new Document(documentsCount);
    document->getObjectTreeWidget()->setObjectName("dockableContent");
    document->getProperties()->setObjectName("dockableContent");
    documents[documentsCount++] = document;
    QString filename("Untitled");
    const int tabIndex = documentArea->addTab(document->getArbDisplayGrid(), filename);
    documentArea->setCurrentIndex(tabIndex);
    connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
            this, &MainWindow::objectTreeWidgetSelectionChanged);
}

void MainWindow::openFile(const QString& filePath)
{
    Document* document = nullptr;

    try {
        document = new Document(documentsCount, &filePath);
    } catch (...) {
        QString msg = "Failed to open " + filePath;
        statusBar->showMessage(msg, statusBarShortMessageDuration);

        QMessageBox msgBox;
        msgBox.setText(msg);
        msgBox.exec();
    }

    if (document != nullptr) {
        document->getObjectTreeWidget()->setObjectName("dockableContent");
        document->getProperties()->setObjectName("dockableContent");
        documents[documentsCount++] = document;
        QString filename(QFileInfo(filePath).fileName());
        const int tabIndex = documentArea->addTab(document->getArbDisplayGrid(), filename);
        documentArea->setCurrentIndex(tabIndex);
        connect(documents[activeDocumentId]->getObjectTreeWidget(), &ObjectTreeWidget::selectionChanged,
                this, &MainWindow::objectTreeWidgetSelectionChanged);
    }
}

bool MainWindow::saveFile(const QString& filePath)
{
    if (!documents[activeDocumentId]->isModified()) return false;

    return documents[activeDocumentId]->Save(filePath.toUtf8().data());
}

bool MainWindow::saveFileId(const QString& filePath, int documentId)
{
    return documents[documentId]->Save(filePath.toUtf8().data());
}

void MainWindow::openFileDialog() 
{
    const QString filePath = QFileDialog::getOpenFileName(documentArea, tr("Open BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()) openFile(filePath);
}

void MainWindow::saveAsFileDialog()
{
    if (activeDocumentId == -1) return;
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()) {
        if (saveFile(filePath)) {
            documents[activeDocumentId]->setFilePath(filePath);
            QString filename(QFileInfo(filePath).fileName());
            documentArea->setTabText(documentArea->currentIndex(), filename);
            statusBarPathLabel->setText(*documents[activeDocumentId]->getFilePath());
            statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
        }
    }
}

bool MainWindow::saveAsFileDialogId(int documentId)
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save BRL-CAD database"), QString(), "BRL-CAD Database (*.g)");
    if (!filePath.isEmpty()) {
        if (saveFileId(filePath, documentId)) {
            documents[documentId]->setFilePath(filePath);
            QString filename(QFileInfo(filePath).fileName());
            documentArea->setTabText(documentArea->currentIndex(), filename);
            statusBarPathLabel->setText(*documents[documentId]->getFilePath());
            statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            return true;
        }
    }

    return false;
}

void MainWindow::saveFileDefaultPath()
{
    if (activeDocumentId == -1) return;
    if (documents[activeDocumentId]->getFilePath() == nullptr) saveAsFileDialog();
    else {
        const QString filePath = *documents[activeDocumentId]->getFilePath();
        if (!filePath.isEmpty()) {
            if (saveFile(filePath)) {
                statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            }
        }
    }
}

bool MainWindow::saveFileDefaultPathId(int documentId)
{
    if (documentId == -1) return false;
    if (documents[documentId]->getFilePath() == nullptr) return saveAsFileDialogId(documentId);
    
    const QString filePath = *documents[documentId]->getFilePath();
    if (!filePath.isEmpty()) {
        if (saveFileId(filePath, documentId)) {
            statusBar->showMessage("Saved to " + filePath, statusBarShortMessageDuration);
            return true;
        }
    }

    return false;
}

bool MainWindow::maybeSave(int documentId, bool *cancel)
{
    // Checks if the document has any unsaved changes
    if (documents[documentId]->isModified()) {
        QFileInfo pathName(documents[documentId]->getFilePath() != nullptr ? *documents[documentId]->getFilePath() : "Untitled");

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Do you want to save the changes you made to " + pathName.fileName() + "?");
        msgBox.setInformativeText("Your changes will be lost if you don't save them.");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        msgBox.setDefaultButton(QMessageBox::Save);

        if (cancel != nullptr) {
            msgBox.addButton(QMessageBox::Cancel);
            *cancel = false;
        }
            
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Save:
            if (saveFileDefaultPathId(documentId)) return true;
            return false;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            assert(cancel != nullptr);
            *cancel = true;
            return false;
        }
    }

    return true;
}

void MainWindow::onActiveDocumentChanged(const int newIndex)
{
    ArbDisplayGrid * displayGrid = dynamic_cast<ArbDisplayGrid*>(documentArea->widget(newIndex));
    if (displayGrid != nullptr) {
        if (displayGrid->getDocument()->getDocumentId() != activeDocumentId){
            activeDocumentId = displayGrid->getDocument()->getDocumentId();
            objectTreeWidgetDockable->setContent(documents[activeDocumentId]->getObjectTreeWidget());
            objectPropertiesDockable->setContent(documents[activeDocumentId]->getProperties());
            statusBarPathLabel->setText(documents[activeDocumentId]->getFilePath()  != nullptr ? *documents[activeDocumentId]->getFilePath() : "Untitled");

            if (documents[activeDocumentId]->getArbDisplayGrid()->inQuadArbDisplayMode()) {
                currentViewport->setCurrentIndex(4);
                for(QAction * action:singleViewAct) action->setChecked(false);
            } else {
                currentViewport->setCurrentIndex(documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplayId());
                for(QAction * action:singleViewAct) action->setChecked(false);
                singleViewAct[documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplayId()]->setChecked(true);
            }
        }
    } else if (activeDocumentId != -1) {
        objectTreeWidgetDockable->clear();
        objectPropertiesDockable->clear();
        statusBarPathLabel->setText("");
        activeDocumentId = -1;
    }
}

void MainWindow::tabCloseRequested(const int i)
{
    int documentId = -1;
    ArbDisplayGrid* displayGrid = dynamic_cast<ArbDisplayGrid*>(documentArea->widget(i));
    
    if (displayGrid != nullptr) {
        documentId = displayGrid->getDocument()->getDocumentId();

        if (!maybeSave(documentId)) return;
    }
    documentArea->removeTab(i);
    if (documentId != -1) {
        delete documents[documentId];
        documents.erase(documentId);
    }
    if (documentArea->currentIndex() == -1) {
        objectTreeWidgetDockable->clear();
        objectPropertiesDockable->clear();
        statusBarPathLabel->setText("");
        activeDocumentId = -1;
    }
}

void MainWindow::objectTreeWidgetSelectionChanged(int objectId)
{
    documents[activeDocumentId]->getProperties()->bindObject(objectId);
}

void MainWindow::closeButtonPressed()
{
    close();
}

void MainWindow::minimizeButtonPressed()
{
    showMinimized();
}

void MainWindow::maximizeButtonPressed()
{
    if(!isMaximized()) showMaximized();
    else showNormal();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static QPoint dragPosition{};
    static bool wasButtonPressed;

    // if wasButtonPressed is true, it means that the cursor tried to grab the MENU BAR from a button
    if ((watched == minimizeButton) || (watched == maximizeButton) || (watched == closeButton)) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton) {
                wasButtonPressed = true;
                return false;
            }
        }
    // if the cursor was pressed inside MENU BAR and if no menu is open -> save its current coordinates
    } else if (watched == menuTitleBar || watched == applicationIcon) {
        if (event->type() == QEvent::MouseButtonPress && nMenusOpen == 0) {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton) {
                wasButtonPressed = false;
                dragPosition = mouse_event->globalPosition().toPoint() - frameGeometry().topLeft();
                return false;
            }
        /* If the cursor is trying to drag the MENU BAR, if no menu is open and if no button was pressed:
         *     resize the MainWindow if it was maximized
         *     move the MainWindow accordingly to the previously saved coordinates */
        } else if (event->type() == QEvent::MouseMove && nMenusOpen == 0 && !wasButtonPressed) {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if ((mouse_event->buttons() & Qt::LeftButton)) {
                if(isMaximized()) {
                    showNormal();  // to show the ability to resize the window on Linux
                    setGeometry(
                        (mouse_event->globalPosition().toPoint().x() + frameGeometry().left())/ 2,
                        frameGeometry().top(),
                        size().width() / 2,
                        size().height() / 2
                    );
                    dragPosition = mouse_event->globalPosition().toPoint() - frameGeometry().topLeft();
                    return false;
                }
                move(mouse_event->globalPosition().toPoint() - dragPosition);
                return false;
            }
        }
    }

    return false;
}

void MainWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange) {
        if (this->windowState() == Qt::WindowMaximized) {
            maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/sadeep_edited_baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
        } else {
            maximizeButton->setIcon(QPixmap::fromImage(coloredIcon(":/icons/baseline_crop_din_white_36dp.png","$Color-WindowTitleButtons")));
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    int documentSize = documents.size();
    bool cancel = false;
    ArbDisplayGrid * displayGrid = nullptr;

    for (int documentIndex = 1; documentIndex <= documentSize; ++documentIndex) {
        displayGrid = dynamic_cast<ArbDisplayGrid*>(documentArea->widget(documentIndex));
        int documentId = displayGrid->getDocument()->getDocumentId();
        
        if (maybeSave(documentId, &cancel)) continue;
        else break;
    }

    if (cancel == true) event->ignore();
    else event->accept();
}

void MainWindow::moveCameraButtonAction()
{
    if (activeDocumentId != -1) {
        ArbDisplayGrid* displayGrid = documents[activeDocumentId]->getArbDisplayGrid();

        if (displayGrid != nullptr) displayGrid->setMoveCameraMouseAction();
    }
}

void MainWindow::selectObjectButtonAction()
{
    ArbDisplayGrid* displayGrid = documents[activeDocumentId]->getArbDisplayGrid();

    if (displayGrid != nullptr) displayGrid->setSelectObjectMouseAction();
}

void MainWindow::updateMouseButtonObjectState()
{
    if (activeDocumentId == -1) {
        selectObjectAct->setChecked(false);
        return;
    }

    if (selectObjectAct->isChecked() == true) {
        documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->moveCameraEnabled = false;
        selectObjectAct->setToolTip("Select Object OFF");
        selectObjectButtonAction();
    } else if (documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->moveCameraEnabled == false) {
        documents[activeDocumentId]->getArbDisplayGrid()->getActiveArbDisplay()->moveCameraEnabled = true;
        selectObjectAct->setToolTip("Select Object ON");
        moveCameraButtonAction();
    }
}

void MainWindow::increaseMenusOpen()
{
    nMenusOpen++;
}

void MainWindow::decreaseMenusOpen()
{
    nMenusOpen--;
}
