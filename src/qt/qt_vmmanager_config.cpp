#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include "qt_vmmanager_config.hpp"

extern "C" {
#include <86box/path.h>
}

VMManagerConfig::VMManagerConfig(ConfigType type, const QString& section)
{
    QString organization_name;
#ifdef Q_OS_MAC
    organization_name = "net.86Box.86Box";
    // On macos, QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)) resolves to ~/Library/Application Support
    auto config_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir(config_dir).path());
#else
    organization_name = "86box";
#endif

//    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
//                             "86box", "86Box");
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                             organization_name, "VMManager");
    settings->setFallbacksEnabled(false);
    // General or machine?
    // if a section name is set, it automatically is set to machine
    if (section.isEmpty()) {
        config_type = VMManagerConfig::ConfigType::General;
//        settings->beginGroup("VMManager");
        qInfo() << "Config file will use section [General]";
    } else {
        config_type = VMManagerConfig::ConfigType::System;
        system_name = section;
        settings->beginGroup(system_name);
        qInfo().noquote().nospace() << "Config file will use section [" << system_name << "]";
    }
    auto extra_debug = section.isEmpty() ? QString() : " with name: " + section;
    qInfo().noquote() << Q_FUNC_INFO << "Created configuration object of type" << type << extra_debug;
    qInfo().noquote() << "Located at " << settings->fileName();
}

VMManagerConfig::~VMManagerConfig() {
    settings->endGroup();
}

void
VMManagerConfig::initConfig()
{
}
QString
VMManagerConfig::getValue(const QString& key) const
{
    auto value = settings->value(key);
    auto return_value = value.type() == QVariant::Invalid ? QString() : value.toString();
    return return_value;
}
bool
VMManagerConfig::setValue(const QString& key, const QString& value) const
{
    qInfo().nospace().noquote() << Q_FUNC_INFO << " Writing value \"" << value << "\" to key \"" << key << "\"";
    settings->setValue(key, value);
    return false;
}

