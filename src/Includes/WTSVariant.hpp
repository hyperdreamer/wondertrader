/*!
 * \file WTSVariant.hpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief Wtͨ�ñ���������
 */
#pragma once

#include "WTSTypes.h"
#include "WTSObject.hpp"
#include "WTSCollection.hpp"

#include <string>
#include <string.h>
#include <vector>
#include <map>

#ifdef _MSC_VER
#define INT64_FMT	"%I64d"
#define UINT64_FMT	"%I64u"
#else
#define INT64_FMT	"%ld"
#define UINT64_FMT	"%lu"
#endif


NS_WTP_BEGIN

/*
 *	WTSVariant��һ��ͨ����������,���Ŀ����Json��Value��
 *	��Json��ͬ�ĵط�����,WTSVariant����WTϵͳ�ڵ�������ϵ
 *	����ͨ�����ü�����������,�Ӷ��������ݸ���
 */
class WTSVariant : public WTSObject {
public:
    typedef WTSArray					ChildrenArray;
    typedef WTSHashMap<std::string>		ChildrenMap;
    typedef std::vector<std::string>	MemberNames;
    /***************************************************************/
    typedef enum {
        VT_Null,
        VT_Array,
        VT_Int32,
        VT_Uint32,
        VT_Int64,
        VT_Uint64,
        VT_String,
        VT_Real,
        VT_Boolean,
        VT_Object
    } ValueType;

protected:
    WTSVariant() :_type(VT_Null) {}

private:
    static inline WTSVariant* create(int32_t i32)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Int32;
        char s[32] = { 0 };
        sprintf(s, "%d", i32);
        ret->_value._string = new std::string(s);
        return ret;
    }

    static inline WTSVariant* create(uint32_t u32)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Uint32;
        char s[32] = { 0 };
        sprintf(s, "%u", u32);
        ret->_value._string = new std::string(s);
        return ret;
    }

    static inline WTSVariant* create(int64_t i64)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Int64;
        char s[32] = { 0 };
        sprintf(s, INT64_FMT, i64);
        ret->_value._string = new std::string(s);
        return ret;
    }

    static inline WTSVariant* create(uint64_t u64)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Uint64;
        char s[32] = { 0 };
        sprintf(s, UINT64_FMT, u64);
        ret->_value._string = new std::string(s);
        return ret;
    }

    static inline WTSVariant* create(double _real)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Real;
        char s[32] = { 0 };
        sprintf(s, "%.10f", _real);
        ret->_value._string = new std::string(s);
        return ret;
    }

    static inline WTSVariant* create(const char* _string)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_String;
        ret->_value._string = new std::string(_string);
        return ret;
    }

    static inline WTSVariant* create(bool _bool)
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Boolean;
        ret->_value._string = new std::string(_bool ? "true" : "false");
        return ret;
    }

public:
    static inline WTSVariant* createObject()
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Object;
        ret->_value._map = ChildrenMap::create();
        return ret;
    }

    static inline WTSVariant* createArray()
    {
        WTSVariant* ret = new WTSVariant();
        ret->_type = VT_Array;
        ret->_value._array = ChildrenArray::create();
        return ret;
    }

    inline bool has(const char* key) const
    {
        if (_type != VT_Object) return false;
     
        auto it = _value._map->find(key);
        return it != _value._map->end();
    }

    inline int32_t asInt32() const
    {
        switch (_type) {
        case VT_Null:
            return 0;
        case VT_Int32:
        case VT_Uint32:
        case VT_Int64:
        case VT_Uint64:
        case VT_Real:
        case VT_String:
            return _value._string ? (int32_t) atof(_value._string->c_str()) : 0;
        default:
            return 0;
        }
    }

    inline uint32_t asUInt32() const
    {
        switch (_type) {
        case VT_Null:
            return 0;
        case VT_Int32:
        case VT_Uint32:
        case VT_Int64:
        case VT_Uint64:
        case VT_Real:
        case VT_String:
            return _value._string ? (uint32_t) atof(_value._string->c_str()) : 0;
        default:
            return 0;
        }
    }

    inline int64_t asInt64() const
    {
        switch (_type) {
        case VT_Null:
            return 0;
        case VT_Int32:
        case VT_Uint32:
        case VT_Int64:
        case VT_Uint64:
        case VT_Real:
        case VT_String:
            return _value._string ? strtoll(_value._string->c_str(), NULL, 10) : 0;
        default:
            return 0;
        }
    }

    inline uint64_t asUInt64() const
    {
        switch (_type) {
        case VT_Null:
            return 0;
        case VT_Int32:
        case VT_Uint32:
        case VT_Int64:
        case VT_Uint64:
        case VT_Real:
        case VT_String:
            return _value._string ? strtoull(_value._string->c_str(), NULL, 10) : 0;
        default:
            return 0;
        }
    }

    inline double asDouble() const
    {
        switch (_type) {
        case VT_Null:
            return 0.0;
        case VT_Int32:
        case VT_Uint32:
        case VT_Int64:
        case VT_Uint64:
        case VT_Real:
        case VT_String:
            return _value._string ? strtod(_value._string->c_str(), NULL) : 0.0;
        default:
            return 0.0;
        }
    }

    inline std::string asString() const
    {
        if (_type != VT_Object && _type != VT_Array && _value._string)
            return *_value._string;
        return "";
    }

    inline const char* asCString() const
    {
        if (_type != VT_Object && _type != VT_Array && _value._string)
            return _value._string->c_str();
        return "";
    }

    inline bool asBoolean() const
    {
        if (_value._string)
            return wt_stricmp(_value._string->c_str(), "true") == 0 
                   || wt_stricmp(_value._string->c_str(), "yes") == 0;
        return false;
    }

    inline int32_t getInt32(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asInt32();
        return 0;
    }

    inline uint32_t getUInt32(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asUInt32();
        return 0;
    }

    inline int64_t getInt64(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asInt64();
        return 0;
    }

    inline uint64_t getUInt64(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asUInt64();
        return 0;
    }

    inline double getDouble(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asDouble();
        return 0.0;
    }

    inline std::string getString(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asString();
        return "";
    }

    inline const char* getCString(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asCString();
        return "";
    }

    inline bool getBoolean(const char* name) const
    {
        WTSVariant* p = get(name);
        if (p) return p->asBoolean();
        return false;
    }

    inline WTSVariant* get(const char* name) const
    {
        if (_type != VT_Object) return NULL;
        if (!_value._map) return NULL;
     
        WTSVariant* ret = static_cast<WTSVariant*>(_value._map->get(name));
        return ret;
    }

    inline WTSVariant* get(const std::string& name) const
    {
        if (_type != VT_Object) return NULL;
        if (!_value._map) return NULL;
     
        WTSVariant* ret = static_cast<WTSVariant*>(_value._map->get(name));
        return ret;
    }

    inline WTSVariant* get(uint32_t idx) const
    {
        if (_type != VT_Array) return NULL;
        if (!_value._array) return NULL;
     
        WTSVariant* ret = static_cast<WTSVariant*>(_value._array->at(idx));
        return ret;
    }

#define append_map_scalar(key, value) \
    if (_type != VT_Object) return false; \
    if (!_value._map) _value._map = ChildrenMap::create(); \
    WTSVariant* item = WTSVariant::create(value); \
    _value._map->add(key, item, false); \
    return true

    inline bool append(const char* _name, const char* _string)
    {
        append_map_scalar(_name, _string);
    }

    inline bool append(const char* _name, int32_t _i32)
    {
        append_map_scalar(_name, _i32);
    }

    inline bool append(const char* _name, uint32_t _u32)
    {
        append_map_scalar(_name, _u32);
    }

    inline bool append(const char* _name, int64_t _i64)
    {
        append_map_scalar(_name, _i64);
    }

    inline bool append(const char* _name, uint64_t _u64)
    {
        append_map_scalar(_name, _u64);
    }

    inline bool append(const char* _name, double _real)
    {
        append_map_scalar(_name, _real);
    }

    inline bool append(const char* _name, bool _bool)
    {
        append_map_scalar(_name, _bool);
    }

    inline bool append(const char* _name, WTSVariant *item, bool bAutoRetain = true)
    {
        if (_type != VT_Object || !item) return false;
     
        if (!_value._map) _value._map = ChildrenMap::create();
        _value._map->add(_name, item, bAutoRetain);
     
        return true;
    }

#define append_array_scalar(value) \
    if (_type != VT_Array) return false; \
    if (!_value._array) _value._array = ChildrenArray::create(); \
    WTSVariant* item = WTSVariant::create(value); \
    _value._array->append(item, false); \
    return true

    inline bool append(const char* _str)
    {
        append_array_scalar(_str);
    }

    inline bool append(int32_t _i32)
    {
        append_array_scalar(_i32);
    }

    inline bool append(uint32_t _u32)
    {
        append_array_scalar(_u32);
    }

    inline bool append(int64_t _i64)
    {
        append_array_scalar(_i64);
    }

    inline bool append(uint64_t _u64)
    {
        append_array_scalar(_u64);
    }

    inline bool append(double _real)
    {
        append_array_scalar(_real);
    }

    inline bool append(bool _bool)
    {
        append_array_scalar(_bool);
    }

    inline bool append(WTSVariant* item, bool bAutoRetain = true)
    {
        if (_type != VT_Array || !item) return false;
     
        if (!_value._array) _value._array = ChildrenArray::create();
        _value._array->append(item, bAutoRetain);
     
        return true;
    }

    inline uint32_t size() const
    {
        if (_type != VT_Array && _type != VT_Object) return 0;
        if (_type == VT_Array) 
            return _value._array ? _value._array->size() : 0;
        return _value._map ? _value._map->size() : 0;
    }

    inline MemberNames memberNames() const
    {
        MemberNames names;
        if (_type == VT_Object && _value._map) 
            for (auto it = _value._map->begin(); it != _value._map->end(); ++it)
                names.emplace_back(it->first);
        return std::move(names);
    }

    virtual void release()
    {
        if (isSingleRefs()) {
            switch (_type) {
            case VT_Array:
                if (_value._array) _value._array->release();
                break;
            case VT_Object:
                if (_value._map) _value._map->release();
                break;
            default:
                if (_value._string) delete _value._string;
                break;
            }
        }
        WTSObject::release();
    }

    inline ValueType type() const { return _type; }

    inline bool isArray() const { return _type == VT_Array; }
    inline bool isObject() const { return _type == VT_Object; }

private:
    union ValueHolder
    {
        std::string*	_string;
        ChildrenMap*	_map;
        ChildrenArray*	_array;
    };
    ValueHolder	_value;
    ValueType	_type;
};

NS_WTP_END
