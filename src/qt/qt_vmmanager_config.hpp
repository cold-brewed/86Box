#ifndef QT_VMMANAGER_CONFIG_H
#define QT_VMMANAGER_CONFIG_H

#include <QSettings>

class VMManagerConfig : QObject {
    Q_OBJECT

public:
    enum class ConfigType {
        General,
        System,
    };
    Q_ENUM(ConfigType);

    explicit VMManagerConfig(ConfigType type, const QString& section = {});
    ~VMManagerConfig() override;
    void initConfig();
    QString getValue(const QString& key) const;
    bool setValue(const QString& key, const QString& value) const;

    QSettings *settings;
    ConfigType config_type;
    QString system_name;
};

#endif // QT_VMMANAGER_CONFIG_H