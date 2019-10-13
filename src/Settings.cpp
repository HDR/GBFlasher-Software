#include <QLabel>
#include <QLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QSettings>
#include "Settings.h"
#include "Gui.h"
#include <math.h>

bool
  Settings::commanual = 0;
alg_t
  Settings::algorythm = ALG16;
dap_t
  Settings::dap = TOGGLE;
bool
  Settings::showbbl = 0;
speed_type
  Settings::speed = STANDARD;
bool
  Settings::darkmode = 0;
QThread::Priority Settings::priority = QThread::NormalPriority;

Settings::Settings (QWidget * parent):QGroupBox (tr ("Settings"), parent)
{
  QSettings
    set;

  labels = new QVBoxLayout ();
  combo_boxes = new QVBoxLayout ();
  down = new QHBoxLayout ();
  north = new QHBoxLayout ();
  all = new QVBoxLayout ();

  mbc_label = new QLabel ("MBC:", this);

  labels->addWidget (mbc_label);
  mbc_combo = new QComboBox (this);
  combo_boxes->addWidget (mbc_combo);
  mbc_combo->insertItem (0, "MBC AUTO");
  mbc_combo->insertItem (1, "MBC1");
  mbc_combo->insertItem (2, "MBC2");
  mbc_combo->insertItem (3, "MBC3");
  mbc_combo->insertItem (4, "ROM ONLY");
  mbc_combo->insertItem (5, "MBC5");
  mbc_combo->insertItem (6, "RUMBLE");

  flash_label = new QLabel ("FLASH:", this);
  labels->addWidget (flash_label);
  flash_combo = new QComboBox (this);
  combo_boxes->addWidget (flash_combo);
  flash_combo->insertItem (0, "32 KB");
  flash_combo->insertItem (1, "64 KB");
  flash_combo->insertItem (2, "128 KB");
  flash_combo->insertItem (3, "256 KB");
  flash_combo->insertItem (4, "512 KB");
  flash_combo->insertItem (5, "1024 KB");
  flash_combo->insertItem (6, "2048 KB");
  flash_combo->insertItem (7, "4096 KB");
  flash_combo->insertItem (8, "8192 KB");
  flash_combo->setCurrentIndex (4);

  ram_label = new QLabel ("RAM:", this);
  labels->addWidget (ram_label);
  ram_combo = new QComboBox (this);
  combo_boxes->addWidget (ram_combo);
  ram_combo->insertItem (0, "2 KB");
  ram_combo->insertItem (1, "8 KB");
  ram_combo->insertItem (2, "32 KB");
  ram_combo->insertItem (3, "64 KB");
  ram_combo->insertItem (4, "128 KB");
  ram_combo->setCurrentIndex (1);

  auto_check = new QCheckBox (tr ("Check file size"), this);
  auto_check->setCheckState (Qt::Checked);
  down->addWidget (auto_check);


  north->addLayout (labels);
  north->addLayout (combo_boxes);
  all->addLayout (north);
  all->addLayout (down);
  setLayout (all);

  auto_size = 1;
  mbc = MBCAUTO;
  ram_size = 8;
  flash_size = 512;


  connect (mbc_combo, SIGNAL (activated (int)), this, SLOT (setMbc (int)));
  connect (flash_combo, SIGNAL (activated (int)), this,
       SLOT (setFlash (int)));
  connect (ram_combo, SIGNAL (activated (int)), this, SLOT (setRam (int)));
  connect (auto_check, SIGNAL (stateChanged (int)), this,
       SLOT (setAuto (int)));

}

void
Settings::setFlash (int index)
{
  /* size = 32 * 2 ^ index */
  flash_size = 32 * (int) pow (2.0, (double) index);
}

void
Settings::setRam (int index)
{
  switch (index)
    {
    case 0:
      ram_size = 2;
      break;
    case 1:
      ram_size = 8;
      break;
    case 2:
      ram_size = 32;
      break;
    case 3:
      ram_size = 64;
      break;
    case 4:
      ram_size = 128;
      break;
    }

}

void
Settings::setMbc (int mbc_nr)
{
  mbc = (mbc_t) mbc_nr;
  switch (mbc)
    {
    case MBCAUTO:
      ram_types (5);
      flash_types (8);
      break;
    case MBC1:
      ram_types (3);
      flash_types (6);
      break;
    case MBC2:
      ram_types (0);
      flash_types (3);
      break;
    case MBC3:
      ram_types (4);
      flash_types (7);
      break;
    case ROMONLY:
      ram_types (0);
      flash_types (0);
      break;
    case MBC5:
      ram_types (5);
      flash_types (8);
      break;
    case RUMBLE:
      ram_types (3);
      flash_types (8);
      break;
    }
  setFlash (flash_combo->currentIndex ());
  setRam (ram_combo->currentIndex ());
  emit refresh_ram_buttons ();
}

//fill ram_combo depending on cart type
void
Settings::ram_types (int type)
{
  int ile = ram_combo->count ();
  for (int i = ile - 1; i >= 0; i--)
    ram_combo->removeItem (i);
  switch (type)
    {
    case 0:
      break;
    case 3:
      ram_combo->insertItem (0, "2 KB");
      ram_combo->insertItem (1, "8 KB");
      ram_combo->insertItem (2, "32 KB");
      break;
    case 4:
      ram_combo->insertItem (0, "2 KB");
      ram_combo->insertItem (1, "8 KB");
      ram_combo->insertItem (2, "32 KB");
      ram_combo->insertItem (3, "64 KB");
      break;
    case 5:
      ram_combo->insertItem (0, "2 KB");
      ram_combo->insertItem (1, "8 KB");
      ram_combo->insertItem (2, "32 KB");
      ram_combo->insertItem (3, "64 KB");
      ram_combo->insertItem (4, "128 KB");
      break;
    }
}

void
Settings::flash_types (int type)
{
  int ile = flash_combo->count ();
  for (int i = ile - 1; i >= 0; i--)
    flash_combo->removeItem (i);

  for (int i = 0; i <= type; i++)
    {
      int pojemnosc = 32 * (int) pow (2.0, (double) i);
      flash_combo->insertItem (i, QString::number (pojemnosc) + " KB");
    }
}

void
Settings::setAuto (int state)
{
  if (state == Qt::Checked)
    auto_size = 1;
  else
    auto_size = 0;
}
