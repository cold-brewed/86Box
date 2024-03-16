/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		Header for 86Box VM manager system details module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#ifndef QT_VMMANAGER_DETAILS_H
#define QT_VMMANAGER_DETAILS_H

#include <QWidget>
#include <QTimer>
#include "qt_vmmanager_system.hpp"
#include "qt_vmmanager_details_section.hpp"


QT_BEGIN_NAMESPACE
//namespace Ui { class VMManagerDetails; class CollapseButton;}
namespace Ui { class VMManagerDetails;}
QT_END_NAMESPACE

class VMManagerDetails : public QWidget {
    Q_OBJECT

public:
    explicit VMManagerDetails(QWidget *parent = nullptr);

    ~VMManagerDetails() override;

    void updateData(VMManagerSystem *passed_sysconfig);

    void updateProcessStatus();

    void updateWindowStatus();

    void processScreenshotAck();
//    CollapseButton *systemCollapseButton;

private:
    Ui::VMManagerDetails *ui;
    VMManagerSystem *sysconfig;

    VMManagerDetailsSection *systemSection;
    VMManagerDetailsSection *videoSection;
    VMManagerDetailsSection *storageSection;
    VMManagerDetailsSection *audioSection;

    QVBoxLayout *detailsLayout;

    QTimer *detail_updates;
    void updateScreenshot();
    bool hasRunningProcess();

private slots:
    void timerUpdates();

//    CollapseButton *systemCollapseButton;
//    QFrame *systemFrame;
//    CollapseButton *displayCollapseButton;
//    QFrame *displayFrame;
//    CollapseButton *storageCollapseButton;
//    QFrame *storageFrame;
};



#endif //QT_VMMANAGER_DETAILS_H
