#pragma once

#include "framelesshelper.hpp"
#include <QDialog>
#include <QDialogButtonBox>

class FramelessDialog : public QDialog
{
    Q_OBJECT;

public:
    explicit FramelessDialog(QWidget *parent, Theme theme = Theme::Dark);
    explicit FramelessDialog(QDialogButtonBox::StandardButtons, QWidget *parget, Theme theme = Theme::Dark);
    virtual ~FramelessDialog() override;

public:
    void setCentralWidget(QWidget *widget);
    QWidget *centralWidget() const;

protected:
    virtual void onAccepted();
    virtual void onRejected();
protected:
    bool event(QEvent *event) override;

Q_SIGNALS:
    void themeChanged(Theme theme);

private:
    class FramelessHelper *helper_{ nullptr };
    class QBoxLayout *content_;
    QWidget *central_{ nullptr };
};
