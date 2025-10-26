#ifndef FRAMELESSHELPER_H
#define FRAMELESSHELPER_H

#include <QWidget>
#include <QTimer>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QWKWidgets/widgetwindowagent.h>

#include <widgetframe/windowbar.h>
#include <widgetframe/windowbutton.h>
#include <QActionGroup>
#include <QStyle>
#include <QFile>

namespace QWK {
    class WidgetWindowAgent;
    class StyleAgent;
}

static inline void emulateLeaveEvent(QWidget *widget) {
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const QScreen *screen = widget->screen();
#else
        const QScreen *screen = widget->windowHandle()->screen();
#endif
        const QPoint globalPos = QCursor::pos(screen);
        if (!QRect(widget->mapToGlobal(QPoint{0, 0}), widget->size()).contains(globalPos)) {
            QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
            if (widget->testAttribute(Qt::WA_Hover)) {
                const QPoint localPos = widget->mapFromGlobal(globalPos);
                const QPoint scenePos = widget->window()->mapFromGlobal(globalPos);
                static constexpr const auto oldPos = QPoint{};
                const Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                const auto event =
                    new QHoverEvent(QEvent::HoverLeave, scenePos, globalPos, oldPos, modifiers);
                Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
                QCoreApplication::postEvent(widget, event);
            }
        }
    });
}

enum Theme {
    Dark,
    Light,
};

class FramelessHelper : public QObject {
    Q_OBJECT;
public:
    FramelessHelper(QWidget* parent, Theme theme):QObject(parent),m_target(parent),m_currentTheme(theme)
    {
        parent->setAttribute(Qt::WA_DontCreateNativeAncestors);

        installWindowAgent();

        loadStyleSheet(theme);
    }

    void setMenuBar(QMenuBar* menuBar)
    {
        menuBar->setObjectName(QStringLiteral("win-menu-bar"));
        m_windowBar->setMenuBar(menuBar);
        m_windowAgent->setHitTestVisible(menuBar, true);

    }

    void installWindowAgent() {
        // 1. Setup window agent
        m_windowAgent = new QWK::WidgetWindowAgent(m_target);
        m_windowAgent->setup(m_target);


        // menuBar->setObjectName(QStringLiteral("win-menu-bar"));

        auto titleLabel = new QLabel();
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setObjectName(QStringLiteral("win-title-label"));

#ifndef Q_OS_MAC
        m_windowBar = new QWK::WindowBar();

        auto iconButton = new QWK::WindowButton();
        iconButton->setObjectName(QStringLiteral("icon-button"));
        iconButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_windowBar->setIconButton(iconButton);
        m_windowAgent->setSystemButton(QWK::WindowAgentBase::WindowIcon, iconButton);

        auto flags = m_target->windowFlags();
        if (flags.testFlag(Qt::WindowMinimizeButtonHint))
        {
            auto minButton = new QWK::WindowButton();
            minButton->setObjectName(QStringLiteral("min-button"));
            minButton->setProperty("system-button", true);
            minButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_windowBar->setMinButton(minButton);
            m_windowAgent->setSystemButton(QWK::WindowAgentBase::Minimize, minButton);
        }

        if (flags.testFlag(Qt::WindowMaximizeButtonHint))
        {
            auto maxButton = new QWK::WindowButton();
            maxButton->setCheckable(true);
            maxButton->setObjectName(QStringLiteral("max-button"));
            maxButton->setProperty("system-button", true);
            maxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_windowBar->setMaxButton(maxButton);
            m_windowAgent->setSystemButton(QWK::WindowAgentBase::Maximize, maxButton);
        }

        if (flags.testFlag(Qt::WindowCloseButtonHint))
        {
            auto closeButton = new QWK::WindowButton();
            closeButton->setObjectName(QStringLiteral("close-button"));
            closeButton->setProperty("system-button", true);
            closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_windowBar->setCloseButton(closeButton);
            m_windowAgent->setSystemButton(QWK::WindowAgentBase::Close, closeButton);
        }
#endif
        // windowBar->setMenuBar(menuBar);
        m_windowBar->setTitleLabel(titleLabel);
        m_windowBar->setHostWidget(m_target);

        m_windowAgent->setTitleBar(m_windowBar);

        // windowAgent->setHitTestVisible(menuBar, true);

#ifdef Q_OS_MAC
        windowAgent->setSystemButtonAreaCallback([](const QSize &size) {
            static constexpr const int width = 75;
            return QRect(QPoint(size.width() - width, 0), QSize(width, size.height())); //
        });
#endif

        // setMenuWidget(windowBar);

#ifndef Q_OS_MAC
        QObject::connect(m_windowBar, &QWK::WindowBar::minimizeRequested, m_target, &QWidget::showMinimized);
        QObject::connect(m_windowBar, &QWK::WindowBar::maximizeRequested, m_target, [this](bool max) {
            if (max)
            {
                m_target->showMaximized();
            }
            else
            {
                m_target->showNormal();
            }

                   // It's a Qt issue that if a QAbstractButton::clicked triggers a window's maximization,
                   // the button remains to be hovered until the mouse move. As a result, we need to
                   // manually send leave events to the button.

            QTimer::singleShot(0, [btn = m_windowBar->maxButton()] { emulateLeaveEvent(btn); });
        });
        QObject::connect(m_windowBar, &QWK::WindowBar::closeRequested, m_target, &QWidget::close);
#endif
    }

    void loadStyleSheet(Theme theme) {
        if (!m_target->styleSheet().isEmpty() && theme == m_currentTheme)
            return;
        m_currentTheme = theme;

        if (QFile qss(theme == Dark ? QStringLiteral(":/qss/dark-style.qss")
                                    : QStringLiteral(":/qss/light-style.qss"));
            qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_target->setStyleSheet(QString::fromUtf8(qss.readAll()));
            Q_EMIT themeChanged(m_currentTheme);
        }
    }

    QWidget *titleBar() const
    {
        return m_windowBar;
    }

    void setWindowAttribute(const QString &key, const QVariant &attribute)
    {
        m_windowAgent->setWindowAttribute(key, attribute);
    }

    Theme getTheme()
    {
        return m_currentTheme;
    }

Q_SIGNALS:
    void themeChanged(Theme theme);

private:

    QWidget *m_target{nullptr};
    Theme m_currentTheme{};
    QWK::WidgetWindowAgent *m_windowAgent;
    QWK::WindowBar* m_windowBar;
};

#endif // FRAMELESSHELPER_H
