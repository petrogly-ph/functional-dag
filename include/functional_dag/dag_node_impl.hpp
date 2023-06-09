/**
 *   ___                 .___               
 *  |_  \              __| _/____     ____  
 *   /   \    ______  / __ |\__  \   / ___\ 
 *  / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 * /_/  \__\         \____ |(____  /\___  / 
 *                        \/     \//_____/   
 *                             
 * If a user decides to implement the functionals themselves, then they can use these interfaces.
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 
#pragma once

#include <functional_dag/dag_interface.hpp>
#include <functional_dag/dag_fanout_impl.hpp>

#include <string>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>


namespace fn_dag {
  using namespace std; 
  using namespace fn_dag;

  extern bool __g_run_single_threaded;
  extern bool __g_filter_off;

  template<typename Out, typename IDType> class dag_fanout_node;

  template <class Type, class IDType>
  class _abstract_internal_dag_node {
  public:
    virtual ~_abstract_internal_dag_node() = default;
    virtual IDType get_id() = 0;
    virtual void runFilter(const Type *_data) = 0;
    virtual void print(string _plus) = 0;
  };

  template<typename In, typename Out, typename IDType>
  class _internal_dag_node : public _abstract_internal_dag_node<In, IDType> {
  protected:
    dag_node<In,Out> *m_node_hook;
    thread m_thread;
    const IDType m_node_id;
  private:
    const In *m_lastObject;
    std::future<Out> m_future;
    std::packaged_task<Out *(const In*)> m_fn;
    
  public:
    dag_fanout_node<Out, IDType> *m_child;
    _internal_dag_node(IDType _node_id, dag_node<In,Out> *_node) :
      m_node_id(_node_id),
      m_node_hook(_node),
      m_child(new dag_fanout_node<Out, IDType>()){}

    ~_internal_dag_node() {
      if(m_thread.joinable())
        m_thread.join();
      delete m_child;
      delete m_node_hook;
    }

    void runFilter(const In *_data) {
      m_lastObject = _data;
      // if(!fn_dag::__g_run_single_threaded) {
      //   std::packaged_task<void()> task(std::bind(&_internal_dag_node<In,Out,IDType>::runOnce, this));
      //   // std::future<int> f1 = task.get_future();  // get a future
      //   std::thread tmp_thread(std::thread(std::move(task)));
      //   m_thread.swap(tmp_thread); // launch on a thread
      //   // tmp_thread.join();
      // } else {
        runOnce();
      // }
    }

    void runOnce() {
      Out *data_out = m_node_hook->update(m_lastObject);
      if(!__g_filter_off && data_out != NULL) {
        m_child->spread_around(data_out);
        m_lastObject = nullptr;
      }
    }

    void print(string _indent_str) {
      std::cout << m_node_id << std::endl;
      m_child->print(_indent_str);
    }

    IDType get_id() {
      return m_node_id;
    }
  };
}
