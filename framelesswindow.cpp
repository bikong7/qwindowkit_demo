// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#include "framelesswindow.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>
#include <QtWidgets/QPushButton>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QtGui/QActionGroup>
#else
#  include <QtWidgets/QActionGroup>
#endif

// #include <QtWebEngineWidgets/QWebEngineView>

#include "FramelessDialog.h"

class ClockWidget : public QLabel {
public:
    explicit ClockWidget(QWidget *parent = nullptr) : QLabel(parent) {
        startTimer(100);
        setAlignment(Qt::AlignCenter);
    }

    ~ClockWidget() override = default;

protected:
    void timerEvent(QTimerEvent *event) override {
        QLabel::timerEvent(event);
        setText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    }
};

FramelessWindow::FramelessWindow(QWidget *parent) : QMainWindow(parent) {

    m_helper = new FramelessHelper(this, Theme::Dark);

    // 2. Construct your title bar
    auto menuBar = [this]() {
        auto menuBar = new QMenuBar(this);

        // Virtual menu
        auto file = new QMenu(tr("File(&F)"), menuBar);
        file->addAction(new QAction(tr("New(&N)"), menuBar));
        file->addAction(new QAction(tr("Open(&O)"), menuBar));
        file->addSeparator();

        auto edit = new QMenu(tr("Edit(&E)"), menuBar);
        edit->addAction(new QAction(tr("Undo(&U)"), menuBar));
        edit->addAction(new QAction(tr("Redo(&R)"), menuBar));

        auto dlgAction = new QAction(tr("custom dialog"), menuBar);
        connect(dlgAction, &QAction::triggered, this, [this]() {
            auto label = new QLabel("Hello world");
            label->setObjectName("test");
            label->setAlignment(Qt::AlignCenter);
            label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            FramelessDialog dialog(this, m_helper->getTheme());
            dialog.setWindowTitle("Help");
            dialog.setFixedSize(400, 240);
            dialog.setCentralWidget(label);
            dialog.exec();
        });

        // Theme action
        auto darkAction = new QAction(tr("Enable dark theme"), menuBar);
        darkAction->setCheckable(true);
        darkAction->setChecked(true);
        connect(darkAction, &QAction::triggered, this, [this](bool checked) {
            m_helper->loadStyleSheet(checked ? Dark : Light); //
        });
        connect(m_helper, &FramelessHelper::themeChanged, darkAction, [this, darkAction](Theme theme) {
            darkAction->setChecked(theme == Dark); //
        });

#ifdef Q_OS_WIN
        auto noneAction = new QAction(tr("None"), menuBar);
        noneAction->setData(QStringLiteral("none"));
        noneAction->setCheckable(true);
        noneAction->setChecked(true);

        auto dwmBlurAction = new QAction(tr("Enable DWM blur"), menuBar);
        dwmBlurAction->setData(QStringLiteral("dwm-blur"));
        dwmBlurAction->setCheckable(true);

        auto acrylicAction = new QAction(tr("Enable acrylic material"), menuBar);
        acrylicAction->setData(QStringLiteral("acrylic-material"));
        acrylicAction->setCheckable(true);

        auto micaAction = new QAction(tr("Enable mica"), menuBar);
        micaAction->setData(QStringLiteral("mica"));
        micaAction->setCheckable(true);

        auto micaAltAction = new QAction(tr("Enable mica alt"), menuBar);
        micaAltAction->setData(QStringLiteral("mica-alt"));
        micaAltAction->setCheckable(true);

        auto winStyleGroup = new QActionGroup(menuBar);
        winStyleGroup->addAction(noneAction);
        winStyleGroup->addAction(dwmBlurAction);
        winStyleGroup->addAction(acrylicAction);
        winStyleGroup->addAction(micaAction);
        winStyleGroup->addAction(micaAltAction);
        connect(winStyleGroup, &QActionGroup::triggered, this,
                [this, winStyleGroup](QAction *action) {
                    // Unset all custom style attributes first, otherwise the style will not display
                    // correctly
                    for (const QAction *_act : winStyleGroup->actions()) {
                        const QString data = _act->data().toString();
                        if (data.isEmpty() || data == QStringLiteral("none")) {
                            continue;
                        }
                        m_helper->setWindowAttribute(data, false);
                    }
                    const QString data = action->data().toString();
                    if (data == QStringLiteral("none")) {
                        setProperty("custom-style", false);
                    } else if (!data.isEmpty()) {
                        m_helper->setWindowAttribute(data, true);
                        setProperty("custom-style", true);
                    }
                    style()->polish(this);
                });

#elif defined(Q_OS_MAC)
        // Set whether to use system buttons (close/minimize/zoom)
        // - true:  Hide system buttons (use custom UI controls)
        // - false: Show native system buttons (default behavior)
        windowAgent->setWindowAttribute(QStringLiteral("no-system-buttons"), false);

        auto darkBlurAction = new QAction(tr("Dark blur"), menuBar);
        darkBlurAction->setCheckable(true);
        connect(darkBlurAction, &QAction::toggled, target_, [target_](bool checked) {
            if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"), "dark")) {
                return;
            }
            if (checked) {
                setProperty("custom-style", true);
                style()->polish(target_);
            }
        });

        auto lightBlurAction = new QAction(tr("Light blur"), menuBar);
        lightBlurAction->setCheckable(true);
        connect(lightBlurAction, &QAction::toggled, target_, [target_](bool checked) {
            if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"), "light")) {
                return;
            }
            if (checked) {
                setProperty("custom-style", true);
                style()->polish(target_);
            }
        });

        auto noBlurAction = new QAction(tr("No blur"), menuBar);
        noBlurAction->setCheckable(true);
        connect(noBlurAction, &QAction::toggled, target_, [target_](bool checked) {
            if (!windowAgent->setWindowAttribute(QStringLiteral("blur-effect"), "none")) {
                return;
            }
            if (checked) {
                setProperty("custom-style", false);
                style()->polish(target_);
            }
        });

        auto macStyleGroup = new QActionGroup(menuBar);
        macStyleGroup->addAction(darkBlurAction);
        macStyleGroup->addAction(lightBlurAction);
        macStyleGroup->addAction(noBlurAction);
#endif

               // Real menu
        auto settings = new QMenu(tr("Settings(&S)"), menuBar);
        settings->addAction(darkAction);
        settings->addAction(dlgAction);


#ifdef Q_OS_WIN
        settings->addSeparator();
        settings->addAction(noneAction);
        settings->addAction(dwmBlurAction);
        settings->addAction(acrylicAction);
        settings->addAction(micaAction);
        settings->addAction(micaAltAction);
#elif defined(Q_OS_MAC)
        settings->addAction(darkBlurAction);
        settings->addAction(lightBlurAction);
        settings->addAction(noBlurAction);
#endif

        menuBar->addMenu(file);
        menuBar->addMenu(edit);
        menuBar->addMenu(settings);
        return menuBar;
    }();

    m_helper->setMenuBar(menuBar);
    setMenuWidget(m_helper->titleBar());

#if 1
    auto clockWidget = new ClockWidget();
    clockWidget->setObjectName(QStringLiteral("clock-widget"));
    clockWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCentralWidget(clockWidget);
#else
    auto webView = new QWebEngineView();
    webView->load(QUrl("https://www.baidu.com"));
    setCentralWidget(webView);
#endif



    setWindowTitle(tr("Example MainWindow"));
    resize(800, 600);

    // setFixedHeight(600);
    // windowAgent->centralize();
}

FramelessWindow::~FramelessWindow() = default;

bool FramelessWindow::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::WindowActivate: {
            auto menu = menuWidget();
            if (menu) {
                menu->setProperty("bar-active", true);
                style()->polish(menu);
            }
            break;
        }

        case QEvent::WindowDeactivate: {
            auto menu = menuWidget();
            if (menu) {
                menu->setProperty("bar-active", false);
                style()->polish(menu);
            }
            break;
        }

        default:
            break;
    }
    return QMainWindow::event(event);
}

