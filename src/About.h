/*****************************************************************************
** About.h
** Author: Kraku
*****************************************************************************/
#ifndef FL_ABOUT_H_
#define FL_ABOUT_H_
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QPixmap>
#include <QtWidgets/QLabel>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QGroupBox>
class About:public QDialog
{
  Q_OBJECT QPushButton * ok_btn;
  QLabel *image;
  QVBoxLayout *all;
  QGroupBox *box;
  QLabel *name, *copy, *desc1, *desc2, *warning, *translator;
  QVBoxLayout *labels;
public:
    About (QWidget * parent = nullptr);
};

#endif
