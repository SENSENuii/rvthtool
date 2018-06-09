/***************************************************************************
 * RVT-H Tool (librvth)                                                    *
 * RvtHModel.hpp: QAbstractListModel for RvtH objects.                     *
 *                                                                         *
 * Copyright (c) 2018 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#include "RvtHModel.hpp"

#include "librvth/rvth.h"

// C includes. (C++ namespace)
#include <cassert>

// Qt includes.
#include <QApplication>
#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtGui/QIcon>
#include <QtGui/QPalette>

/** RvtHModelPrivate **/

class RvtHModelPrivate
{
	public:
		explicit RvtHModelPrivate(RvtHModel *q);

	protected:
		RvtHModel *const q_ptr;
		Q_DECLARE_PUBLIC(RvtHModel)
	private:
		Q_DISABLE_COPY(RvtHModelPrivate)

	public:
		RvtH *rvth;

		// Style variables.
		struct style_t {
			/**
			 * Initialize the style variables.
			 */
			void init(void);

			// Background colors for "deleted" banks.
			QBrush brush_lostFile;
			QBrush brush_lostFile_alt;

			/**
			 * Load an icon from the Qt resources.
			 * @param dir Base directory.
			 * @param name Icon name.
			 */
			static QIcon loadIcon(const QString &dir, const QString &name);

			// Pixmaps for COL_ISVALID.
			static const QSize szPxmIsValid;
#if 0
			QPixmap pxmIsValid_unknown;
			QPixmap pxmIsValid_invalid;
			QPixmap pxmIsValid_good;
#endif

			// Monosapced font.
			QFont fntMonospace;

			// Icon IDs.
			enum IconID {
				ICON_GCN,	// GameCube (retail)
				ICON_NR,	// NR Reader (debug)

				ICON_MAX
			};

			/**
			 * Load an icon.
			 * @param id Icon ID.
			 * @return Icon.
			 */
			QIcon getIcon(IconID id) const;

		private:
			// Icons for COL_TYPE.
			mutable QIcon m_icons[ICON_MAX];
		};
		style_t style;

		/**
		 * Get the icon for the specified bank.
		 * @param bank Bank number.
		 * @return QIcon.
		 */
		QIcon iconForBank(unsigned int bank) const;
};

const QSize RvtHModelPrivate::style_t::szPxmIsValid = QSize(16, 16);

RvtHModelPrivate::RvtHModelPrivate(RvtHModel *q)
	: q_ptr(q)
	, rvth(nullptr)
{
	// Initialize the style variables.
	style.init();
}

/**
 * Load an icon from the Qt resources.
 * @param dir Base directory.
 * @param name Icon name.
 */
QIcon RvtHModelPrivate::style_t::loadIcon(const QString &dir, const QString &name)
{
	// Icon sizes.
	static const unsigned int icoSz[] = {32, 48, 64, 128, 0};

	QIcon icon;
	for (const unsigned int *p = icoSz; *p != 0; p++) {
		const QString s_sz = QString::number(*p);
		QString full_path = QLatin1String(":/") +
			dir + QChar(L'/') +
			s_sz + QChar(L'x') + s_sz + QChar(L'/') +
			name + QLatin1String(".png");;
		QPixmap pxm(full_path);
		if (!pxm.isNull()) {
			icon.addPixmap(pxm);
		}
	}

	return icon;
}

/**
 * Load an icon.
 * @param id Icon ID.
 * @return Icon.
 */
QIcon RvtHModelPrivate::style_t::getIcon(IconID id) const
{
	assert(id >= 0);
	assert(id < ICON_MAX);
	if (id < 0 || id >= ICON_MAX) {
		return QIcon();
	}

	if (m_icons[id].isNull()) {
		static const char *const names[] = {
			"gcn", "nr"
		};
		m_icons[id] = loadIcon(QLatin1String("hw"), QLatin1String(names[id]));
		assert(!m_icons[id].isNull());
	}

	return m_icons[id];
}

/**
 * Initialize the style variables.
 */
void RvtHModelPrivate::style_t::init(void)
{
	// TODO: Call this function if the UI style changes.

	// Initialize the background colors for "lost" files.
	QPalette pal = QApplication::palette("QTreeView");
	QColor bgColor_lostFile = pal.base().color();
	QColor bgColor_lostFile_alt = pal.alternateBase().color();

	// Adjust the colors to have a yellow hue.
	int h, s, v;

	// "Lost" file. (Main)
	bgColor_lostFile.getHsv(&h, &s, &v, nullptr);
	h = 60;
	s = (255 - s);
	bgColor_lostFile.setHsv(h, s, v);

	// "Lost" file. (Alternate)
	bgColor_lostFile_alt.getHsv(&h, &s, &v, nullptr);
	h = 60;
	s = (255 - s);
	bgColor_lostFile_alt.setHsv(h, s, v);

	// Save the background colors in QBrush objects.
	brush_lostFile = QBrush(bgColor_lostFile);
	brush_lostFile_alt = QBrush(bgColor_lostFile_alt);

	// TODO: Pixmaps.
#if 0
	// Initialize the COL_ISVALID pixmaps.
	// NOTE: Using Oxygen icons for all systems.
	// TODO: Better dialog-question icon.
	// TODO: Hi-DPI icon handling.
	static const QSize sz(16, 16);
	const QString s_sz = QString::number(sz.width()) +
		QChar(L'x') + QString::number(sz.height());
	pxmIsValid_unknown = QPixmap(QLatin1String(":/oxygen/") + s_sz +
		QLatin1String("/dialog-question.png"));
	pxmIsValid_invalid = QPixmap(QLatin1String(":/oxygen/") + s_sz +
		QLatin1String("/dialog-error.png"));
	pxmIsValid_good    = QPixmap(QLatin1String(":/oxygen/") + s_sz +
		QLatin1String("/dialog-ok-apply.png"));
#endif

	// Monospaced font.
	fntMonospace = QFont(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);
}

/**
 * Get the icon for the specified bank.
 * @param bank Bank number.
 * @return QIcon.
 */
QIcon RvtHModelPrivate::iconForBank(unsigned int bank) const
{
	if (!rvth) {
		// No RVT-H Reader image.
		return QIcon();
	}

	const RvtH_BankEntry *entry = rvth_get_BankEntry(rvth, bank, nullptr);
	assert(entry != nullptr);
	if (!entry) {
		// No bank entry here...
		return QIcon();
	}
	const RvtH_ImageType_e imageType = rvth_get_ImageType(rvth);

	switch (entry->type) {
		case RVTH_BankType_Empty:
		case RVTH_BankType_Unknown:
		case RVTH_BankType_Wii_DL_Bank2:
		default:
			// No icon for these banks.
			return QIcon();

		case RVTH_BankType_GCN:
			// GameCube.
			if (imageType == RVTH_ImageType_GCM) {
				// Standalone GCM. Use the retail icon.
				return style.getIcon(style_t::ICON_GCN);
			} else {
				// GCM with SDK header, or RVT-H Reader.
				// Use the NR Reader icon.
				return style.getIcon(style_t::ICON_NR);
			}
			break;

		case RVTH_BankType_Wii_SL:
		case RVTH_BankType_Wii_DL:
			// TODO: Wii icons.
			// Using NR Reader icon for now.
			if (imageType == RVTH_ImageType_GCM) {
				// Standalone GCM. Use the retail icon.
				return style.getIcon(style_t::ICON_GCN);
			} else {
				// GCM with SDK header, or RVT-H Reader.
				// Use the NR Reader icon.
				return style.getIcon(style_t::ICON_NR);
			}
			break;
	}

	assert(!"Should not get here!");
	return QIcon();
}

/** RvtHModel **/

RvtHModel::RvtHModel(QObject *parent)
	: super(parent)
	, d_ptr(new RvtHModelPrivate(this))
{
	// TODO: Not implemented yet...
#if 0
	// Connect the "themeChanged" signal.
	connect(qApp, SIGNAL(themeChanged()),
		this, SLOT(themeChanged_slot()));
#endif
}

RvtHModel::~RvtHModel()
{
	Q_D(RvtHModel);
	delete d;
}


int RvtHModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const RvtHModel);
	if (d->rvth) {
		return rvth_get_BankCount(d->rvth);
	}
	return 0;
}

int RvtHModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const RvtHModel);
	if (d->rvth) {
		return COL_MAX;
	}
	return 0;
}

QVariant RvtHModel::data(const QModelIndex& index, int role) const
{
	Q_D(const RvtHModel);
	if (!d->rvth || !index.isValid())
		return QVariant();
	if (index.row() >= rowCount())
		return QVariant();

	// Get the bank entry.
	const unsigned int bank = static_cast<unsigned int>(index.row());
	const RvtH_BankEntry *const entry = rvth_get_BankEntry(d->rvth, bank, nullptr);
	if (!entry) {
		// No entry...
		return QVariant();
	}

	// HACK: Increase icon width on Windows.
	// Figure out a better method later.
#ifdef Q_OS_WIN
	static const int iconWadj = 8;
#else
	static const int iconWadj = 0;
#endif

	switch (entry->type) {
		case RVTH_BankType_Empty:
			// Empty slot.
			switch (index.column()) {
				case COL_BANKNUM:
					// Bank number.
					switch (role) {
						case Qt::DisplayRole:
							return QString::number(bank + 1);
						case Qt::TextAlignmentRole:
							// Center-align the text.
							return Qt::AlignCenter;
						default:
							break;
					}
					break;

				case COL_TYPE:
					if (role == Qt::SizeHintRole) {
						// Using 32x32 icons.
						// (Hi-DPI is handled by Qt automatically.)
						QSize size(32 + iconWadj, 32);
						if (entry->type == RVTH_BankType_Wii_DL) {
							// Double-size the row.
							size.setHeight(size.height() * 2);
						}
						return size;
					}
					break;

				default:
					break;
			}
			// All other columns are empty.
			return QVariant();
		case RVTH_BankType_Wii_DL_Bank2:
			// TODO: Make the previous bank double-tall.
			return QVariant();
		default:
			break;
	}

	// TODO: Move some of this to RvtHItemDelegate?
	switch (role) {
		case Qt::DisplayRole:
			// TODO: Cache these?
			switch (index.column()) {
				case COL_BANKNUM: {
					QString banknum = QString::number(bank + 1);
					if (entry->type == RVTH_BankType_Wii_DL) {
						// Print both bank numbers.
						banknum += QChar(L'\n') + QString::number(bank + 2);
					}
					return banknum;
				}
				case COL_TITLE:
					// TODO: Convert from Japanese if necessary.
					// Also cp1252.
					return QString::fromLatin1(entry->discHeader.game_title, sizeof(entry->discHeader.game_title)).trimmed();
				case COL_GAMEID:
					return QLatin1String(entry->discHeader.id6, sizeof(entry->discHeader.id6));
				case COL_DISCNUM:
					return QString::number(entry->discHeader.disc_number);
				case COL_REVISION:
					// TODO: BCD?
					return QString::number(entry->discHeader.revision);
				case COL_REGION:
					// TODO: Icon?
					switch (entry->region_code) {
						default:
							return QString::number(entry->region_code);
						case GCN_REGION_JAPAN:
							return QLatin1String("JPN");
						case GCN_REGION_USA:
							return QLatin1String("USA");
						case GCN_REGION_PAL:
							return QLatin1String("EUR");
						case GCN_REGION_FREE:
							return QLatin1String("ALL");
						case GCN_REGION_SOUTH_KOREA:
							return QLatin1String("KOR");
					}
					break;

				case COL_IOS_VERSION:
					// Wii only.
					if (entry->type == RVTH_BankType_Wii_SL ||
					    entry->type == RVTH_BankType_Wii_DL)
					{
						return QString::number(entry->ios_version);
					}
					break;

				case COL_ENCRYPTION:
					switch (entry->crypto_type) {
						case RVTH_CryptoType_Unknown:
						default:
							return QString::number(entry->crypto_type);
						case RVTH_CryptoType_None:
							return tr("None");
						case RVTH_CryptoType_Debug:
							return tr("Debug");
						case RVTH_CryptoType_Retail:
							return tr("Retail");
						case RVTH_CryptoType_Korean:
							return tr("Korean");
					}
					break;

				case COL_SIG_TICKET:
				case COL_SIG_TMD:
				case COL_APPLOADER:
					// TODO
					break;

				default:
					break;
			}
			break;

		case Qt::DecorationRole:
			if (index.column() == COL_TYPE) {
				// Get the icon for this bank.
				return d->iconForBank(bank);
			}
			break;

		case Qt::TextAlignmentRole:
			switch (index.column()) {
				case COL_TITLE:
					// Left-align, center vertically.
					return (int)(Qt::AlignLeft | Qt::AlignVCenter);

				case COL_TYPE:
				case COL_BANKNUM:
				case COL_GAMEID:
				case COL_DISCNUM:
				case COL_REVISION:
				case COL_REGION:
				case COL_IOS_VERSION:
				case COL_ENCRYPTION:
				case COL_SIG_TICKET:
				case COL_SIG_TMD:
				case COL_APPLOADER:
				default:
					// Center-align the text.
					return Qt::AlignCenter;
			}

		case Qt::FontRole:
			switch (index.column()) {
				case COL_GAMEID: {
					// These columns should be monospaced.
					return d->style.fntMonospace;
				}

				default:
					break;
			}
			break;

		case Qt::BackgroundRole:
			// "Deleted" banks should be displayed using a different color.
			if (entry->is_deleted) {
				// TODO: Check if the item view is using alternating row colors before using them.
				if (bank & 1)
					return d->style.brush_lostFile_alt;
				else
					return d->style.brush_lostFile;
			}
			break;

		case Qt::SizeHintRole: {
			if (index.column() == COL_TYPE) {
				// Using 32x32 icons.
				// (Hi-DPI is handled by Qt automatically.)
				QSize size(32 + iconWadj, 32);
				if (entry->type == RVTH_BankType_Wii_DL) {
					// Double-size the row.
					size.setHeight(size.height() * 2);
				}
				return size;
			}
			break;
		}

		case DualLayerRole:
			return (entry->type == RVTH_BankType_Wii_DL);

		default:
			break;
	}

	// Default value.
	return QVariant();
}

QVariant RvtHModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);

	switch (role) {
		case Qt::DisplayRole:
			switch (section) {
				case COL_BANKNUM:	return tr("#");
				case COL_TYPE:		return tr("Type");
				case COL_TITLE:		return tr("Title");
				//: 6-digit game ID, e.g. GALE01.
				case COL_GAMEID:	return tr("Game ID");
				case COL_DISCNUM:	return tr("Disc #");
				case COL_REVISION:	return tr("Revision");
				case COL_REGION:	return tr("Region");
				case COL_IOS_VERSION:	return tr("IOS");
				case COL_ENCRYPTION:	return tr("Encryption");
				case COL_SIG_TICKET:	return tr("Ticket Sig");
				case COL_SIG_TMD:	return tr("TMD Sig");
				case COL_APPLOADER:	return tr("AppLoader");

				default:
					break;
			}
			break;

		case Qt::TextAlignmentRole:
			// Center-align the text.
			return Qt::AlignHCenter;
	}

	// Default value.
	return QVariant();
}

/**
 * Set the RVT-H Reader disk image to use in this model.
 * @param card RVT-H Reader disk image.
 */
void RvtHModel::setRvtH(RvtH *rvth)
{
	Q_D(RvtHModel);

	// NOTE: No signals, since librvth is a C library.

	// Disconnect the Card's changed() signal if a Card is already set.
	if (d->rvth) {
		// Notify the view that we're about to remove all rows.
		const int bankCount = rvth_get_BankCount(d->rvth);
		if (bankCount > 0) {
			beginRemoveRows(QModelIndex(), 0, (bankCount - 1));
		}

		d->rvth = nullptr;

		// Done removing rows.
		if (bankCount > 0) {
			endRemoveRows();
		}
	}

	if (rvth) {
		// Notify the view that we're about to add rows.
		const int bankCount = rvth_get_BankCount(rvth);
		if (bankCount > 0) {
			beginInsertRows(QModelIndex(), 0, (bankCount - 1));
		}

		d->rvth = rvth;

		// Done adding rows.
		if (bankCount > 0) {
			endInsertRows();
		}
	}
}

/** Slots. **/

/**
 * The system theme has changed.
 */
void RvtHModel::themeChanged_slot(void)
{
	// Reinitialize the style.
	Q_D(RvtHModel);
	d->style.init();

	// TODO: Force an update?
}