#include "MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <vector>

#include "Module/Player.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUI();
    setupMenu();
    setupModel();
    // setupTimer();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    this->setMinimumSize(1024, 720);
}

void MainWindow::setupMenu() {
    /////////// 文件
    QMenu *fileMenu = menuBar()->addMenu(tr("文件"));

    QAction *openAction = fileMenu->addAction(tr("打开"));
    openAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    // 连接打开动作到槽函数
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenUrl);

    QAction *saveAction = fileMenu->addAction(tr("保存"));
    saveAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));

    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("退出"));

    /////////// 编辑
    QMenu *editMenu = menuBar()->addMenu("编辑");

    QAction *copyAction = editMenu->addAction("复制");
    QAction *pasteAction = editMenu->addAction("粘贴");
    QAction *deleteAction = editMenu->addAction("删除");

    /////////// 帮助
    QMenu *helpMenu = menuBar()->addMenu(tr("帮助"));

    QAction *aboutAction = helpMenu->addAction(tr("关于"));
    aboutAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogHelpButton));

    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupModel() {
    // 创建数据模型
    _pModel = new RQTableModel({ tr("DTS"), tr("PTS"), tr("I帧"), tr("信息帧"), tr("可抛弃"), tr("可解码"), tr("码流类型") }, this);
    _pRtspView = new RtspView;

    // 设置模型到视图
    _pRtspView->setModel(_pModel); // 直接使用model
    _pRtspView->verticalHeader()->hide();
    _pRtspView->horizontalHeader()->setStretchLastSection(true);

    this->setCentralWidget(_pRtspView);
}

void MainWindow::setupTimer() {
    // 创建定时器对象
    QTimer *timer = new QTimer(this); // this 表示父对象，会自动管理内存

    // 连接定时器的 timeout 信号到槽函数
    connect(timer, &QTimer::timeout, this, [this]() {
        qDebug() << "定时器触发，当前时间：" << QTime::currentTime().toString("hh:mm:ss.zzz");

        QList<QString> list;
        for (int col = 0; col < 6; ++col) {
            list.append(QString("%1").arg(col));
        }
        _pModel->AppendRow(list);

        // if (_autoScrollBottom) _pRtspView->scrollToBottom();
    });

    // 启动定时器，1000ms = 1秒
    timer->start(1000);
}

void MainWindow::onOpenUrl() {
    bool ok;
    QString url = QInputDialog::getText(this, tr("输入URL"), tr("请输入 URL:"), QLineEdit::Normal, "", &ok);

    if (ok && !url.isEmpty()) {
        // 用户点击了确定并且输入了URL
        qDebug() << "输入的URL:" << url;

        auto func = [](void *arg, const std::vector<std::string> &v) {
            MainWindow *self = static_cast<MainWindow *>(arg);
            QMetaObject::invokeMethod(self, [=]() {
                self->onFrames(v);
            }, Qt::QueuedConnection);
        };
        _playerID = AddPlayer("rtsp://admin:chen5477@192.168.2.111/stream1", func, (void *)this);
    } else if (!ok) {
        // 用户点击了取消
        qDebug() << "用户取消输入";
    }
}

void MainWindow::onFrames(const std::vector<std::string> v)
{
    qDebug() << "帧信息..." << v[0].c_str() << ":" << v[1].c_str();
    _pModel->AppendRow({ v[0].c_str(), v[1].c_str(), v[2].c_str(), v[3].c_str(), v[4].c_str(), v[5].c_str(), v[6].c_str() });
}