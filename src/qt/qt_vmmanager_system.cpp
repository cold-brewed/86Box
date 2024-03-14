/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		86Box VM manager system module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#include <QString>
#include <QDirIterator>
#include <QDebug>
#include <QTimer>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QtNetwork>
#include "qt_vmmanager_system.hpp"
#include "qt_vmmanager_details_section.hpp"

extern "C" {
#include <86box/86box.h>
#include <86box/device.h>
#include <86box/video.h>
#include <86box/vid_xga_device.h>
#include <86box/machine.h>
//#include <86box/hdd.h>
#include <86box/sound.h>
#include <cpu.h>
}

VMManagerSystem::VMManagerSystem(QFileInfo sysconfig_file)  {

    config_file = sysconfig_file;
    config_name = config_file.dir().dirName();
    config_dir = shortened_dir = config_file.dir().path();
    process_status = VMManagerSystem::ProcessStatus::Stopped;
#if not defined(Q_OS_WINDOWS)
    if (config_dir.startsWith(QDir::homePath())) {
        shortened_dir.replace(QDir::homePath(), "~");
    }
#endif
    loadSettings();
    setupPaths();
    // Paths must be setup before vars!
    setupVars();

    serverIsRunning = false;
    window_obscured = false;

    find86BoxBinary();
    platform = QApplication::platformName();
    process = new QProcess();
    connect(process, &QProcess::stateChanged, this, &VMManagerSystem::processStatusChanged);

//    auto one_sec_timer = new QTimer(this);
//    QObject::connect(one_sec_timer, &QTimer::timeout, this, &VMManagerSystem::statusRefresh);
//    one_sec_timer->setTimerType(Qt::TimerType::PreciseTimer);
//    one_sec_timer->start(1000);

    // Server type for this instance
    socket_server_type = VMManagerServerSocket::ServerType::Standard;
    socket_server = new VMManagerServerSocket(config_file, socket_server_type);
    // NOTE: When unique names or UUIDs are written to the individual VM config file, use that
    // here instead of the auto-generated unique_name
    // Persistent config temporarily disabled below
//    config_settings = new VMManagerConfig(VMManagerConfig::ConfigType::System, unique_name);
//    config_settings->setValue("system_name", config_name);
}

VMManagerSystem::VMManagerSystem() {
    serverIsRunning = false;
    //    legacy_server = nullptr ; // FIXME: I (temporarily) want the program to crash if I try to use the object from *this* constructor
}

VMManagerSystem::~VMManagerSystem() {
    delete socket_server;
}

QVector<VMManagerSystem *>
VMManagerSystem::scanForConfigs() {
    QVector<VMManagerSystem *> system_configs;

    QString config_file_name = "86box.cfg";
    QFileInfoList matches;
    // FIXME: Preferences. Multiple locations would be preferred.
    QString search_directory = vmm_path;

    QDirIterator dir_iterator(search_directory, QDir::Files, QDirIterator::Subdirectories);

    qInfo("Searching %s for %s", qPrintable(search_directory), qPrintable(config_file_name));

    while (dir_iterator.hasNext()) {
        QString filename = dir_iterator.next();
        QFileInfo file(filename);
        // We only care about files
        if (file.isDir()) {
            continue;
        }
        if (QString::localeAwareCompare(config_file_name, file.fileName()) == 0) {
            matches.append(file);
        }

    }

    qInfo("Found %i configs in %s", matches.size(), qPrintable(search_directory));
    foreach (QFileInfo hit, matches) {
        system_configs.append(new VMManagerSystem(hit));
    }
    return system_configs;
}

QFileInfoList
VMManagerSystem::getScreenshots() {

    // Don't bother unless the directory exists
    if(!screenshot_directory.exists()) {
        return {};
    }

    auto screen_scan_dir = QDir(screenshot_directory.path(), "Monitor_1*", QDir::SortFlag::LocaleAware | QDir::SortFlag::IgnoreCase, QDir::Files);
    auto screenshot_files = screen_scan_dir.entryInfoList();
    return screenshot_files;
}

void
VMManagerSystem::loadSettings() {
    QSettings settings(config_file.filePath(), QSettings::IniFormat);
    qInfo() << "Loaded "<< config_file.filePath() << "status:" << settings.status();

    // Clear out the config hash in case the config is reloaded
    for ( const auto& outer_key: config_hash.keys()) {
        config_hash[outer_key].clear();
    }

    // General
    for ( const auto& key_name : settings.childKeys()) {
        config_hash["General"][key_name] = settings.value(key_name).toString();
    }

    for ( auto& group_name : settings.childGroups()) {
        settings.beginGroup(group_name);
        for ( const auto& key_name : settings.allKeys()) {
            QString setting_value;
            // QSettings will interpret lines with commas as QStringList.
            // Check for it and join them back to a string.
            if (settings.value(key_name).type() == QVariant::StringList ) {
                setting_value = settings.value(key_name).toStringList().join(", ");
            } else {
                setting_value = settings.value(key_name).toString();
            }
            config_hash[group_name][key_name] = setting_value;
        }
        settings.endGroup();
    }
}

QString
VMManagerSystem::getAll(const QString& category) const {
    auto value = config_hash[category].keys().join(", ");
    return value;
}

QHash <QString, QHash <QString, QString>>
VMManagerSystem::getConfigHash() const {
    return config_hash;
}

QHash<QString, QString>
VMManagerSystem::getCategory(const QString &category) const {
    return config_hash[category];
}

void
VMManagerSystem::find86BoxBinary() {
    // We'll use our own self to launch the VMs
    main_binary = QFileInfo(QCoreApplication::applicationFilePath());
}

bool
VMManagerSystem::has86BoxBinary() {
    return main_binary.exists();
}

void
VMManagerSystem::launchMainProcess() {

    if(!has86BoxBinary()) {
        qWarning("No binary found! returning");
        return;
    }

    // start the server first to get the socket name
    if (!serverIsRunning) {
        if(!startServer()) {
            // FIXME: Better error handling
            qInfo("Failed to start VM Manager server");
            return;
        }
    }
    setProcessEnvVars();
    QString program = main_binary.filePath();
    QStringList args;
    args << "-P" << config_dir;
    process->setProgram(program);
    process->setArguments(args);
    qDebug() << Q_FUNC_INFO << " Full Command:" << process->program() << " " << process->arguments();
    process->start();
}

void
VMManagerSystem::startButtonPressed() {
    launchMainProcess();
}

void
VMManagerSystem::launchSettings() {
    if(!has86BoxBinary()) {
        qWarning("No binary found! returning");
        return;
    }

    // If the system is already running, instruct it to show settings
    if (process->processId() != 0) {
        socket_server->serverSendMessage(VMManagerProtocol::ManagerMessage::ShowSettings);
        return;
    }

    // Otherwise, launch the system with the settings parameter
    setProcessEnvVars();
    QString program = main_binary.filePath();
    QStringList open_command_args;
    QStringList args;
    args << "-P" << config_dir << "-S";
    process->setProgram(program);
    process->setArguments(args);
    qDebug() << Q_FUNC_INFO << " Full Command:" << process->program() << " " << process->arguments();
    process->start();
}

void
VMManagerSystem::setupPaths() {
    application_temp_directory.setPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    standard_temp_directory.setPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    QString temp_subdir = QApplication::applicationName();
    if (!application_temp_directory.exists(temp_subdir)) {
        // FIXME: error checking
        application_temp_directory.mkdir(temp_subdir);
    }
    // QT always replaces `/` with native separators, so it is safe to use here for all platforms
    application_temp_directory.setPath(application_temp_directory.path() + "/" + temp_subdir);
    app_data_directory.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!app_data_directory.exists()) {
        // FIXME: Error checking
        app_data_directory.mkpath(app_data_directory.path());
    }
    screenshot_directory.setPath(config_dir + "/" + "screenshots");
}

void
VMManagerSystem::setupVars() {
    unique_name = QCryptographicHash::hash(config_file.path().toUtf8().constData(), QCryptographicHash::Algorithm::Sha256).toHex().right(9);
    // Set up the display vars
    // This will likely get moved out to its own class
    // This will likely get moved out to its own class
    auto machine_config = getCategory("Machine");
    auto video_config = getCategory("Video");
    auto disk_config = getCategory("Hard disks");
    auto audio_config = getCategory("Sound");
    auto machine_name = QString();
    int i = 0;
    int ram_granularity = 0;
    // Machine
    for (int ci = 0; ci < machine_count(); ++ci) {
        if (machine_available(ci)) {
                if (machines[ci].internal_name == machine_config["machine"]) {
                    machine_name = machines[ci].name;
                    ram_granularity = machines[ci].ram.step;
                }
        }
    }
    display_table[Display::Name::Machine] = machine_name;

    // CPU: Combine name with speed
    auto cpu_name = QString();
    while (cpu_families[i].package != 0) {
        if (cpu_families[i].internal_name == machine_config["cpu_family"]) {
            cpu_name = QString("%1 %2").arg(cpu_families[i].manufacturer, cpu_families[i].name);
        }
        i++;
    }
    int speed_display = machine_config["cpu_speed"].toInt() / 1000000;
    cpu_name.append(QString::number(speed_display).prepend(" / "));
    cpu_name.append(QCoreApplication::translate("", "MHz").prepend(' '));
    display_table[Display::Name::CPU] = cpu_name;

    // Memory
    int divisor = (ram_granularity < 1024) ? 1 : 1024;
    QString display_unit = (divisor == 1) ? "KB" : "MB";
    auto mem_display = QString::number(machine_config["mem_size"].toInt() / divisor);
    mem_display.append(QCoreApplication::translate("", display_unit.toUtf8().constData()).prepend(' '));
    display_table[Display::Name::Memory] = mem_display;

    // Video card
    int video_int = video_get_video_from_internal_name(video_config["gfxcard"].toUtf8().data());
    const device_t* video_dev = video_card_getdevice(video_int);
    display_table[Display::Name::Video] = DeviceConfig::DeviceName(video_dev, video_get_internal_name(video_int), 1);
    if (!video_config["voodoo"].isEmpty()) {
        // FIXME: Come back to this later to add more for secondary video
//        display_table[Display::Name::Video].append(" (with voodoo)");
        display_table[Display::Name::Voodoo] = "Voodoo enabled";
    }

    // Drives
    // First the number of disks
    QMap<QString, int> disks;
    for(const auto& key: disk_config.keys()) {
        // Assuming the format hdd_NN_*
        QStringList pieces = key.split('_');
        QString disk = QString("%1_%2").arg(pieces.at(0), pieces.at(1));
        if(!disk.isEmpty()) {
            disks[disk] = 1;
        }
    }
    // Next, the types
    QHash<QString, int> bus_types;
    for (const auto& key: disks.keys()) {
        auto        disk_parameter_key = QString("%1_parameters").arg(key);
        QStringList pieces = disk_config[disk_parameter_key].split(",");
        QString bus_type = pieces.value(pieces.length() - 1).trimmed();
        bus_types[bus_type] = 1;
    }
    QString disks_display = tr("%n disk(s)", "", disks.count());
    if (disks.count()) {
        disks_display.append(" / ").append(bus_types.keys().join(", ").toUpper());
    }
//    display_table[Display::Name::Disks] = disks_display;

    // Drives
    QString new_disk_display;
    for (const auto& key: disks.keys()) {
        auto        disk_parameter_key = QString("%1_parameters").arg(key);
        // Converting a string to an int back to a string to remove the zero (e.g. 01 to 1)
        auto disk_number = QString::number(key.split("_").last().toInt());
        QStringList pieces = disk_config[disk_parameter_key].split(",");
        QString sectors = pieces.value(0).trimmed();
        QString heads = pieces.value(1).trimmed();
        QString cylinders = pieces.value(2).trimmed();
        QString bus_type = pieces.value(pieces.length() - 1).trimmed();
        // Add separator for each subsequent value, skipping the first
        if(!new_disk_display.isEmpty()) {
            new_disk_display.append(QString("%1").arg(VMManagerDetailsSection::sectionSeparator));
        }
        int diskSizeRaw = (cylinders.toInt() * heads.toInt() * sectors.toInt()) >> 11;
        QString diskSizeFinal;
        QString unit = "MiB";
        if(diskSizeRaw > 1000) {
            unit = "GiB";
            diskSizeFinal = QString::number(diskSizeRaw * 1.0 / 1000, 'f', 1);
        } else {
            diskSizeFinal = QString::number(diskSizeRaw);
        }
        // Only prefix each disk when there are multiple disks
        QString diskNumberDisplay = disks.count() > 1 ? QString("Disk %1: ").arg(disk_number) : "";
        new_disk_display.append(QString("%1%2 %3 %4").arg(diskNumberDisplay, diskSizeFinal, unit, bus_type.toUpper()));
    }
    if(new_disk_display.isEmpty()) {
        new_disk_display = "No disks";
    }
    display_table[Display::Name::Disks] = new_disk_display;

    // Audio
    int sound_int = sound_card_get_from_internal_name(audio_config["sndcard"].toUtf8().data());
    const device_t* audio_dev = sound_card_getdevice(sound_int);
    display_table[Display::Name::Audio] = DeviceConfig::DeviceName(audio_dev, sound_card_get_internal_name(sound_int), 1);
}

bool
VMManagerSystem::startServer() {
    if (socket_server->startServer()) {
        serverIsRunning = true;
        connect(socket_server, &VMManagerServerSocket::dataReceived, this, &VMManagerSystem::dataReceived);
        connect(socket_server, &VMManagerServerSocket::windowStatusChanged, this, &VMManagerSystem::windowStatusChangeReceived);
        connect(socket_server, &VMManagerServerSocket::runningStatusChanged, this, &VMManagerSystem::runningStatusChangeReceived);
        return true;
    } else {
        return false;
    }
}

void
VMManagerSystem::setProcessEnvVars() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString env_var_name = (socket_server_type == VMManagerServerSocket::ServerType::Standard) ? "VMM_86BOX_SOCKET" : "86BOX_MANAGER_SOCKET";
    env.insert(env_var_name, socket_server->getSocketPath());
    process->setProcessEnvironment(env);
}

void
VMManagerSystem::restartButtonPressed() {
    socket_server->serverSendMessage(VMManagerProtocol::ManagerMessage::ResetVM);

}

void
VMManagerSystem::pauseButtonPressed() {
    socket_server->serverSendMessage(VMManagerProtocol::ManagerMessage::Pause);
}
void
VMManagerSystem::dataReceived()
{
    qInfo() << Q_FUNC_INFO << "Note: Respond to data received events here.";
}
void
VMManagerSystem::windowStatusChangeReceived(int status)
{
    window_obscured = status;
    emit windowStatusChanged();
    processStatusChanged();
}
QString
VMManagerSystem::getDisplayValue(Display::Name key)
{
    return (display_table.contains(key)) ? display_table[key] : "";
}
void
VMManagerSystem::shutdownButtonPressed()
{
    socket_server->serverSendMessage(VMManagerProtocol::ManagerMessage::RequestShutdown);
}
void
VMManagerSystem::processStatusChanged()
{
    // set to running if the process is running and the state is stopped
    if (process->state() == QProcess::ProcessState::Running) {
        if (process_status == VMManagerSystem::ProcessStatus::Stopped) {
            process_status = VMManagerSystem::ProcessStatus::Running;
        }
    } else if (process->state() == QProcess::ProcessState::NotRunning) {
        process_status = VMManagerSystem::ProcessStatus::Stopped;
    }
    emit itemDataChanged();
}
void
VMManagerSystem::statusRefresh()
{
    processStatusChanged();
}
QString
VMManagerSystem::processStatusToString(VMManagerSystem::ProcessStatus status)
{
//    QMetaEnum qme = QMetaEnum::fromType<VMManagerSystem::ProcessStatus>();
//    return qme.valueToKey(static_cast<int>(status));
        switch (status) {
            case VMManagerSystem::ProcessStatus::Stopped:
                return tr("Powered Off");
            case VMManagerSystem::ProcessStatus::Running:
                return tr("Running");
            case VMManagerSystem::ProcessStatus::Paused:
                return tr("Paused");
            case VMManagerSystem::ProcessStatus::PausedWaiting:
            case VMManagerSystem::ProcessStatus::RunningWaiting:
                return tr("Paused (Waiting)");
            default:
                return tr("Unknown Status");
        }
}

QString
VMManagerSystem::getProcessStatusString() const
{
    return processStatusToString(process_status);
}

VMManagerSystem::ProcessStatus
VMManagerSystem::getProcessStatus() const
{
    return process_status;
}
// Maps VMManagerProtocol::RunningState to VMManagerSystem::ProcessStatus
void
VMManagerSystem::runningStatusChangeReceived(VMManagerProtocol::RunningState state)
{
    if(state == VMManagerProtocol::RunningState::Running) {
        process_status = VMManagerSystem::ProcessStatus::Running;
    } else if(state == VMManagerProtocol::RunningState::Paused) {
        process_status = VMManagerSystem::ProcessStatus::Paused;
    } else if(state == VMManagerProtocol::RunningState::RunningWaiting) {
        process_status = VMManagerSystem::ProcessStatus::RunningWaiting;
    } else if(state == VMManagerProtocol::RunningState::PausedWaiting) {
        process_status = VMManagerSystem::ProcessStatus::PausedWaiting;
    } else {
        process_status = VMManagerSystem::ProcessStatus::Unknown;
    }
    processStatusChanged();
}
void
VMManagerSystem::reloadConfig()
{
    loadSettings();
    setupVars();
}
