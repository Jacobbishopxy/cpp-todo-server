/**
 * @file:	ISpi.h
 * @author:	Jacob Xie
 * @date:	2024/12/02 13:13:33 Monday
 * @brief:
 **/

#ifndef __ISPI__H__
#define __ISPI__H__

#include "Adt.h"

struct ISpi
{
    virtual const std::vector<Todo> procQueryTodos() const = 0;
    virtual std::optional<Todo> procQueryTodo(uint todoId) const = 0;
    virtual bool procNewTodo(const Todo& todo) = 0;
    virtual bool procModifyTodo(const Todo& todo) = 0;
    virtual bool procDeleteTodo(uint todoId) = 0;
    virtual void procSubscribedMessage(std::string_view message) = 0;
};

template <typename T>
concept IsSpi = std::is_base_of_v<ISpi, T>;

#endif  //!__ISPI__H__
