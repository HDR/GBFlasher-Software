#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QSettings>
#include <QTimer>
#include <QTextCodec>
#include <QtWidgets/QMessageBox>
#include "Gui.h"
#include "const.h"
#include "Settings.h"
#include <stdio.h>
#include <iostream>

void
parse_params (int /*argc*/, char *argv[])
{
  while (*++argv)
    {
/* disable auto com search in windows */
      if (strcmp (*argv, "-commanual") == 0)
	Settings::commanual = true;
/* additional communication params */
      if (strcmp (*argv, "-12bit") == 0)
	Settings::algorythm = ALG12;
      if (strcmp (*argv, "-longer") == 0)
	Settings::dap = LONGER;
      if (strcmp (*argv, "-default") == 0)
	Settings::dap = DEFAULT;
      if (strcmp (*argv, "-datapoll") == 0)
	Settings::dap = DATAPOLL;
      if (strcmp (*argv, "-showbbl") == 0)
	Settings::showbbl = true;
      if (strcmp (*argv, "-lowspeed") == 0)
    Settings::speed = LOW;
      if (strcmp (*argv, "-highspeed") == 0)
    Settings::speed = HIGH;
/*
 * Appearance settings
 */
      if (strcmp (*argv, "-darkmode") == 0)
    Settings::darkmode = true;


/* 
 * communication threads priority is used when communication
 * or gui is freezing
 */
      if ((*argv)[0] == '-' && isdigit ((*argv)[1]) && (*argv)[1] >= '0'
	  && (*argv)[1] <= '7')
	{
	  switch ((*argv)[1] - '0')
	    {
	    case 0:
	      Settings::priority = QThread::IdlePriority;
	      break;
	    case 1:
	      Settings::priority = QThread::LowestPriority;
	      break;
	    case 2:
	      Settings::priority = QThread::LowPriority;
	      break;
	    case 3:
	      Settings::priority = QThread::NormalPriority;
	      break;
	    case 4:
	      Settings::priority = QThread::HighPriority;
	      break;
	    case 5:
	      Settings::priority = QThread::HighestPriority;
	      break;
	    case 6:
	      Settings::priority = QThread::TimeCriticalPriority;
	      break;
	    case 7:
	      Settings::priority = QThread::InheritPriority;
	      break;
	    }

	}
    }
}

int
main (int argc, char *argv[])
{
  parse_params (argc, argv);
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app (argc, argv);

  Gui window;
  window.show ();
#ifdef Q_OS_WIN
  window.startup_info();
#endif
  return app.exec ();
}
