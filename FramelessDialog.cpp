#include "FramelessHelper.hpp"
#include "FramelessDialog.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>

FramelessDialog::FramelessDialog(QWidget *parent, Theme theme)
    : FramelessDialog(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, parent, theme)
{
}

FramelessDialog::FramelessDialog(QDialogButtonBox::StandardButtons buttons, QWidget *parent, Theme theme)
    : QDialog(parent, Qt::Dialog)
    , helper_(new FramelessHelper(this, theme))
    , content_(new QVBoxLayout)
    , central_(new QWidget(this))
{
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    auto box = new QDialogButtonBox(buttons, this);
    content_->setContentsMargins(9, 9, 9, 9);
    content_->addWidget(central_, 1);
    content_->addWidget(box);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(helper_->titleBar(), 0);
    layout->addLayout(content_, 1);

    connect(box, &QDialogButtonBox::accepted, this, &FramelessDialog::onAccepted);
    connect(box, &QDialogButtonBox::rejected, this, &FramelessDialog::onRejected);
    connect(helper_, &FramelessHelper::themeChanged, this, &FramelessDialog::themeChanged);
}

FramelessDialog::~FramelessDialog()
{
}

void FramelessDialog::setCentralWidget(QWidget *widget)
{
    content_->replaceWidget(central_, widget);
    central_->deleteLater();
    central_ = widget;
    central_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QWidget *FramelessDialog::centralWidget() const
{
    return central_;
}

void FramelessDialog::onAccepted()
{
    accept();
}

void FramelessDialog::onRejected()
{
    reject();
}

bool FramelessDialog::event(QEvent *event) {
    switch (event->type()) {
        case QEvent::WindowActivate: {
            auto menu = helper_->titleBar();
            if (menu) {
                menu->setProperty("bar-active", true);
                style()->polish(menu);
            }
            break;
        }

        case QEvent::WindowDeactivate: {
            auto menu = helper_->titleBar();
            if (menu) {
                menu->setProperty("bar-active", false);
                style()->polish(menu);
            }
            break;
        }

        default:
            break;
    }
    return QDialog::event(event);
}
