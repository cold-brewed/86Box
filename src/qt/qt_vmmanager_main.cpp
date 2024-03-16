/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		86Box VM manager main module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#include <QAbstractListModel>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include "qt_vmmanager_main.hpp"
#include "ui_qt_vmmanager_main.h"
#include "qt_vmmanager_model.hpp"

VMManagerMain::VMManagerMain(QDialog *parent) :
    QDialog(parent), ui(new Ui::VMManagerMain) {
    ui->setupUi(this);
    this->setWindowTitle("86Box VM Manager");
    ui->listView->setItemDelegate(new VMManagerListViewDelegate);
    vm_model = new VMManagerModel;
    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSourceModel(vm_model);
    ui->listView->setModel(proxy_model);
    proxy_model->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->listView->model()->sort(0, Qt::AscendingOrder);

    // Button setup
    connect(ui->startStopButton, &QPushButton::clicked, this, &VMManagerMain::startButtonPressed);
    connect(ui->settingsButton, &QPushButton::clicked, this, &VMManagerMain::settingsButtonPressed);
    connect( ui->restartButton, &QPushButton::clicked, this, &VMManagerMain::restartButtonPressed);
    connect( ui->pauseButton, &QPushButton::clicked, this, &VMManagerMain::pauseButtonPressed);
    connect( ui->addButton, &QPushButton::clicked, this, &VMManagerMain::addButtonPressed);

    // Buttons are disabled by default
    ui->startStopButton->setEnabled(false);
    ui->settingsButton->setEnabled(false);
    ui->restartButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);

    vm_details = new VMManagerDetails();
    ui->scrollArea->setWidget(vm_details);
    QItemSelectionModel *selection_model = ui->listView->selectionModel();
    connect(selection_model, &QItemSelectionModel::currentChanged, this, &VMManagerMain::changeCurrent);
    // TODO: Once prefs are implemented, save the last row selected when closed and select here
    if (proxy_model->rowCount(QModelIndex()) > 0) {
        QModelIndex first_index = proxy_model->index(0, 0);
        ui->listView->setCurrentIndex(first_index);
    }

}

VMManagerMain::~VMManagerMain() {
    delete ui;
    delete vm_model;
}

void
VMManagerMain::updateSelection(const QItemSelection &selected,
                               const QItemSelection &deselected)
{
    qInfo("Manual selection here");

}

void
VMManagerMain::changeCurrent(const QModelIndex &current,
                             const QModelIndex &previous)
{
    Q_ASSERT(current.row() >= 0);
    QModelIndex mapped_index = proxy_model->mapToSource(current);
    selected_sysconfig = vm_model->getConfigObjectForIndex(mapped_index);
    vm_details->updateData(selected_sysconfig);
    // connection for process
    connect(selected_sysconfig->process, &QProcess::stateChanged, this, &VMManagerMain::refresh);
    refresh();

}

void
VMManagerMain::settingsButtonPressed() {
    if(!selected_sysconfig) {
        return;
    }
    selected_sysconfig->launchSettings();
    connect(selected_sysconfig->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){
                if ((exitCode != 0) || (exitStatus != QProcess::NormalExit)) {
                    qInfo().nospace().noquote() << "Abnormal program termination while launching settings: exit code " <<  exitCode << ", exit status " << exitStatus;
                    return;
                }
                selected_sysconfig->reloadConfig();
                vm_details->updateData(selected_sysconfig);
            });
}

void
VMManagerMain::startButtonPressed() {
    if(!selected_sysconfig) {
        return;
    }
    selected_sysconfig->startButtonPressed();
}

void
VMManagerMain::restartButtonPressed() {

    if(!selected_sysconfig) {
        return;
    }
    selected_sysconfig->restartButtonPressed();

}

void
VMManagerMain::pauseButtonPressed() {
    if(!selected_sysconfig) {
        return;
    }
    selected_sysconfig->pauseButtonPressed();
}
void
VMManagerMain::shutdownButtonPressed()
{
    if(!selected_sysconfig) {
        return;
    }
    selected_sysconfig->shutdownButtonPressed();
}
void
VMManagerMain::testButtonPressed()
{
    if(!selected_sysconfig) {
        return;
    }
    selected_sysconfig->sendClientScreenshotRequest();
}
void
VMManagerMain::refresh()
{
    ui->startStopButton->disconnect();
    bool running = selected_sysconfig->process->state() == QProcess::ProcessState::Running;
    //    The state of this button changes depending on the process state
    ui->startStopButton->disconnect();
    if(running) {
        ui->startStopButton->setEnabled(true);
        connect(ui->startStopButton, &QPushButton::clicked, this, &VMManagerMain::shutdownButtonPressed);
        ui->startStopButton->setText(tr("Shutdown"));
    } else {
        ui->startStopButton->setEnabled(true);
        connect(ui->startStopButton, &QPushButton::clicked, this, &VMManagerMain::startButtonPressed);
        ui->startStopButton->setText(tr("Start"));
    }
    // FIXME: True as long as the object is valid. Need a better way to determine if it is valid
    if(!selected_sysconfig->config_file.path().isEmpty()) {
        ui->settingsButton->setEnabled(true);
    }
    ui->restartButton->setEnabled(running);
    ui->pauseButton->setEnabled(running);
}
void
VMManagerMain::addButtonPressed()
{
    QString dir_selection = QFileDialog::getExistingDirectory(this, tr("Select directory for the new VM"), vmm_path, QFileDialog::ShowDirsOnly);
    if(dir_selection.isEmpty()) {
        // Canceled
        return;
    }
    auto new_directory = QDir(dir_selection);
    // qt replaces `/` with native separators
    auto new_config_file = QFileInfo(new_directory.path() + "/" + "86box.cfg");
    if(new_config_file.exists()) {
        // Already a config there
        QMessageBox::critical(this, tr("Directory in use"), tr("The selected directory is already in use. Please select a different directory."));
        return;
    }
    auto new_system = new VMManagerSystem(new_config_file);
    new_system->launchSettings();
    // Handle this in a closure so we can capture the temporary new_system object
    connect(new_system->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){
                if ((exitCode != 0) || (exitStatus != QProcess::NormalExit)) {
                    qInfo().nospace().noquote() << "Abnormal program termination while creating new system: exit code " <<  exitCode << ", exit status " << exitStatus;
                    qInfo() << "Not adding system due to errors";
                    delete new_system;
                    return;
                }
                if(!new_system->config_file.exists()) {
                    // No config file which means the cancel button was pressed in the settings dialog
                    delete new_system;
                    return;
                }
                auto current_index = ui->listView->currentIndex();
                vm_model->reload();
                auto created_object = vm_model->getIndexForConfigFile(new_system->config_file);
                if (created_object.row() < 0) {
                    // For some reason the index of the new object couldn't be determined. Fall back to the old index.
                    ui->listView->setCurrentIndex(current_index);
                    delete new_system;
                    return;
                }
                // Get the index of the newly-created system and select it
                QModelIndex mapped_index = proxy_model->mapFromSource(created_object);
                ui->listView->setCurrentIndex(mapped_index);
                delete new_system;
            });
}
