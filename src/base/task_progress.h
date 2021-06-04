/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task_common.h"
#include <QtCore/QString>
#include <atomic>

namespace Mayo {

class Task;
class TaskManager;

class TaskProgress {
public:
    TaskProgress();
    TaskProgress(TaskProgress* parent, double portionSize, const QString& step = {});
    ~TaskProgress();

    TaskId taskId() const;
    TaskManager* taskManager() const;

    // Value in [0,100]
    int value() const { return m_value; }
    void setValue(int pct);

    const QString& step() const { return m_step; }
    void setStep(const QString& title);

    bool isRoot() const { return m_parent != nullptr; }
    const TaskProgress* parent() const { return m_parent; }
    TaskProgress* parent() { return m_parent; }

    bool isAbortRequested() const { return m_isAbortRequested; }
    static bool isAbortRequested(const TaskProgress* progress);

    // Disable copy
    TaskProgress(const TaskProgress&) = delete;
    TaskProgress(TaskProgress&&) = delete;
    TaskProgress& operator=(const TaskProgress&) = delete;
    TaskProgress& operator=(TaskProgress&&) = delete;

private:
    void setTask(const Task* task);
    void requestAbort();

    friend class TaskManager;

    TaskProgress* m_parent = nullptr;
    const Task* m_task = nullptr;
    double m_portionSize = -1;
    std::atomic<int> m_value = 0;
    QString m_step;
    bool m_isAbortRequested = false;
};

} // namespace Mayo
