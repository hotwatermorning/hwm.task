#ifndef HWM_TASK_TASKBASE_HPP
#define HWM_TASK_TASKBASE_HPP

namespace hwm {

namespace detail { namespace ns_task {

//! タスクキューで扱うタスクを表すベースクラス
struct task_base
{
    virtual ~task_base() {}
    virtual void run() = 0;
};

}}  //namespace detail::ns_task

}   //namespace hwm

#endif  //HWM_TASK_TASKBASE_HPP

