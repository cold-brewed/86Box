#ifndef QT_HARDDISKDIALOG_HPP
#define QT_HARDDISKDIALOG_HPP

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>
#include <optional>

namespace Ui {
class HarddiskDialog;
}

class HarddiskDialog : public QDialog {
    Q_OBJECT

public:
    explicit HarddiskDialog(bool existing, QWidget *parent = nullptr);
    ~HarddiskDialog();

    uint8_t  bus() const;
    uint8_t  channel() const;
    QString  fileName() const;
    uint32_t cylinders() const { return cylinders_; }
    uint32_t heads() const { return heads_; }
    uint32_t sectors() const { return sectors_; }
    uint32_t speed() const;

    struct HDDPreset {
        int     id;
        QString manufacturer;
        QString model;
        int     size;
        int     c;
        int     h;
        int     s;
        QString bus;
    };

signals:
    void fileProgress(int i);

public slots:
    void accept() override;

private slots:
    void on_lineEditSectors_textEdited(const QString &arg1);
    void on_lineEditHeads_textEdited(const QString &arg1);
    void on_lineEditCylinders_textEdited(const QString &arg1);
    void on_lineEditSize_textEdited(const QString &arg1);
    void on_comboBoxBus_currentIndexChanged(int index);
    void on_comboBoxFormat_currentIndexChanged(int index);
    void onCreateNewFile();
    void onExistingFileSelected(const QString &fileName, bool precheck);
    void populatePresetModels(int index) const;
    void setPresetParameters(int index);
    void presetTextChanged(const QString &text);
    void sizeSliderChanged(int value);
    void sizeSliderMoved(int value) const;
    void sizeSliderRangeChanged(int min, int max) const;

private:
    enum HDDPresetModelRoles {
        Manufacturer = Qt::UserRole + 1,
        Model,
        Size,
        Cylinders,
        Heads,
        Sectors,
        Bus,
    };

    Ui::HarddiskDialog *ui;

    uint32_t cylinders_ = 0;
    uint32_t heads_ = 0;
    uint32_t sectors_ = 0;

    uint32_t max_sectors   = 0;
    uint32_t max_heads     = 0;
    uint32_t max_cylinders = 0;

    bool disallowSizeModifications = false;

    QStringList filters;
    // "Dynamic-size VHD" is number 4 in the `filters` list and the
    // comboBoxFormat model
    const uint8_t  DEFAULT_DISK_FORMAT     = 4;
    const uint16_t DEFAULT_MAX_SIZE_SLIDER = 4096;

    bool checkAndAdjustCylinders();
    bool checkAndAdjustHeads();
    bool checkAndAdjustSectors();
    void recalcSize();
    void recalcSelection();
    void parsePresetList();
    static std::optional<HDDPreset> presetFromJson(const QJsonObject& json);
    QStringListModel*      createManufacturersModel();
    QStandardItemModel*    createHddPresetModel();
    QSortFilterProxyModel* createHddPresetProxyModel(QStandardItemModel* sourceModel);

    QList<HDDPreset>       presets;
    QSortFilterProxyModel *hddPresetModel;
};

#endif // QT_HARDDISKDIALOG_HPP
