#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QNetworkAccessManager>
#include "Settings.h"
#include "Console.h"
#include "Logic.h"
#include "About.h"

#include "ReadFlashThread.h"
#include "WriteFlashThread.h"
#include "EraseThread.h"
#include "ReadRamThread.h"
#include "WriteRamThread.h"

class Gui:public QWidget
{
  Q_OBJECT Settings * settings;
  Console *console;
  QGridLayout *grid;
  QVBoxLayout *left;
  QVBoxLayout *right;
  QVBoxLayout *center;
  QHBoxLayout *down;
  QProgressBar *progress;
  QLabel *image;
  QPixmap *logo;
  QPushButton *cancel_btn;
  QPushButton *status_btn;
  QPushButton *rflash_btn;
  QPushButton *wflash_btn;
  QPushButton *rram_btn;
  QPushButton *wram_btn;
  QPushButton *eflash_btn;
  QPushButton *eram_btn;
  QPushButton *about_btn;
  QPushButton *firmware_btn;
  QString file_name;
  QString path;
  About about_dlg;
  WriteFlashThread *thread_WFLA;
  ReadFlashThread *thread_RFLA;
  EraseThread *thread_E;
  ReadRamThread *thread_RRAM;
  WriteRamThread * thread_WRAM;
  QNetworkAccessManager *manager;

public:
    Gui (QWidget * parent = nullptr);
  static int port_type;


  public slots:void startup_info (void);
  void setEnabledButtons (bool stan);
  void setRamButtons ();
  void print_error (int err);
  void write_flash (void);
  void read_flash (void);
  void read_ram (void);
  void write_ram (void);
  void show_info (void);
  void erase_flash (void);
  void erase_ram (void);
  void about ();
  void firmware ();
  void download (QNetworkReply *reply);
  void setProgress (int ile, int max);
  AbstractPort *create_port ();
};
