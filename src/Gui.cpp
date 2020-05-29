#include <QtWidgets/QFileDialog>
#include <QPixmap>
#include <QtWidgets/QMessageBox>
#include <QThread>
#include <QtSerialPort/QSerialPortInfo>
#include "Gui.h"
#include "Settings.h"
#include "Logic.h"
#include "About.h"
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <string>
#include "QtWidgets/QApplication"
#include "QTextStream"
#include "QDebug"
#include "QDateTime"
#include "QDesktopServices"
#include "windows.h"
#include <QWinTaskbarProgress>

#ifdef Q_OS_WIN
#include "USBPortWin.h"
#endif

#include "const.h"
#include "icon.xpm"

Gui::Gui (QWidget * parent):QWidget (parent)
{
  QThread::currentThread ()->setPriority (QThread::NormalPriority);
  path = ".";			//current startup dir'
  if (Settings::darkmode == true)
  {
      QFile f(":qdarkstyle/style.qss");
      if (!f.exists())
      {
          printf("Unable to set stylesheet, file not found\n");
      }
      else
      {
          f.open(QFile::ReadOnly | QFile::Text);
          QTextStream ts(&f);
          qApp->setStyleSheet(ts.readAll());
      }
  }

  this->setWindowIcon (QIcon (QPixmap (icon)));
  this->setWindowTitle (tr ("GB Cart Flasher version ") + VER);
  grid = new QGridLayout (this);
  left = new QVBoxLayout ();
  right = new QVBoxLayout ();
  center = new QVBoxLayout ();
  down = new QHBoxLayout ();

  image = new QLabel (this);

  image->setFixedSize (200, 162);
  settings = new Settings (this);
  left->addWidget (settings);
  left->addWidget (image);
  left->addStretch (1);
  grid->addLayout (left, 0, 0);
  console = new Console (this);
  right->addWidget (console);
  progress = new QProgressBar (this);
  winTaskbar = new QWinTaskbarButton(this);
  QWinTaskbarProgress *winProgress = winTaskbar->progress();
  down->addWidget (progress);
  cancel_btn = new QPushButton (tr ("Cancel"), this);
  cancel_btn->setEnabled (false);
  down->addWidget (cancel_btn);
  right->addLayout (down);
  grid->addLayout (right, 0, 2);
  status_btn = new QPushButton (tr ("Cart Info"), this);
  rflash_btn = new QPushButton (tr ("Read FLASH"), this);
  wflash_btn = new QPushButton (tr ("Write FLASH"), this);
  rram_btn = new QPushButton (tr ("Read RAM"), this);
  wram_btn = new QPushButton (tr ("Write RAM"), this);
  eflash_btn = new QPushButton (tr ("Erase FLASH"), this);
  eram_btn = new QPushButton (tr ("Erase RAM"), this);
  about_btn = new QPushButton (tr ("About"), this);
  patch_btn = new QPushButton (tr ("ROM Patcher"),this );
  firmware_btn = new QPushButton (tr ("Update Flasher Firmware"), this);
  keepfiles_check = new QCheckBox (tr ("Keep Downloaded Files"), this);

  center->addWidget (status_btn, Qt::AlignTop);
  center->addWidget (rflash_btn);
  center->addWidget (wflash_btn);
  center->addWidget (rram_btn);
  center->addWidget (wram_btn);
  center->addWidget (eflash_btn);
  center->addWidget (eram_btn);
  center->addSpacing (20);
  center->addWidget (about_btn);
  center->addStretch (1);
  patch_btn->setFixedWidth(75);
  left->addWidget (patch_btn);
  firmware_btn->setFixedWidth(140);
  left->addWidget (firmware_btn);
  left->addWidget (keepfiles_check);
  grid->addLayout (center, 0, 1);
  keepfiles_check->setCheckState (Qt::Checked);
  winProgress->setVisible(true);

  thread_WFLA = new WriteFlashThread;
  thread_RFLA = new ReadFlashThread;
  thread_E = new EraseThread;
  thread_RRAM = new ReadRamThread;
  thread_WRAM = new WriteRamThread;
  int func_wr = rand() % 100 + 1;
  if (func_wr == 23){winTaskbar->setWindow(this->windowHandle());winTaskbar->progress()->setVisible(true);winTaskbar->setOverlayIcon(QIcon(":/qss_icons/rc/genericarrow.png"));}


  connect (cancel_btn, SIGNAL (clicked ()), thread_RFLA, SLOT (canceled ()));
  connect (cancel_btn, SIGNAL (clicked ()), thread_WFLA, SLOT (canceled ()));
  connect (cancel_btn, SIGNAL (clicked ()), thread_RRAM, SLOT (canceled ()));
  connect (cancel_btn, SIGNAL (clicked ()), thread_WRAM, SLOT (canceled ()));

  connect (cancel_btn, SIGNAL (clicked ()), thread_E, SLOT (canceled ()));
  connect (wflash_btn, SIGNAL (clicked ()), this, SLOT (write_flash ()));
  connect (rflash_btn, SIGNAL (clicked ()), this, SLOT (read_flash ()));
  connect (status_btn, SIGNAL (clicked ()), this, SLOT (show_info ()));
  connect (eflash_btn, SIGNAL (clicked ()), this, SLOT (erase_flash ()));
  connect (rram_btn, SIGNAL (clicked ()), this, SLOT (read_ram ()));
  connect (wram_btn, SIGNAL (clicked ()), this, SLOT (write_ram ()));
  connect (eram_btn, SIGNAL (clicked ()), this, SLOT (erase_ram ()));
  connect (about_btn, SIGNAL (clicked ()), this, SLOT (about ()));
  connect (firmware_btn, SIGNAL (clicked ()), this, SLOT (firmware ()));
  connect (patch_btn, SIGNAL (clicked ()), this, SLOT (patcher ()));

  connect (thread_WFLA, SIGNAL (set_progress (int, int)), this,
       SLOT (setProgress (int, int)));
  connect (thread_RFLA, SIGNAL (set_progress (int, int)), this,
       SLOT (setProgress (int, int)));
  connect (thread_E, SIGNAL (set_progress (int, int)), this,
       SLOT (setProgress (int, int)));
  connect (thread_RRAM, SIGNAL (set_progress (int, int)), this,
       SLOT (setProgress (int, int)));
  connect (thread_WRAM, SIGNAL (set_progress (int, int)), this,
       SLOT (setProgress (int, int)));


  connect (thread_RFLA, SIGNAL (error (int)), this, SLOT (print_error (int)));
  connect (thread_WFLA, SIGNAL (error (int)), this, SLOT (print_error (int)));
  connect (thread_RRAM, SIGNAL (error (int)), this, SLOT (print_error (int)));
  connect (thread_WRAM, SIGNAL (error (int)), this, SLOT (print_error (int)));
  connect (thread_E, SIGNAL (error (int)), this, SLOT (print_error (int)));

  connect (settings, SIGNAL (refresh_ram_buttons (void)), this,
	   SLOT (setRamButtons (void)));
  setProgress (0, 1);
  console->setTextColor(Qt::white);

  console->print (tr ("GB Cart Flasher version ") + VER + tr (" started."));
  checkVersion();
}

AbstractPort *
Gui::create_port (void)
{
    return new USBPortWin;
}

void
Gui::startup_info (void)
{
  status_t status;

  if (Settings::commanual == false)
    {
      AbstractPort *port = create_port ();
      if (Logic::read_status (port, "USB", NREAD_ID, 0x00, 0x00, &status) == true)
	{
	  QString tmp;
      console->print (tr ("Device connected via: USB"));
      tmp = tmp.sprintf (" %d%d.%d%d", status.ver_11, status.ver_12, status.ver_21, status.ver_22);
	  console->print (tr ("Device firmware version:") + tmp);
	  console->line ();
	  return;
	}
    }
  console->line ();

}

void
Gui::show_info ()
{
  status_t status;
  QString tmp;
  AbstractPort *port = create_port ();
  int return_code = Logic::read_status (port, settings->getCom().toLatin1(), READ_ID,
					settings->getMbc (),
					Settings::algorythm, &status);

  if (return_code == true)	/* no error */
    {

      console->print (tr ("--Device information--"));
      tmp =
	tmp.sprintf (" %d%d.%d%d", status.ver_11, status.ver_12,
		     status.ver_21, status.ver_22);
      console->print (tr ("Device firmware version:") + tmp);
      console->print ("\n" + tr ("--Cartridge information--"));
      tmp = tmp.sprintf (" 0x%x", status.manufacturer_id);
      console->print (tr ("FLASH memory manufacturer ID:") + tmp);
      tmp = tmp.sprintf (" %s", status.manufacturer);
      console->print (tr ("FLASH memory manufacturer name:") + tmp);
      tmp = tmp.sprintf (" 0x%x", status.chip_id);
      console->print (tr ("FLASH memory chip ID:") + tmp);

      if (Settings::showbbl == true)
	{
	  if (status.bbl == 1)
	    tmp = tr ("Locked!");
	  else
	    tmp = tr ("Unlocked");
	  console->print (tr ("Boot Block Status: ") + tmp);
	}

      if (status.logo_correct == 1)
	{
	  console->print ("\n" + tr ("--ROM/FLASH content information--"));
	  console->print (tr ("Game logo signature is correct."));
	  tmp = tmp.sprintf (" %s", status.game_name);
	  console->print (tr ("Game title:") + tmp);
	  if (status.cgb == 1)
	    tmp = tr ("YES");
	  else
	    tmp = tr ("NO");
	  console->print (tr ("Designed for Color GB: ") + tmp);
	  if (status.sgb == 1)
	    tmp = tr ("YES");
	  else
	    tmp = tr ("NO");
	  console->print (tr ("Designed for Super GB: ") + tmp);
	  tmp = tmp.sprintf (" %s", status.typ);
	  console->print (tr ("Cartridge type:") + tmp);
	  tmp = tmp.sprintf (" %s", status.rom_size);
	  console->print (tr ("ROM size:") + tmp);
	  tmp = tmp.sprintf (" %s", status.ram_size);
	  console->print (tr ("RAM size:") + tmp);
	  tmp = tmp.sprintf (" 0x%x", status.crc16);
	  console->print (tr ("Checksum:") + tmp);
	  console->line ();
	}
      else
	{
	  console->print (tr ("Game logo signature is incorrect."));
	  console->
	    print (tr ("Cartridge is blank, damaged or not connected."));
	  console->line ();
	}
    }
  else
    print_error (return_code);

}


void
Gui::read_flash (void)
{
  file_name =
    QFileDialog::getSaveFileName (this, tr ("Write FLASH to..."), path,
				  tr ("GB Rom Dumps (*.gb *.gbc *.sgb)"));
  path = Logic::get_path (file_name);
  if (file_name != "")
    {
      thread_RFLA->port = create_port ();
      if (thread_RFLA->port->open_port (settings->getCom().toLatin1()) == false)
	{
	  print_error (PORT_ERROR);
	  return;
	}
      if (!file_name.contains (".gb", Qt::CaseInsensitive)
	  && !file_name.contains (".gbc", Qt::CaseInsensitive)
	  && !file_name.contains (".sgb", Qt::CaseInsensitive))
	file_name = file_name + ".gb";

      thread_RFLA->file = fopen (file_name.toLatin1 (), "wb");
      thread_RFLA->mbc = settings->getMbc ();
      thread_RFLA->page_count = settings->getFlash () / 16;
      thread_RFLA->dap = Settings::dap;
      thread_RFLA->algorythm = Settings::algorythm;

      setEnabledButtons (false);
      thread_RFLA->start (Settings::priority);

      console->print (tr ("Reading data from FLASH to file:") + "\n" +
		      file_name);
    }
}

void
Gui::write_flash (void)
{

  file_name =
    QFileDialog::getOpenFileName (this, tr ("Read FLASH from..."), path,
				  tr ("GB Rom Dumps (*.gb *.gbc *.sgb)"));
  path = Logic::get_path (file_name);
  if (file_name != "")
    {
      long bytesCount;
      short kilobytes_count;
      thread_WFLA->port = create_port ();
      if (thread_WFLA->port->open_port (settings->getCom().toLatin1()) == false)
	{
	  print_error (PORT_ERROR);
	  return;
	}
      thread_WFLA->file = fopen (file_name.toLatin1 (), "rb");
      thread_WFLA->mbc = settings->getMbc ();
      thread_WFLA->algorythm = Settings::algorythm;
      thread_WFLA->dap = Settings::dap;

      if (settings->isAuto () == false)
	{
      bytesCount = Logic::file_size (thread_WFLA->file);
	  thread_WFLA->page_count =
        (short) ((bytesCount % 16384L) ? (bytesCount / 16384 +
                           1) : (bytesCount / 16384L));
	  kilobytes_count =
        (short) ((bytesCount % 1024L) ? (bytesCount / 1024 +
                          1) : (bytesCount / 1024L));

	}
      else if ((kilobytes_count = Logic::flash_file_size (thread_WFLA->file))
	       != false)
	thread_WFLA->page_count = kilobytes_count / 16;
      else
	{
	  print_error (WRONG_SIZE);
	  thread_WFLA->port->close_port ();
	  return;
	}
      setEnabledButtons (false);

      thread_WFLA->start (Settings::priority);
      console->print (tr ("Writing data to FLASH from file:") + "\n" +
		      file_name);
      console->print (tr ("File size: ") + QString::number (kilobytes_count) +
		      "KB");
    }
}


void
Gui::read_ram (void)
{


  file_name =
    QFileDialog::getSaveFileName (this, tr ("Write RAM to..."), path,
				  tr ("GB Save (*.sav)"));
  path = Logic::get_path (file_name);
  if (file_name != "")
    {
      thread_RRAM->port = create_port ();
      if (thread_RRAM->port->open_port (settings->getCom().toLatin1()) == false)
	{
	  print_error (PORT_ERROR);
	  return;
	}
      if (!file_name.contains (".sav", Qt::CaseInsensitive))
	file_name = file_name + ".sav";
      thread_RRAM->file = fopen (file_name.toLatin1 (), "wb");
      thread_RRAM->mbc = settings->getMbc ();
      thread_RRAM->algorythm = Settings::algorythm;
      thread_RRAM->dap = Settings::dap;
      if (settings->getRam () == 2)
	{
	  thread_RRAM->_2k = 1;
	  thread_RRAM->page_count = 1;
	}
      else
	{
	  thread_RRAM->_2k = 0;
	  thread_RRAM->page_count = settings->getRam () / 8;
	}
      setEnabledButtons (false);
      thread_RRAM->start (Settings::priority);
      console->print (tr ("Reading data from RAM to file:") + "\n" +
		      file_name);
    }
}

void
Gui::write_ram (void)
{

  file_name =
    QFileDialog::getOpenFileName (this, tr ("Read RAM from..."), path,
				  tr ("GB Save (*.sav)"));
  path = Logic::get_path (file_name);
  if (file_name != "")
    {
      long bytes_count;
      short kilobytes_count;
      thread_WRAM->port = create_port ();
      if (thread_WRAM->port->open_port (settings->getCom().toLatin1()) == false)
	{
	  print_error (PORT_ERROR);
	  return;
	}
      thread_WRAM->file = fopen (file_name.toLatin1 (), "rb");
      thread_WRAM->mbc = settings->getMbc ();
      thread_WRAM->algorythm = Settings::algorythm;
      thread_WRAM->dap = Settings::dap;

      if (settings->isAuto () == false)
	{
	  bytes_count = Logic::file_size (thread_WRAM->file);
	  if (bytes_count == 2048)
	    {
	      thread_WRAM->_2k = 1;
	      thread_WRAM->page_count = 1;
	      kilobytes_count = 2;
	    }
	  else
	    {
	      thread_WRAM->_2k = 0;
	      thread_WRAM->page_count =
		(short) ((bytes_count % 8192L) ? (bytes_count / 8192L +
						  1) : bytes_count / 8192L);
	      kilobytes_count =
		(short) ((bytes_count % 1024L) ? (bytes_count / 1024 +
						  1) : (bytes_count / 1024L));
	    }
	}
      else if ((kilobytes_count = Logic::ram_file_size (thread_WRAM->file)) !=
	       false)
	if (kilobytes_count == 2)
	  {
	    thread_WRAM->_2k = 1;
	    thread_WRAM->page_count = 1;
	  }
	else
	  {
	    thread_WRAM->_2k = 0;
	    thread_WRAM->page_count = kilobytes_count / 8;
	  }
      else
	{
	  print_error (WRONG_SIZE);
	  thread_WRAM->port->close_port ();
	  return;
	}
      setEnabledButtons (false);

      thread_WRAM->start (Settings::priority);
      console->print (tr ("Writing data to RAM from file:") + "\n" +
		      file_name);
      console->print (tr ("File size: ") + QString::number (kilobytes_count) +
		      "KB");
    }
}

void
Gui::erase_flash (void)
{
  thread_E->port = create_port ();
  if (thread_E->port->open_port (settings->getCom().toLatin1()) == false)
    {
      print_error (PORT_ERROR);
      return;
    }
  thread_E->mbc = settings->getMbc ();
  thread_E->mem = EFLA;		//FLASH
  thread_E->par = Settings::algorythm;
  thread_E->dap = Settings::dap;
  setEnabledButtons (false);
  console->print (tr ("Erasing FLASH memory..."));
  thread_E->start (Settings::priority);
}

void
Gui::erase_ram (void)
{
  thread_E->port = create_port ();
  if (thread_E->port->open_port (settings->getCom().toLatin1()) == false)
    {
      print_error (PORT_ERROR);
      return;
    }
  int windowCount;
  thread_E->mbc = settings->getMbc ();
  thread_E->mem = ERAM;
  switch (settings->getRam ())
    {
    case 2:
    case 8:
      windowCount = 0;
      break;
    case 32:
      windowCount = 3;
      break;
    case 64:
      windowCount = 8;
      break;
    case 128:
      windowCount = 15;
      break;
    default:
      windowCount = 15;
    }

  thread_E->par = windowCount;
  thread_E->dap = Settings::dap;
  setEnabledButtons (false);
  console->print (tr ("Erasing RAM memory..."));
  thread_E->start (Settings::priority);
}



void
Gui::setProgress (int ile, int max)
{
  progress->setMinimum (0);
  progress->setMaximum (max);
  progress->setValue (ile);
  winTaskbar->setWindow(this->windowHandle());
  winTaskbar->progress()->setVisible(true);
  winTaskbar->progress()->setMinimum (0);
  winTaskbar->progress()->setMaximum (max);
  winTaskbar->progress()->setValue (ile);
}


void
Gui::setEnabledButtons (bool state)
{
  status_btn->setEnabled (state);
  rflash_btn->setEnabled (state);
  wflash_btn->setEnabled (state);
  eflash_btn->setEnabled (state);
  cancel_btn->setEnabled (!state);
  if (settings->isRamDisabled ())
    state = false;
  rram_btn->setEnabled (state);
  wram_btn->setEnabled (state);
  eram_btn->setEnabled (state);

}

void
Gui::setRamButtons ()
{
  if (status_btn->isEnabled ())
    setEnabledButtons (true);
}

void
Gui::print_error (int err)
{
  switch (err)
    {
    case FILEERROR_O:
      console->print (tr (">Error opening file."));
      break;

    case FILEERROR_W:
      console->print (tr (">File write error."));
      break;

    case FILEERROR_R:
      console->print (tr (">File read error."));
      break;

    case SEND_ERROR:
      console->print (tr (">Error sending data to device."));
      break;

    case TIMEOUT:
      console->print (tr (">Timeout!"));
      break;

    case END:
      console->print (tr (">Canceled."));
      break;

    case PORT_ERROR:
      console->print (tr (">No Cart Flasher Connected."));
      break;

    case WRONG_SIZE:
      console->print (tr (">Bad file size."));
      break;

    case false:
      console->print (tr (">Operation failure."));
      break;

    case true:
      console->print (tr (">Success!"));	/* succes is not a error code */
      break;
    }

  console->line ();
  setProgress (0, 1);
  setEnabledButtons (true);
}

void
Gui::about ()
{
  About about (this);
  about.exec ();
}

void Gui::checkVersion ()
{
    verMan = new QNetworkAccessManager(this);
    connect(verMan,SIGNAL(finished(QNetworkReply*)),this,SLOT(VSHandler(QNetworkReply*)));
    QNetworkRequest verReq(QUrl("https://github.com/HDR/GBFlasher-Firmware/releases/latest/"));
    verReq.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    verMan->get(verReq);
}

void Gui::VSHandler (QNetworkReply *reply)
{
    status_t status;
    AbstractPort *port = create_port ();
    QString tmp;
    if (Logic::read_status (port, "USB", NREAD_ID, 0x00, 0x00, &status) == true){
        if (reply->url().toString().right(4) > tmp.sprintf("%d.%d%d", status.ver_12, status.ver_21, status.ver_22)){
            console->print("A firmware update is available!");
        }
    }
}

void Gui::firmware ()
{
    if(QSerialPortInfo::availablePorts().empty()){
        console->print("> Error: Could not detect any connected devices");
    } else {
        QMessageBox firmwarebox(QMessageBox::Question, tr("Firmware Update"), tr("Please reconnect your flasher and click continue"), QMessageBox::Yes | QMessageBox::No, nullptr);
        firmwarebox.setButtonText(QMessageBox::Yes, tr("Continue"));
        firmwarebox.setButtonText(QMessageBox::No, tr("Cancel"));
        QString directory = QDir::currentPath() + "//gbflasher";
        QFile firm_file(directory + "\\GBFlasher-Firmware.hex");
        QFile tsb_file(directory + "\\tsbloader_adv.exe");
        if (firmwarebox.exec() == QMessageBox::Yes){
            if (firm_file.exists() && keepfiles_check->isChecked()){
                if (tsb_file.exists() && keepfiles_check->isChecked()){
                    flashFirmware();
                } else {
                    manager2 = new QNetworkAccessManager(this);
                    connect(manager2,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloadTSB(QNetworkReply*)));
                    QNetworkRequest request(QUrl("https://github.com/HDR/GBFlasher-Firmware/raw/master/tools/tsbloader_adv.exe"));
                    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
                    manager2->get(request);
                }
            } else {
                manager = new QNetworkAccessManager(this);
                connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloadFirmware(QNetworkReply*)));
                QNetworkRequest request(QUrl("https://github.com/HDR/GBFlasher-Firmware/releases/latest/download/GBFlasher-Firmware.hex"));
                request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
                manager->get(request);
            }
        }
    }
}

void Gui::downloadFirmware(QNetworkReply *reply)
{
    connect(&updateProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    if(reply->error()){
        console->print(reply->errorString());
        console->print("OpenSSL is missing, Opening link to download in browser");
        QDesktopServices::openUrl(QUrl("https://github.com/HDR/GBFlasher-Software/releases/download/1.5.1/Win64OpenSSL-1_1_1d.exe"));
    }
    else{
        QString directory = QDir::currentPath() + "//gbflasher";
        QDir dir = directory;
        if (!dir.exists()) { dir.mkpath(".");}
        QFile file(directory + "\\GBFlasher-Firmware.hex");
        if(file.open(QFile::WriteOnly)) {
            file.write(reply->readAll());
            file.flush();
            file.close();
            console->print(">Successfully Downloaded Firmware!");
            manager2 = new QNetworkAccessManager(this);
            connect(manager2,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloadTSB(QNetworkReply*)));
            QNetworkRequest request(QUrl("https://github.com/HDR/GBFlasher-Firmware/raw/master/tools/tsbloader_adv.exe"));
            request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
            manager2->get(request);
        }
    }
}

void Gui::downloadTSB(QNetworkReply *reply)
{
    if(reply->error()){
        console->print(reply->errorString());
        console->print("OpenSSL is missing, Opening link to download in browser");
        QDesktopServices::openUrl(QUrl("https://slproweb.com/download/Win64OpenSSL-1_1_1d.exe"));
    }
    else{
        QString directory = QDir::currentPath() + "//gbflasher";
        QDir dir = directory;
        if (!dir.exists()) { dir.mkpath(".");}
        QFile file(directory + "\\tsbloader_adv.exe");
        if(file.open(QFile::WriteOnly)) {
            file.write(reply->readAll());
            file.flush();
            file.close();
            console->print(">Successfully Downloaded TSBLoader!");
            flashFirmware();
        }
    }
}

void Gui::flashFirmware(){
    connect(&updateProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    if (keepfiles_check->checkState() == Qt::Unchecked){connect(&patchProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(removeTempdir()));}
    QString directory = QDir::currentPath() + "//gbflasher";
    QString exec = directory + "\\tsbloader_adv.exe";
    QStringList params; params << "-port=" + QSerialPortInfo::availablePorts().first().portName() << "-fop=wv" << "-ffile=" + directory + "\\GBFlasher-Firmware.hex";
    updateProcess.start(exec, params);
    updateProcess.terminate();

}

void Gui::patcher () {
    QString directory = QDir::currentPath() + "//gbflasher";
    QFile file(directory + "\\flips.exe");
    if (file.exists() && keepfiles_check->isChecked()){
        applyPatch();
    } else {
        manager3 = new QNetworkAccessManager(this);
        connect(manager3,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloadFlips(QNetworkReply*)));
        QNetworkRequest request(QUrl("https://github.com/HDR/GBFlasher-Firmware/raw/master/tools/flips.exe"));
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        manager3->get(request);
    }
}

void Gui::downloadFlips(QNetworkReply *reply)
{
    if(reply->error()){
        console->print(reply->errorString());
        console->print("OpenSSL is missing, Opening link to download in browser");
        QDesktopServices::openUrl(QUrl("https://slproweb.com/download/Win64OpenSSL-1_1_1d.exe"));
    }
    else{
        QString directory = QDir::currentPath() + "//gbflasher";
        QDir dir = directory;
        if (!dir.exists()) { dir.mkpath(".");}
        QFile file(directory + "\\flips.exe");
        if(file.open(QFile::WriteOnly)) {
            file.write(reply->readAll());
            file.flush();
            file.close();
            applyPatch();
        }
    }
}

void Gui::applyPatch(){
    QString directory = QDir::currentPath() + "//gbflasher";
    QString romfilename = QFileDialog::getOpenFileName(this, tr("Open"), "",tr("ROM Files (*.gb *gbc)"));
    QString patchfilename = QFileDialog::getOpenFileName(this, tr("Open"), "",tr("Patch File (*.ips *.bps)"));
    QString romName = QFileInfo (QFile(romfilename)).baseName();

    if (QString (QFileInfo (QFile (patchfilename)).suffix()) == "ips" || QString (QFileInfo (QFile (patchfilename)).suffix()) == "bps"){
        connect(&patchProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processPatchOut()));
        if (keepfiles_check->checkState() == Qt::Unchecked){connect(&patchProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(removeTempdir()));}
        QString patchedRom = QFileDialog::getSaveFileName(this, tr("Save"), romName + "-Patched",tr("ROM File (*.gb *.gbc)"));
        QStringList params; params << "-a" << patchfilename << romfilename << patchedRom;
        patchProcess.start(directory + "\\flips.exe",params);
    }
}

void Gui::processOutput(){console->print(updateProcess.readAllStandardOutput());}
void Gui::processPatchOut(){console->print(patchProcess.readAllStandardOutput());}
void Gui::removeTempdir(){QDir tmpdir(QDir::currentPath() + "//gbflasher"); tmpdir.removeRecursively();}
