// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QtWidgets/QMainWindow>
#include "framelesshelper.hpp"


class FramelessWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit FramelessWindow(QWidget *parent = nullptr);
    ~FramelessWindow() override;


protected:
    bool event(QEvent *event) override;

private:

    FramelessHelper* m_helper;
};

#endif // FRAMELESSWINDOW_H
