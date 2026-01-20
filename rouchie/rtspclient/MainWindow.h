#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include <string>
#include <vector>

#include "RtspView.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private:
    void setupUI();
    void setupMenu();
    void setupModel();
    void setupTimer();

private slots:
    void onOpenUrl();
    void onFrames(const std::vector<std::string> v);

private:
    RQTableModel *_pModel = nullptr;
    RtspView *_pRtspView = nullptr;

    bool _autoScrollBottom = true;

    int _playerID = -1;
};