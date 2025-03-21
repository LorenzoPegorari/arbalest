#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QtWidgets/QMdiArea>
#include <unordered_map>
#include "Document.h"
#include "Dockable.h"
#include "QSSPreprocessor.h"
#include <QStatusBar>
#include <QMenuBar>
#include <QComboBox>
#include "MouseAction.h"

class Document;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStatusBar *getStatusBar() const {
        return statusBar;
    }

    const int statusBarShortMessageDuration = 7000;

private:
    // UI components
    Dockable *objectTreeWidgetDockable;
    Dockable *objectPropertiesDockable;
    Dockable *toolboxDockable;
    QStatusBar *statusBar;
    QMenuBar* menuTitleBar;
    QPushButton *applicationIcon;
    QPushButton *minimizeButton;
    QPushButton *maximizeButton;
    QPushButton *closeButton;
    QTabWidget *documentArea;
    QLabel *statusBarPathLabel;
    QComboBox * currentViewport;
    QAction* singleViewAct[4];
    MouseAction *m_mouseAction;
    QAction* selectObjectAct;
    
    // Stores pointers to all the currently opened documents. Item removed when document is closed. Key is documents ID.
    std::unordered_map<int, Document*> documents;

    // Total number of documents ever opened in application's life time. This is not decreased when closing documents.
    // A document's ID is set to documentsCount at the moment of opening it.
    int documentsCount = 0;
    // The ID of the active document.
    int activeDocumentId = -1;

    // Number of menus open in the MENU BAR
    int nMenusOpen = 0;
    
    void loadTheme();
    // Prepares MENU BAR, STATUS BAR and DOCUMENT AREA
    void prepareUi();
    // Prepares OBJECTS SIDE MENU and PROPERTIES SIDE MENU
    void prepareDockables();

    bool saveFile(const QString& filePath);
    bool maybeSave(int documentId, bool *cancel = nullptr);

    QAction *themeAct[2];
    
protected:
    // Drag MAIN WINDOW by holding from MENU BAR (now functioning as title bar too)
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent* event) override;
    
    void moveCameraButtonAction();
    void selectObjectButtonAction();

public slots:
    // New empty file
    void newFile();
    // Open file
    void openFile(const QString& filePath);
    bool saveFileId(const QString& filePath, int documentId);
    void openFileDialog();
    void saveAsFileDialog();
    bool saveAsFileDialogId(int documentId);
    void saveFileDefaultPath();
    bool saveFileDefaultPathId(int documentId);
    void onActiveDocumentChanged(int newIndex);
    void tabCloseRequested(int i) ;
    void objectTreeWidgetSelectionChanged(int objectId);
    void closeButtonPressed();
    void minimizeButtonPressed();
    void maximizeButtonPressed();
    void updateMouseButtonObjectState();
    void increaseMenusOpen();
    void decreaseMenusOpen();
};

#endif  // MAINWINDOW_H
