/*!
 * \file WTSObject.hpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief Wt����Object����
 */
#pragma once
#include <stdint.h>
#include <atomic>
#include <boost/smart_ptr/detail/spinlock.hpp>

#include "WTSMarcos.h"
#include "../Share/ObjectPool.hpp"
#include "../Share/SpinMutex.hpp"

NS_WTP_BEGIN
class WTSObject {
public:
    WTSObject() :m_uRefs(1) {}
    virtual ~WTSObject() {}
    //////////////////////////////////////////////////////////////////////////
    inline uint32_t	retain() { return m_uRefs.fetch_add(1) + 1; }

    virtual void release()
    {
        if (!m_uRefs) return;
     
        try {
            uint32_t cnt = m_uRefs.fetch_sub(1);
            if (cnt == 1) delete this;
        }
        catch(...) {
            // nothing
        }
    }

    inline bool	isSingleRefs() { return m_uRefs == 1; }
    inline uint32_t	retainCount() { return m_uRefs; }

protected:
    volatile std::atomic<uint32_t> m_uRefs;
};

template<typename T>
class WTSPoolObject : public WTSObject {
public:
    WTSPoolObject() :_pool(NULL) {}
    virtual ~WTSPoolObject() {}

private:
    typedef ObjectPool<T> MyPool;

public:
    static T* allocate()
    {   /*
         *	By Wesley @ 2022.06.14
         *	���û�����������ʹ����thread_local���߳����ٵĻ����ڴ��Ҳ������
         *	���û���Trader�︴�������bug�����Trader�ײ�������һ��API����ʵ��
         *	��ô�����ڴ�ؾ��Ѿ������ˣ��������ϵͳ�д洢��retain��Trader�����Ķ���WTSOrderInfo�ȣ���
         *	�����ַ���Խ�������
         *
         *	�������ȥ��thread_local���ĳɴ���̬�ģ����ܶ��̲߳����ĳ�����Ҳ����һЩ����
         *	��֮���Ҫ���װ�ȫ����ô������Ҫ��һ�������У�����������������ܿ���
         *	����ע��һ�£����������Ŀ��Բο�һ��
         */
        thread_local static MyPool		pool;    // only the creator thread allows to allocate
        thread_local static SpinMutex	mtx;
     
        mtx.lock();
        T* ret = pool.construct();
        mtx.unlock();
        ret->_pool = &pool;
        ret->_mutex = &mtx;
        return ret;
    }

    virtual void release() override
    {
        if (!m_uRefs) return;
     
        try {
            uint32_t cnt = m_uRefs.fetch_sub(1);
            if (cnt == 1) {
                _mutex->lock();     // lock to avoid allocation
                _pool->destroy((T*) this);
                _mutex->unlock();
            }
        }
        catch (...) {
            // nothing
        }
    }

private:
    MyPool*	_pool;      // you have to do this for future destroying & recycling
    SpinMutex* _mutex;
};
NS_WTP_END
