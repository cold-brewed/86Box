/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		Header for 86Box VM manager main module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#ifndef QT_VMMANAGER_MAIN_H
#define QT_VMMANAGER_MAIN_H

#include <QWidget>
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include "qt_vmmanager_model.hpp"
#include "qt_vmmanager_details.hpp"
#include "qt_vmmanager_listviewdelegate.hpp"
#include "qt_vmmanager_config.hpp"

extern "C" {
#include <86box/86box.h>
}


QT_BEGIN_NAMESPACE
namespace Ui { class VMManagerMain; }
QT_END_NAMESPACE

class VMManagerMain : public QDialog {
    Q_OBJECT

public:
    explicit VMManagerMain(QDialog *parent = nullptr);

    ~VMManagerMain() override;

private:
    Ui::VMManagerMain *ui;
    void updateSelection(const QItemSelection &selected,
                                       const QItemSelection &deselected);
    void changeCurrent(const QModelIndex &current,
                                     const QModelIndex &previous);

    VMManagerModel   *vm_model;
    VMManagerDetails *vm_details;
    VMManagerSystem  *selected_sysconfig;
    VMManagerConfig *config;
    QSortFilterProxyModel *proxy_model;
    void startButtonPressed();
    void settingsButtonPressed();
    void restartButtonPressed();
    void pauseButtonPressed();
    void shutdownButtonPressed();
    void refresh();
    void addButtonPressed();
};

#endif //QT_VMMANAGER_MAIN_H
