/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#ifdef HAVE_GMIO
#include "dialog_export_options.h"

#include "options.h"
#include "ui_dialog_export_options.h"

#include <gmio_stl/stl_format.h>

#include <QtCore/QByteArray>
#include <QtCore/QSettings>
#include <QtGui/QStandardItemModel>

namespace Mayo {

namespace Internal {

static const char keyExportOptionsLast[] = "ExportOptions/last";

QByteArray toByteArray(const Application::ExportOptions& opts)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << static_cast<uint32_t>(opts.stlFormat)
           << opts.stlAsciiSolidName.c_str()
           << static_cast<uint32_t>(opts.stlAsciiFloat32Format)
           << static_cast<uint32_t>(opts.stlAsciiFloat32Precision);
    return bytes;
}

Application::ExportOptions fromByteArray(const QByteArray& bytes)
{
    Application::ExportOptions opts;
    QDataStream stream(bytes);
    stream >> *reinterpret_cast<uint32_t*>(&opts.stlFormat);

    char* stlaSolidName = nullptr;
    stream >> stlaSolidName;
    opts.stlAsciiSolidName = stlaSolidName;
    delete[] stlaSolidName;

    stream >> *reinterpret_cast<uint32_t*>(&opts.stlAsciiFloat32Format);
    stream >> *reinterpret_cast<uint32_t*>(&opts.stlAsciiFloat32Precision);

    return opts;
}

} // namespace Internal

DialogExportOptions::DialogExportOptions(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogExportOptions)
{
    m_ui->setupUi(this);
    m_ui->comboBox_StlFormat->addItem(
                tr("ASCII"), gmio::STL_Format_Ascii);
    m_ui->comboBox_StlFormat->addItem(
                tr("Binary litte-endian"), gmio::STL_Format_BinaryLittleEndian);
    m_ui->comboBox_StlFormat->addItem(
                tr("Binary big-endian"), gmio::STL_Format_BinaryBigEndian);

    QSettings settings;
    const QByteArray bytesExportOptions =
            settings.value(Internal::keyExportOptionsLast, QByteArray())
            .toByteArray();
    if (!bytesExportOptions.isEmpty()) {
        const Application::ExportOptions opts =
                Internal::fromByteArray(bytesExportOptions);
        m_ui->comboBox_StlFormat->setCurrentIndex(
                    m_ui->comboBox_StlFormat->findData(opts.stlFormat));
        m_ui->lineEdit_StlGmioAsciiSolidName->setText(
                    QString::fromLatin1(
                        QByteArray::fromStdString(opts.stlAsciiSolidName)));
        switch (opts.stlAsciiFloat32Format) {
        case gmio::FloatTextFormat::DecimalLowercase:
        case gmio::FloatTextFormat::DecimalUppercase:
            m_ui->comboBox_StlGmioAsciiFloatFormat->setCurrentIndex(0); break;
        case gmio::FloatTextFormat::ScientificLowercase:
        case gmio::FloatTextFormat::ScientificUppercase:
            m_ui->comboBox_StlGmioAsciiFloatFormat->setCurrentIndex(1); break;
        case gmio::FloatTextFormat::ShortestLowercase:
        case gmio::FloatTextFormat::ShortestUppercase:
            m_ui->comboBox_StlGmioAsciiFloatFormat->setCurrentIndex(2); break;
        }
        const bool isFormatUppercase =
                opts.stlAsciiFloat32Format == gmio::FloatTextFormat::DecimalUppercase
                || opts.stlAsciiFloat32Format == gmio::FloatTextFormat::ScientificUppercase
                || opts.stlAsciiFloat32Format == gmio::FloatTextFormat::ShortestUppercase;
        m_ui->checkBox_StlGmioAsciiFloatFormatUppercase->setChecked(isFormatUppercase);
        m_ui->spinBox_StlGmioAsciiFloatPrecision->setValue(opts.stlAsciiFloat32Precision);
    }
}

DialogExportOptions::~DialogExportOptions()
{
    delete m_ui;
}

Application::PartFormat DialogExportOptions::partFormat() const
{
    return m_partFormat;
}

void DialogExportOptions::setPartFormat(Application::PartFormat format)
{
    m_partFormat = format;
    if (format == Application::PartFormat::Stl) {
        const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
        m_ui->widget_StlGmio->setEnabled(lib == Options::StlIoLibrary::Gmio);
        auto comboBoxStlFormatModel =
                qobject_cast<QStandardItemModel*>(m_ui->comboBox_StlFormat->model());
        QStandardItem* itemBinaryBigEndian = comboBoxStlFormatModel->item(2);
        itemBinaryBigEndian->setEnabled(lib == Options::StlIoLibrary::Gmio);
        if (lib == Options::StlIoLibrary::Gmio) {
            QObject::connect(
                        m_ui->comboBox_StlFormat,
                        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        [=](int index) {
                m_ui->widget_StlGmioAscii->setEnabled(index == 0);
            });
            m_ui->widget_StlGmioAscii->setEnabled(
                        m_ui->comboBox_StlFormat->currentData().toInt()
                        == gmio::STL_Format_Ascii);
        }
        else {
            if (m_ui->comboBox_StlFormat->currentData().toInt()
                    == gmio::STL_Format_BinaryBigEndian)
            {
                const int comboBoxBinLeId =
                        m_ui->comboBox_StlFormat->findData(gmio::STL_Format_BinaryLittleEndian);
                m_ui->comboBox_StlFormat->setCurrentIndex(comboBoxBinLeId);
            }
        }
    }
}

Application::ExportOptions DialogExportOptions::currentExportOptions() const
{
    const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
    Application::ExportOptions options;
    options.stlFormat = static_cast<gmio::STL_Format>(
                m_ui->comboBox_StlFormat->currentData().toInt());
    if (lib == Options::StlIoLibrary::Gmio) {
        options.stlAsciiSolidName =
                m_ui->lineEdit_StlGmioAsciiSolidName->text().toLatin1().toStdString();
        const int comboBoxFloatFormatId =
                m_ui->comboBox_StlGmioAsciiFloatFormat->currentIndex();
        const bool floatFormatUppercase =
                m_ui->checkBox_StlGmioAsciiFloatFormatUppercase->isChecked();
        if (comboBoxFloatFormatId == 0) {
            options.stlAsciiFloat32Format = floatFormatUppercase ?
                        gmio::FloatTextFormat::DecimalUppercase :
                        gmio::FloatTextFormat::DecimalLowercase;
        }
        else if (comboBoxFloatFormatId == 1) {
            options.stlAsciiFloat32Format = floatFormatUppercase ?
                        gmio::FloatTextFormat::ScientificUppercase :
                        gmio::FloatTextFormat::ScientificLowercase;
        }
        else if (comboBoxFloatFormatId == 2) {
            options.stlAsciiFloat32Format = floatFormatUppercase ?
                        gmio::FloatTextFormat::ShortestLowercase :
                        gmio::FloatTextFormat::ShortestUppercase;
        }
        options.stlAsciiFloat32Precision =
                m_ui->spinBox_StlGmioAsciiFloatPrecision->value();
    }
    return options;
}

void DialogExportOptions::accept()
{
    QSettings settings;
    settings.setValue(
                Internal::keyExportOptionsLast,
                Internal::toByteArray(this->currentExportOptions()));
    QDialog::accept();
}

} // namespace Mayo

#endif // HAVE_GMIO
