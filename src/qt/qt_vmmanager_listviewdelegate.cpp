
#include "qt_vmmanager_listviewdelegate.hpp"
#include <QApplication>




// Thanks to scopchanov https://github.com/scopchanov/SO-MessageLog
// from https://stackoverflow.com/questions/53105343/is-it-possible-to-add-a-custom-widget-into-a-qlistview


VMManagerListViewDelegate::VMManagerListViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
    m_ptr(new VMManagerListViewDelegatePrivate)
{
}

VMManagerListViewDelegate::~VMManagerListViewDelegate()
{
}

void VMManagerListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    // I see a red door and I want to paint it black
//    painter->fillRect(option.rect.adjusted(1, 1, -1, -1), Qt::SolidPattern);
//    return;

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    const QPalette &palette(opt.palette);
    const QRect &rect(opt.rect);
    const QRect &contentRect(rect.adjusted(m_ptr->margins.left(),
                                              m_ptr->margins.top(),
                                              -m_ptr->margins.right(),
                                              -m_ptr->margins.bottom()));
//    QIcon stopped_icon = QApplication::style()->standardIcon(QStyle::SP_MediaStop);
    QIcon status_icon = QApplication::style()->standardIcon(QStyle::SP_MediaStop);;
    auto process_variant = index.data(Qt::UserRole + 2);
    auto process_status = process_variant.value<VMManagerSystem::ProcessStatus>();
    opt.icon = QIcon(":/settings/win/icons/86Box-gray.ico");
    switch(process_status) {
        case VMManagerSystem::ProcessStatus::Running:
//            status_icon = QIcon(":/vmm/play-16.png");
            status_icon = QIcon(":/menuicons/win/icons/run.ico");
//            status_icon = QApplication::style()->standardIcon(QStyle::SP_MediaPlay);
//            opt.icon = QIcon(":/settings/win/icons/86Box-gray.ico");
            // stuff
            break;
            // FIXME: redundant - remove
        case VMManagerSystem::ProcessStatus::Stopped:
//            status_icon = QApplication::style()->standardIcon(QStyle::SP_MediaStop);
//            status_icon = QIcon(":/vmm/stop-16.png");
            status_icon = QIcon(":/menuicons/win/icons/acpi_shutdown.ico");

//            status_icon = QIcon(":/vmm/red-power-16.png");
//            opt.icon = QIcon(":/settings/win/icons/86Box-gray.ico");
            //
            break;
        case VMManagerSystem::ProcessStatus::PausedWaiting:
        case VMManagerSystem::ProcessStatus::RunningWaiting:
        case VMManagerSystem::ProcessStatus::Paused:
//            status_icon = QIcon(":/vmm/pause-16.png");
            status_icon = QIcon(":/menuicons/win/icons/pause.ico");
//            status_icon = QApplication::style()->standardIcon(QStyle::SP_MediaPause);
            break;
        default:
            status_icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    }


    const bool lastIndex = (index.model()->rowCount() - 1) == index.row();
    const bool hasIcon = !opt.icon.isNull();
    const int bottomEdge = rect.bottom();
    QFont f(opt.font);

    f.setPointSize(m_ptr->statusFontPointSize(opt.font));

    painter->save();
    painter->setClipping(true);
    painter->setClipRect(rect);
    painter->setFont(opt.font);

    // Draw background
    painter->fillRect(rect, opt.state & QStyle::State_Selected ?
                                                               palette.highlight().color() :
                                                               palette.light().color());

    // Draw bottom line
    painter->setPen(lastIndex ? palette.dark().color()
                              : palette.mid().color());
    painter->drawLine(lastIndex ? rect.left() : m_ptr->margins.left(),
                      bottomEdge, rect.right(), bottomEdge);

    // Draw message icon
    if (hasIcon) {
        painter->drawPixmap(contentRect.left(), contentRect.top(),
                            opt.icon.pixmap(m_ptr->iconSize));
    }
//    painter->drawPixmap(contentRect.left(), contentRect.top(),
//                        QApplication::style()->standardIcon(QStyle::SP_MediaPlay).pixmap(32));
    // QApplication::style()->standardIcon(QStyle::SP_MediaPlay))
    // QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward)
    // Draw timestamp
    QRect systemNameRect(m_ptr->systemNameBox(opt, index));

    systemNameRect.moveTo(m_ptr->margins.left() + m_ptr->iconSize.width()
                             + m_ptr->spacingHorizontal, contentRect.top());

//    painter->setFont(f);
    painter->setFont(opt.font);
    painter->setPen(palette.text().color());
//    painter->drawText(systemNameRect, Qt::TextSingleLine,
//                      index.data(Qt::UserRole).toString());
//    painter->drawText(systemNameRect, Qt::TextSingleLine,
//                      index.data(Qt::ToolTipRole).toString());
//    painter->drawText(systemNameRect, Qt::TextSingleLine,
//                      index.data(Qt::UserRole + 1).toString());
//    painter->drawRect(systemNameRect);
    painter->drawText(systemNameRect, Qt::TextSingleLine, opt.text);

    // Draw status icon
    painter->drawPixmap(systemNameRect.left(), systemNameRect.bottom()
                            + m_ptr->spacingVertical,
                        status_icon.pixmap(m_ptr->smallIconSize));
//    auto blah = QRect()
    auto point = QPoint(systemNameRect.left(), systemNameRect.bottom()
                            + m_ptr->spacingVertical);
    auto point2 = QPoint(point.x() + m_ptr->smallIconSize.width(), point.y() + m_ptr->smallIconSize.height());
    auto arect = QRect(point, point2);
//    painter->drawRect(arect);
    // Draw status text
    QRect statusRect(m_ptr->statusBox(opt, index));

//    statusRect.moveTo(systemNameRect.left() + m_ptr->margins.left() + m_ptr->smallIconSize.width(), systemNameRect.bottom()
//                           + m_ptr->spacingVertical);
//    statusRect.moveTo(systemNameRect.left() + m_ptr->margins.left() + m_ptr->smallIconSize.width(),
//                      systemNameRect.bottom() + statusRect.height() * 0.5);
    int extraaa = 2;
//    qInfo().noquote() << "systemNameRect.height()" << systemNameRect.height() << "m_ptr->smallIconSize.height()" << m_ptr->smallIconSize.height() << "adding" << extraaa;
    statusRect.moveTo(systemNameRect.left() + m_ptr->margins.left() + m_ptr->smallIconSize.width(),
                      systemNameRect.bottom() + m_ptr->spacingVertical + extraaa + (m_ptr->smallIconSize.height() - systemNameRect.height() ));
//                      systemNameRect.bottom() + (m_ptr->spacingVertical + (m_ptr->smallIconSize.height() / 2) - (systemNameRect.height() / 2) ));
//                      systemNameRect.bottom() + (margin + icon - (font / 2)));

//    statusRect.moveTo(opt.rect.width() - statusRect.size().width() - m_ptr->margins.right(), systemNameRect.bottom()
//                          + m_ptr->spacingVertical);

//    painter->setFont(opt.font);
    painter->setFont(f);
    painter->setPen(palette.windowText().color());
//    painter->drawText(statusRect, Qt::TextSingleLine, opt.text);
//    painter->drawRect(statusRect);
    painter->drawText(statusRect, Qt::TextSingleLine,
                      index.data(Qt::UserRole + 1).toString());

    painter->restore();

}

QMargins VMManagerListViewDelegate::contentsMargins() const
{
    return m_ptr->margins;
}

void VMManagerListViewDelegate::setContentsMargins(int left, int top, int right, int bottom)
{
    m_ptr->margins = QMargins(left, top, right, bottom);
}

int VMManagerListViewDelegate::horizontalSpacing() const
{
    return m_ptr->spacingHorizontal;
}

void VMManagerListViewDelegate::setHorizontalSpacing(int spacing)
{
    m_ptr->spacingHorizontal = spacing;
}

int VMManagerListViewDelegate::verticalSpacing() const
{
    return m_ptr->spacingVertical;
}

void VMManagerListViewDelegate::setVerticalSpacing(int spacing)
{
    m_ptr->spacingVertical = spacing;
}


QSize VMManagerListViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    int textHeight = m_ptr->systemNameBox(opt, index).height()
        + m_ptr->spacingVertical + m_ptr->statusBox(opt, index).height();
    int iconHeight = m_ptr->iconSize.height();
    int h = textHeight > iconHeight ? textHeight : iconHeight;

    return QSize(opt.rect.width(), m_ptr->margins.top() + h
                     + m_ptr->margins.bottom());
}

VMManagerListViewDelegatePrivate::VMManagerListViewDelegatePrivate() :
    iconSize(32, 32),
    smallIconSize(16, 16),
    margins(4, 8, 8, 8),
    spacingHorizontal(4),
    spacingVertical(4)
{

}

QRect VMManagerListViewDelegatePrivate::statusBox(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QFont f(option.font);

    f.setPointSizeF(statusFontPointSize(option.font));

    return QFontMetrics(f).boundingRect(index.data(Qt::UserRole + 1).toString())
        .adjusted(0, 0, 1, 1);
}

qreal VMManagerListViewDelegatePrivate::statusFontPointSize(const QFont &f) const
{
    return 0.75*f.pointSize();
//    return 1*f.pointSize();
}

QRect VMManagerListViewDelegatePrivate::systemNameBox(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return option.fontMetrics.boundingRect(option.text).adjusted(0, 0, 1, 1);
}