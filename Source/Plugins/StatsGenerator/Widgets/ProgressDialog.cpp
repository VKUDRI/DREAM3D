#include "ProgressDialog.h"

#include "moc_ProgressDialog.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ProgressDialog::ProgressDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog (parent, f)
{
  setupUi(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ProgressDialog::~ProgressDialog()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ProgressDialog::setLabelText(const QString& text)
{
  m_Label->setText(text);
}
