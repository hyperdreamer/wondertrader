/*!
 * \file WTSCollection.hpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief Wt������������ļ�
 */
#pragma once
#include "WTSObject.hpp"
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include "FasterDefs.h"

#include <deque>

NS_WTP_BEGIN

//////////////////////////////////////////////////////////////////////////
//WTSArray

/*
 *	ƽ̨��������
 *	�ڲ�ʹ��vectorʵ��
 *	����ʹ��WTSObjectָ�����
 *	����WTSObject�������඼����ʹ��
 *	����ƽ̨��ʹ��
 */
class WTSArray : public WTSObject {
protected:
    WTSArray() :_holding(false) {}
    virtual ~WTSArray() {}

public:
    /*
     *	���������
     */
    typedef std::vector<WTSObject*>::iterator Iterator;
    typedef std::vector<WTSObject*>::const_iterator ConstIterator;
    /***************************************************************/
    typedef std::vector<WTSObject*>::reverse_iterator ReverseIterator;
    typedef std::vector<WTSObject*>::const_reverse_iterator ConstReverseIterator;
    /***************************************************************/
    typedef std::function<bool(WTSObject*, WTSObject*)>	SortFunc;

    /*
     *	�����������
     */
    static WTSArray* create()
    {
        WTSArray* pRet = new WTSArray();
        return pRet;
    }

    /*
     *	��ȡ���鳤��
     */
    uint32_t size() const{ return (uint32_t) _vec.size(); }

    /*
     *	�������,�����·���ռ�
     *	���øú�����Ԥ�ȷ��䳤��
     *	Ԥ�ȷ���õ����ݶ���NULL
     */
    void resize(uint32_t _size)
    {
        if(!_vec.empty()) clear();
        _vec.resize(_size, NULL);
    }

    /*
     *	��ȡ����ָ��λ�õ�����
     *	�Ա�grab�ӿ�,at�ӿ�ֻȡ������
     *	���������ݵ����ü���
     *	grab�ӿڶ�ȡ�����Ժ�,�������ü���
     */
    WTSObject* at(uint32_t idx)
    {
        if(idx < 0 || idx >= _vec.size()) return NULL;
     
        WTSObject* pRet = _vec.at(idx);
        return pRet;
    }

    uint32_t idxOf(WTSObject* obj)
    {
        if (!obj) return -1;
     
        uint32_t idx = 0;
        for (auto it = _vec.begin(); it != _vec.end(); ++it, ++idx)
            if (obj == *it) return idx;
        return -1;  // not found
    }

    template<typename T> 
    T* at(uint32_t idx)
    {
        if(idx < 0 || idx >= _vec.size()) return NULL;
     
        WTSObject* pRet = _vec.at(idx);
        return static_cast<T*>(pRet);
    }

    /*
     *	[]����������
     *	�÷�ͬat����
     */
    WTSObject* operator [](uint32_t idx)
    {
        if(idx < 0 || idx >= _vec.size()) return NULL;
     
        WTSObject* pRet = _vec.at(idx);
        return pRet;
    }

    /*
     *	��ȡ����ָ��λ�õ�����
     *	�������ü���
     */
    WTSObject* grab(uint32_t idx)
    {
        if(idx < 0 || idx >= _vec.size()) return NULL;
     
        WTSObject* pRet = _vec.at(idx);
        if (pRet) pRet->retain();
     
        return pRet;
    }

    /*
     *	����ĩβ׷������
     *	�����Զ��������ü���
     */
    void append(WTSObject* obj, bool bAutoRetain = true)
    {
        if (bAutoRetain && obj) obj->retain();
     
        _vec.emplace_back(obj);
    }

    /*
     *	����ָ��λ�õ�����
     *	�����λ����������,���ͷŵ����������ü�������
     */
    void set(uint32_t idx, WTSObject* obj, bool bAutoRetain = true)
    {
        if(idx >= _vec.size() || !obj) return;
     
        if(bAutoRetain) obj->retain();
     
        WTSObject* oldObj = _vec.at(idx);
        if(oldObj) oldObj->release();
      
        _vec[idx] = obj;
    }

    void append(WTSArray* ay)
    {
        if(!ay) return;
     
        _vec.insert(_vec.end(), ay->_vec.begin(), ay->_vec.end());
        ay->_vec.clear();
    }

    /*
     *	�������
     *	���������������ͷ�����
     */
    void clear()
    {
        std::vector<WTSObject*>::iterator it;
        for (it = _vec.begin(); it != _vec.end(); ++it) {
            WTSObject* obj = *it;
            if (obj) obj->release();
        }
     
        _vec.clear();
    }

    /*
     *	�ͷ��������,�÷���WTSObject
     *	��ͬ����,������ü���Ϊ1ʱ�ͷ���������
     */
    virtual void release()
    {
        if (!m_uRefs) return;
     
        try {
            uint32_t cnt = m_uRefs.fetch_sub(1);
            if (cnt == 1) {
                clear();
                delete this;
            }
        }
        catch(...) {
            // nothing
        }
    }

    /*
     *	ȡ�����������ʼλ�õĵ�����
     */
    Iterator begin()
    {
        return _vec.begin();
    }

    ConstIterator begin() const
    {
        return _vec.begin();
    }

    ReverseIterator rbegin()
    {
        return _vec.rbegin();
    }

    ConstReverseIterator rbegin() const
    {
        return _vec.rbegin();
    }

    /*
     *	ȡ���������ĩβλ�õĵ�����
     */
    Iterator end()
    {
        return _vec.end();
    }

    ConstIterator end() const
    {
        return _vec.end();
    }

    ReverseIterator rend()
    {
        return _vec.rend();
    }

    ConstReverseIterator rend() const
    {
        return _vec.rend();
    }

    void sort(SortFunc func)
    {
        std::sort(_vec.begin(), _vec.end(), func);
    }

protected:
    std::vector<WTSObject*>	_vec;
    std::atomic<bool> _holding;
};


/*
 *	map����
 *	�ڲ�����std:mapʵ��
 *	ģ������Ϊkey����
 *	����ʹ��WTSObjectָ�����
 *	����WTSObject�������඼����
 */
template <class T>
class WTSMap : public WTSObject {
protected:
    WTSMap() {}
    ~WTSMap() {}

protected:
    typedef typename std::map<T, WTSObject*> _MapType;

public:
    /*
     *	�����������Ķ���
     */
    typedef typename _MapType::iterator			        Iterator;
    typedef typename _MapType::const_iterator	        ConstIterator;
    typedef typename _MapType::reverse_iterator		    ReverseIterator;
    typedef typename _MapType::const_reverse_iterator	ConstReverseIterator;

    /*
     *	����map����
     */
    static WTSMap<T>*	create()
    {
        WTSMap<T>* pRet = new WTSMap<T>();
        return pRet;
    }

    /*
     *	����map�����Ĵ�С
     */
    uint32_t size() const{ return (uint32_t)_map.size(); }

    /*
     *	��ȡָ��key��Ӧ������
     *	���������ݵ����ü���
     *	û���򷵻�NULL
     */
    WTSObject* get(const T &_key)
    {
        Iterator it = _map.find(_key);
        if(it == _map.end())
            return NULL;

        WTSObject* pRet = it->second;
        return pRet;
    }

    /*
     *	[]����������
     *	�÷�ͬget����
     */
    WTSObject* operator[](const T &_key)
    {
        Iterator it = _map.find(_key);
        if(it == _map.end())
            return NULL;

        WTSObject* pRet = it->second;
        return pRet;
    }

    /*
     *	��ȡָ��key��Ӧ������
     *	�������ݵ����ü���
     *	û���򷵻�NULL
     */
    WTSObject* grab(const T &_key)
    {
        Iterator it = _map.find(_key);
        if(it == _map.end())
            return NULL;

        WTSObject* pRet = it->second;
        if (pRet)
            pRet->retain();

        return pRet;
    }

    /*
     *	����һ������,�������������ü���
     *	���key����,��ԭ�������ͷ�
     */
    void add(T _key, WTSObject* obj, bool bAutoRetain = true)
    {
        if(bAutoRetain && obj)
            obj->retain();

        WTSObject* pOldObj = NULL;
        Iterator it = _map.find(_key);
        if(it != _map.end())
        {
            pOldObj = it->second;
        }

        _map[_key] = obj;

        if (pOldObj) pOldObj->release();
    }

    /*
     *	����keyɾ��һ������
     *	���key����,���Ӧ�������ü���-1
     */
    void remove(T _key)
    {
        Iterator it = _map.find(_key);
        if(it != _map.end())
        {
            WTSObject* obj = it->second;
            _map.erase(it);
            if (obj) obj->release();
        }
    }

    /*
     *	��ȡ������ʼλ�õĵ�����
     */
    Iterator begin()
    {
        return _map.begin();
    }

    ConstIterator begin() const
    {
        return _map.begin();
    }

    /*
     *	��ȡ����ĩβλ�õĵ�����
     */
    Iterator end()
    {
        return _map.end();
    }

    ConstIterator end() const
    {
        return _map.end();
    }

    /*
     *	��ȡ������ʼλ�õĵ�����
     */
    ReverseIterator rbegin()
    {
        return _map.rbegin();
    }

    ConstReverseIterator rbegin() const
    {
        return _map.rbegin();
    }

    /*
     *	��ȡ����ĩβλ�õĵ�����
     */
    ReverseIterator rend()
    {
        return _map.rend();
    }

    ConstReverseIterator rend() const
    {
        return _map.rend();
    }

    Iterator find(const T& key)
    {
        return _map.find(key);
    }

    ConstIterator find(const T& key) const
    {
        return _map.find(key);
    }

    void erase(ConstIterator it)
    {
        _map.erase(it);
    }

    Iterator lower_bound(const T& key)
    {
        return _map.lower_bound(key);
    }

    ConstIterator lower_bound(const T& key) const
    {
        return _map.lower_bound(key);
    }

    Iterator upper_bound(const T& key)
    {
        return _map.upper_bound(key);
    }

    ConstIterator upper_bound(const T& key) const
    {
        return _map.upper_bound(key);
    }

    WTSObject* last() 
    {
        if(_map.empty())
            return NULL;

        return _map.rbegin()->second;
    }


    /*
     *	�������
     *	�����������������ü���-1
     */
    void clear()
    {
        Iterator it = _map.begin();
        for(; it != _map.end(); it++)
        {
            it->second->release();
        }
        _map.clear();
    }

    /*
     *	�ͷ���������
     *	����������ü���Ϊ1,�������������
     */
    virtual void release()
    {
        if (m_uRefs == 0)
            return;

        try
        {
            m_uRefs--;
            if (m_uRefs == 0)
            {
                clear();
                delete this;
            }
        }
        catch(...)
        {

        }
    }

protected:
    std::map<T, WTSObject*>	_map;
};

/*
 *	map����
 *	�ڲ�����ankerl::unordered_dense::mapʵ��
 *	ģ������Ϊkey����
 *	����ʹ��WTSObjectָ�����
 *	����WTSObject�������඼����
 */
template <typename T, class Hash = std::hash<T>>
class WTSHashMap : public WTSObject {
protected:  
    WTSHashMap() {}     // it cannot be instantiated directly, but by create()
    virtual ~WTSHashMap() {} // it cannot be deleted directly, but by release()

protected:
    typedef wt_hashmap<T, WTSObject*, Hash>	_MapType;

public:
    /*
     *	�����������Ķ���
     */
    typedef typename _MapType::const_iterator	ConstIterator;

    /*
     *	����map����
     */
    static WTSHashMap<T, Hash>*	create()
    {
        WTSHashMap<T, Hash>* pRet = new WTSHashMap<T, Hash>();
        return pRet;
    }

    /*
     *	����map�����Ĵ�С
     */
    inline uint32_t size() const 
    {
        return (uint32_t) _map.size(); 
    }

    /*
     *	��ȡָ��key��Ӧ������
     *	���������ݵ����ü���
     *	û���򷵻�NULL
     */
    inline WTSObject* get(const T& _key)
    {
        auto it = _map.find(_key);
        if(it == _map.end()) return NULL;
        // it->first is the key, it->second is the value 
        WTSObject* pRet = it->second;
        return pRet;
    }

    /*
     *	��ȡָ��key��Ӧ������
     *	�������ݵ����ü���
     *	û���򷵻�NULL
     */
    inline WTSObject* grab(const T& _key)
    {
        auto it = _map.find(_key);
        if(it == _map.end()) return NULL;
     
        WTSObject* pRet = it->second;
        pRet->retain();
        return pRet;
    }

    /*
     *	����һ������,�������������ü���
     *	���key����,��ԭ�������ͷ�
     */
    inline void add(const T& _key, WTSObject* obj, bool bAutoRetain = true)
    {
        if (bAutoRetain && obj) obj->retain();
     
        WTSObject* pOldObj = NULL;
        auto it = _map.find(_key);
        if (it != _map.end()) pOldObj = it->second;
        if (pOldObj) pOldObj->release();
     
        _map[_key] = obj;
    }

    /*
     *	����keyɾ��һ������
     *	���key����,���Ӧ�������ü���-1
     */
    inline void remove(const T& _key)
    {
        auto it = _map.find(_key);
        if(it != _map.end()) {
            it->second->release();
            _map.erase(it);
        }
    }

    /*
     *	��ȡ������ʼλ�õĵ�����
     */
    inline ConstIterator begin() const
    {
        return _map.begin();
    }

    /*
     *	��ȡ����ĩβλ�õĵ�����
     */
    inline ConstIterator end() const
    {
        return _map.end();
    }

    inline ConstIterator find(const T& key) const
    {
        return _map.find(key);
    }

    /*
     *	�������
     *	�����������������ü���-1
     */
    inline void clear()
    {
        for (ConstIterator it = _map.begin(); it != _map.end(); ++it)
            it->second->release();
        _map.clear();
    }

    /*
     *	�ͷ���������
     *	����������ü���Ϊ1,�������������
     */
    virtual void release()
    {
        if (!m_uRefs) return;
        
        try {
            uint32_t cnt = m_uRefs.fetch_sub(1);
            if (cnt == 1) {
                clear();
                delete this;
            }
        }
        catch (...) {
            // nothing
        }
    }

protected:
    //std::unordered_map<T, WTSObject*>	_map;
    _MapType _map;
};

//////////////////////////////////////////////////////////////////////////
//WTSQueue
class WTSQueue : public WTSObject
{
public:
	typedef std::deque<WTSObject*>::iterator Iterator;
	typedef std::deque<WTSObject*>::const_iterator ConstIterator;

	static WTSQueue* create()
	{
		WTSQueue* pRet = new WTSQueue();
		return pRet;
	}

	void pop()
	{
		_queue.pop_front();
	}

	void push(WTSObject* obj, bool bAutoRetain = true)
	{
		if (obj && bAutoRetain)
			obj->retain();

		_queue.emplace_back(obj);
	}

	WTSObject* front(bool bRetain = true)
	{
		if(_queue.empty())
			return NULL;

		WTSObject* obj = _queue.front();
		if(bRetain)
			obj->retain();

		return obj;
	}

	WTSObject* back(bool bRetain = true)
	{
		if(_queue.empty())
			return NULL;

		WTSObject* obj = _queue.back();
		if(bRetain)
			obj->retain();

		return obj;
	}

	uint32_t size() const{ return (uint32_t)_queue.size(); }

	bool	empty() const{return _queue.empty();}

	void release()
	{
		if (m_uRefs == 0)
			return;

		try
		{
			m_uRefs--;
			if (m_uRefs == 0)
			{
				clear();
				delete this;
			}
		}
		catch (...)
		{

		}
	}

	void clear()
	{
		Iterator it = begin();
		for(; it != end(); it++)
		{
			(*it)->release();
		}
		_queue.clear();
	}

	/*
	 *	ȡ�����������ʼλ�õĵ�����
	 */
	Iterator begin()
	{
		return _queue.begin();
	}

	ConstIterator begin() const
	{
		return _queue.begin();
	}

	void swap(WTSQueue* right)
	{
		_queue.swap(right->_queue);
	}

	/*
	 *	ȡ���������ĩβλ�õĵ�����
	 */
	Iterator end()
	{
		return _queue.end();
	}

	ConstIterator end() const
	{
		return _queue.end();
	}

protected:
	WTSQueue(){}
	virtual ~WTSQueue(){}

	std::deque<WTSObject*>	_queue;
};

NS_WTP_END
