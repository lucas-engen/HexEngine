#include "config_form.h"
#include "ui_config_form.h"
#include <QTime>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>
#include <QByteArray>
#include <QException>

#ifdef QT_DEBUG
#include <QDebug>
#endif

config_form::config_form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::config_form)
{
    ui->setupUi(this);
    this->connect_all_events();
    this->setWindowTitle("Settings");
    this->setWindowIcon(QIcon(":/files/icon2.ico"));

    ui->groupBox->setTitle(QString("Número máximo de dígitos (máx. %1)").arg(ui->dial_ndigits->maximum()));
}

config_form::~config_form()
{
    delete ui;
}

void config_form::handle_dial_value_changed(int value)
{
    QString text;
    qint32 seed;
    QTime tm;
    qint32 random;

    tm = QTime::currentTime();
    seed = tm.hour() + tm.second() + tm.minute() + tm.msec();
    qsrand(static_cast<uint>(seed));
    random = qrand() % 255 + 1;

    try
    {
        text = QString("Hex: %1 Dec: %2").arg(random, ui->dial_ndigits->value(), 16, QLatin1Char('0')).arg(random, 0, 10);
#ifdef QT_DEBUG
        qDebug() << text;
#endif
    } catch(QException &e)
    {
        QMessageBox::critical(this,"Fatal error", QString(e.what()));
        return;
    }

    ui->lcd_display_ndigits->display(value);
    ui->label_ndigits_sample->setText(text);
}

void config_form::handle_window_destroyed(QObject *o)
{
    QString title = "Alert";
    QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
    QString text = "Do you want to save your settings";
    Q_UNUSED(o);

    if(QMessageBox::warning(this, title, text, buttons) == QMessageBox::Yes)
        handle_btn_save_settings_clicked(true);
}

void config_form::handle_btn_save_settings_clicked(bool b)
{
    QFile *ini = nullptr;
    QDateTime tm;

    Q_UNUSED(b);

    ini = new QFile(CONFIG_FILENAME);

    if(ini == nullptr)
    {
        QMessageBox::critical(this, "Error", "Failed to save settings");
        return;
    }

    if(ini->open(QIODevice::WriteOnly) == false)
    {
        QMessageBox::critical(this, "Error", "Failed to save settings [open]");
        return;
    }

    /* Write header */
    tm = QDateTime::currentDateTime().toLocalTime();
    ini->write(QString("# File created by hexengine on %1 at %2\n").arg( \
                   tm.date().toString(),
                   tm.time().toString()).toStdString().c_str());
    ini->write("# Please. Be careful while editing this file directly\n\n");
    ini->write("[settings]\n");

    /* Write settings ... */
    if(ui->checkbox_carray_output->isChecked())
        ini->write(QString("c_array = %1\n").arg(true).toStdString().c_str());
    else
        ini->write(QString("c_array = %1\n").arg(false).toStdString().c_str());

    if(ui->groupBox->isEnabled())
        ini->write(QString("digits = %1").arg(ui->dial_ndigits->value()).toStdString().c_str());
    else
        ini->write("digits = 0");

    ini->close();
    if(ini->size() <= 0)
        QMessageBox::critical(this, "Error", "Settings can't be saved");
    else
    {
        QMessageBox::information(this, "Notice", "Settings saved");
        this->close();
    }
}

void config_form::connect_all_events()
{
    QObject::connect(ui->dial_ndigits, SIGNAL(valueChanged(int)),
                     this, SLOT(handle_dial_value_changed(int)));

    QObject::connect(ui->checkbox_carray_output, SIGNAL(toggled(bool)),
                     ui->groupBox, SLOT(setEnabled(bool)));

    QObject::connect(ui->checkbox_carray_output, SIGNAL(toggled(bool)),
                     ui->dial_ndigits, SLOT(setEnabled(bool)));

    QObject::connect(ui->checkbox_carray_output, SIGNAL(toggled(bool)),
                     ui->label_ndigits_sample, SLOT(setEnabled(bool)));

    QObject::connect(ui->checkbox_carray_output, SIGNAL(toggled(bool)),
                     ui->label, SLOT(setEnabled(bool)));

    QObject::connect(this, SIGNAL(destroyed(QObject*)),
                     this, SLOT(handle_window_destroyed(QObject *)));

    QObject::connect(ui->btn_save_settings, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_save_settings_clicked(bool)));
}
