/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include <TopoDS_Shape.hxx>

namespace Mayo {
namespace IO {

// Reader for OpenCascade BRep file format
class OccBRepReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgressPortion* progress) override;
    bool transfer(DocumentPtr doc, TaskProgressPortion* progress) override;

private:
    TopoDS_Shape m_shape;
    FilePath m_baseFilename;
};

// Writer for OpenCascade BRep file format
class OccBRepWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> appItems, TaskProgressPortion* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgressPortion* progress) override;

private:
    TopoDS_Shape m_shape;
};

} // namespace IO
} // namespace Mayo
