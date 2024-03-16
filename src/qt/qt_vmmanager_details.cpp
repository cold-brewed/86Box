/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		86Box VM manager system details module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#include <QDebug>

#include "qt_vmmanager_details.hpp"
#include "ui_qt_vmmanager_details.h"

VMManagerDetails::VMManagerDetails(QWidget *parent) :
    QWidget(parent), ui(new Ui::VMManagerDetails) {
    ui->setupUi(this);
    // FIXME: I am creating a default constructor here but that is a terrible idea.
    // I either need to remove this or make a better default in VMManagerSystem.
    // Here be potential bugs if this widget doesn't get a proper VMManagerSystem!

//    detailsLayout = new QVBoxLayout();
//    ui->detailsFrame
//    ui->detailsFrame->setLayout(detailsLayout);
    detail_updates = new QTimer;


    systemSection = new VMManagerDetailsSection(tr("System", "Header for System section in VM Manager Details"));
//    ui->gridLayout->addLayout(systemSection->mainLayout, 0, 0);
    ui->detailsLayout->addLayout(systemSection->mainLayout, 0);

    videoSection = new VMManagerDetailsSection(tr("Display", "Header for Display section in VM Manager Details"));
//    ui->gridLayout->addLayout(videoSection->mainLayout, 1, 0);
    ui->detailsLayout->addLayout(videoSection->mainLayout, 0);

    storageSection = new VMManagerDetailsSection(tr("Storage", "Header for Storage section in VM Manager Details"));
//    ui->gridLayout->addLayout(storageSection->mainLayout, 2, 0);
    ui->detailsLayout->addLayout(storageSection->mainLayout, 0);
//    ui->pushButton->isChecked();

    audioSection = new VMManagerDetailsSection(tr("Audio", "Header for Audio section in VM Manager Details"));
    ui->detailsLayout->addLayout(audioSection->mainLayout, 0);

    sysconfig = new VMManagerSystem();
}

VMManagerDetails::~VMManagerDetails() {
    delete ui;
}

void
VMManagerDetails::updateData(VMManagerSystem *passed_sysconfig) {

    sysconfig = passed_sysconfig;

    // Populate each section for VM details

    // System
    systemSection->clear();
    systemSection->addSection("Machine", passed_sysconfig->getDisplayValue(Display::Name::Machine));
    systemSection->addSection("CPU", passed_sysconfig->getDisplayValue(Display::Name::CPU));
    systemSection->addSection("Memory", passed_sysconfig->getDisplayValue(Display::Name::Memory));

    // Video
    videoSection->clear();
    videoSection->addSection("Video", passed_sysconfig->getDisplayValue(Display::Name::Video));
    if(!passed_sysconfig->getDisplayValue(Display::Name::Voodoo).isEmpty()) {
        videoSection->addSection("Video", passed_sysconfig->getDisplayValue(Display::Name::Voodoo));
    }

    // Disks
    storageSection->clear();
    storageSection->addSection("Disks", passed_sysconfig->getDisplayValue(Display::Name::Disks));

    // Audio
    audioSection->clear();
    audioSection->addSection("Audio", passed_sysconfig->getDisplayValue(Display::Name::Audio));

    updateScreenshot();

    // General timer to process updates in this view
    disconnect(detail_updates, &QTimer::timeout, this, &VMManagerDetails::timerUpdates);
    connect(detail_updates, &QTimer::timeout, this, &VMManagerDetails::timerUpdates);
    detail_updates->setTimerType(Qt::CoarseTimer);
    detail_updates->start(2000);


    // Signals for process status changes
    ui->systemLabel->setText(passed_sysconfig->config_name);
    ui->moreLabel->setText(sysconfig->process->processId() == 0 ? "Not running" : QString::asprintf("Running: PID %lli", sysconfig->process->processId()));
    disconnect(sysconfig->process, &QProcess::stateChanged, this, &VMManagerDetails::updateProcessStatus);
    connect(sysconfig->process, &QProcess::stateChanged, this, &VMManagerDetails::updateProcessStatus);

    // Signals for when the client acknowledges a screenshot request
    disconnect(sysconfig, &VMManagerSystem::processScreenshotAck, this, &VMManagerDetails::processScreenshotAck);
    connect(sysconfig, &VMManagerSystem::processScreenshotAck, this, &VMManagerDetails::processScreenshotAck);

    // Signals for windows status updates received from the client
    disconnect(sysconfig, &VMManagerSystem::windowStatusChanged, this, &VMManagerDetails::updateWindowStatus);
    connect(sysconfig, &VMManagerSystem::windowStatusChanged, this, &VMManagerDetails::updateWindowStatus);
    updateProcessStatus();
}

void
VMManagerDetails::updateProcessStatus() {
    bool running = sysconfig->process->state() == QProcess::ProcessState::Running;
    QString status_text = running ? QString::asprintf("Running: PID %lli", sysconfig->process->processId()) : "Not running";
    status_text.append(sysconfig->window_obscured ? " (waiting)" : "");
    ui->moreLabel->setText(status_text);
}

bool
VMManagerDetails::hasRunningProcess()
{
    // FIXME: Why do I check elsewhere for `sysconfig->process->processId() == 0` instead?
    return sysconfig->process->state() == QProcess::ProcessState::Running;
}

void
VMManagerDetails::updateWindowStatus() {
    qInfo("Window status changed: %i", sysconfig->window_obscured);
    updateProcessStatus();

}
void
VMManagerDetails::timerUpdates()
{
    if(sysconfig == nullptr) {
        return;
    }
    updateScreenshot();
    if(hasRunningProcess()) {
        sysconfig->sendClientScreenshotRequest();
    }
}
void
VMManagerDetails::updateScreenshot()
{

    auto screenshots = sysconfig->getScreenshots();
    if (!screenshots.empty()) {
        ui->screenshot->setFrameStyle(QFrame::NoFrame);
        ui->screenshot->setEnabled(true);
        if(QFileInfo::exists(screenshots.last().filePath())) {
            QPixmap pic(screenshots.last().filePath());
            ui->screenshot->setPixmap(pic.scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    } else {
        ui->screenshot->setPixmap(QString());
        ui->screenshot->setFixedSize(320, 240);
        ui->screenshot->setFrameStyle(QFrame::Box | QFrame::Sunken);
        ui->screenshot->setText("No screenshot");
        ui->screenshot->setEnabled(false);
        ui->screenshot->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }
}
void
VMManagerDetails::processScreenshotAck()
{
//    qDebug() << "Processing screenshot ack";
    QTimer::singleShot(1000, this, &VMManagerDetails::updateScreenshot);
}
