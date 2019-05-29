/*****************************************************************************
** About.cpp - Source file of About DialogBox containing info about program
** Author: Kraku
*****************************************************************************/
#include "About.h"
#include "about.xpm"
#include "const.h"

About::About (QWidget * parent):QDialog (parent)
{

  all = new QVBoxLayout (this);

  this->setWindowTitle (tr ("About"));

  QPixmap
  logo (about_xpm);
  image = new QLabel (this);
  image->setFixedSize (501, 218);
  image->setPixmap (logo);

  all->addWidget (image);
  box = new QGroupBox (tr ("GB Cart Flasher Project"), this);
  box->setFixedWidth (501);
  labels = new QVBoxLayout (box);
  name =
    new QLabel (tr ("GB Cart Flasher for Win8, Win8.1 & Win10 version ") +
                VER + " (64-bit)", box);
  labels->addWidget (name);
  copy = new QLabel ("Copyright (c) 2005-2007 Kraku & Chroost", box);
  labels->addWidget (copy);
  desc1 = new QLabel (tr ("This is a custom version"),
		      box);
  labels->addWidget (desc1);
  all->addWidget (box);
  ok_btn = new QPushButton (tr ("Close"), this);
  all->addWidget (ok_btn);
  connect (ok_btn, SIGNAL (clicked ()), this, SLOT (close ()));
}