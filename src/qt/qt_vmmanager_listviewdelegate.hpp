
#ifndef QT_VMMANAGER_LISTVIEWDELEGATE_H
#define QT_VMMANAGER_LISTVIEWDELEGATE_H

#include <QPainter>
#include <QStyledItemDelegate>
#include "qt_vmmanager_system.hpp"

class VMManagerListViewDelegatePrivate
{
    VMManagerListViewDelegatePrivate();

    inline QRect systemNameBox(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    inline qreal statusFontPointSize(const QFont &f) const;
    inline QRect statusBox(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QSize iconSize;
    QSize smallIconSize;
    QMargins margins;
    int spacingHorizontal;
    int spacingVertical;

    friend class VMManagerListViewDelegate;
};

class VMManagerListViewDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit VMManagerListViewDelegate(QObject *parent = nullptr);
    ~VMManagerListViewDelegate() override;
    using QStyledItemDelegate::QStyledItemDelegate;

    QMargins contentsMargins() const;
    void setContentsMargins(int left, int top, int right, int bottom);

    int horizontalSpacing() const;
    void setHorizontalSpacing(int spacing);

    int verticalSpacing() const;
    void setVerticalSpacing(int spacing);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
private:
    VMManagerListViewDelegatePrivate *m_ptr;
};
#endif // QT_VMMANAGER_LISTVIEWDELEGATE_H