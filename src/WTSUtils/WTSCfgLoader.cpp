#include "WTSCfgLoader.h"
#include "../Share/StrUtil.hpp"
#include "../Share/StdUtils.hpp"
#include "../Share/charconv.hpp"
#include "../Includes/WTSVariant.hpp"

#include <rapidjson/document.h>
namespace rj = rapidjson;

static bool json_to_variant(const rj::Value& root, WTSVariant* params);

static inline void _add_key_obj_variant(WTSVariant* _obj, 
                                        const char* _key, const rj::Value& _item)
{
    WTSVariant* subObj = WTSVariant::createObject();
    if (json_to_variant(_item, subObj)) _obj->append(_key, subObj, false);
}

static inline void _add_key_array_variant(WTSVariant* _obj, 
                                          const char* _key, const rj::Value& _item)
{
    WTSVariant* subAy = WTSVariant::createArray();
    if (json_to_variant(_item, subAy)) _obj->append(_key, subAy, false);
}

static inline void _add_key_int_variant(WTSVariant* _obj, 
                                        const char* _key, const rj::Value& _item)
{
    if (_item.IsInt())
        _obj->append(_key, _item.GetInt());
    else if (_item.IsUint())
        _obj->append(_key, _item.GetUint());
    else if (_item.IsInt64())
        _obj->append(_key, _item.GetInt64());
    else if (_item.IsUint64())
        _obj->append(_key, _item.GetUint64());
    else if (_item.IsDouble())
        _obj->append(_key, _item.GetDouble());
}

static inline void _add_obj_variant(WTSVariant* _arr, const rj::Value& _item)
{
    WTSVariant* subObj = WTSVariant::createObject();
    if (json_to_variant(_item, subObj)) _arr->append(subObj, false);
}

static inline void _add_array_variant(WTSVariant* _arr, const rj::Value& _item)
{
    WTSVariant* subAy = WTSVariant::createArray();
    if (json_to_variant(_item, subAy)) _arr->append(subAy, false);
}

static inline void _add_int_variant(WTSVariant* _arr, const rj::Value& _item)
{
    if (_item.IsInt())
        _arr->append(_item.GetInt());
    else if (_item.IsUint())
        _arr->append(_item.GetUint());
    else if (_item.IsInt64())
        _arr->append(_item.GetInt64());
    else if (_item.IsUint64())
        _arr->append(_item.GetUint64());
    else if (_item.IsDouble())
        _arr->append(_item.GetDouble());
}

static bool json_to_variant(const rj::Value& root, WTSVariant* params)
{
    if (root.IsObject() && params->type() != WTSVariant::VT_Object) return false;
    if (root.IsArray() && params->type() != WTSVariant::VT_Array) return false;

    if (root.IsObject()) {
        for (auto& m : root.GetObject()) {
            const char* key = m.name.GetString();
            const rj::Value& item = m.value;
            switch (item.GetType()) {
            case rj::kObjectType:
                _add_key_obj_variant(params, key, item);
                break;
            case rj::kArrayType:
                _add_key_array_variant(params, key, item);
                break;
            case rj::kNumberType:
                _add_key_int_variant(params, key, item);
                break;
            case rj::kStringType:
                params->append(key, item.GetString());
                break;
            case rj::kTrueType:
            case rj::kFalseType:
                params->append(key, item.GetBool());
                break;
            }
        }
    }
    else {
        for (auto& item : root.GetArray()) {
            switch (item.GetType()) {
            case rj::kObjectType:
                _add_obj_variant(params, item);
                break;
            case rj::kArrayType:
                _add_array_variant(params, item);
                break;
            case rj::kNumberType:
                _add_int_variant(params, item);
                break;
            case rj::kStringType:
                params->append(item.GetString());
                break;
            case rj::kTrueType:
            case rj::kFalseType:
                params->append(item.GetBool());
                break;
            }
        }
    }

    return true;
}

WTSVariant* WTSCfgLoader::load_from_json(const char* content)
{
    rj::Document root;
    root.Parse(content);
    if (root.HasParseError()) return NULL;

    WTSVariant* ret = WTSVariant::createObject();
    if (!json_to_variant(root, ret)) {
        ret->release();
        return NULL;
    }
    return ret;
}

#include "../WTSUtils/yamlcpp/yaml.h"
bool yaml_to_variant(const YAML::Node& root, WTSVariant* params)
{
	if (root.IsNull() && params->type() != WTSVariant::VT_Object)
		return false;

	if (root.IsSequence() && params->type() != WTSVariant::VT_Array)
		return false;

	bool isMap = root.IsMap();
	for (auto& m : root)
	{
		std::string key = isMap ? m.first.as<std::string>() : "";
		const YAML::Node& item = isMap ? m.second : m;
		switch (item.Type())
		{
		case YAML::NodeType::Map:
		{
			WTSVariant* subObj = WTSVariant::createObject();
			if (yaml_to_variant(item, subObj))
			{
				if(isMap)
					params->append(key.c_str(), subObj, false);
				else
					params->append(subObj, false);
			}
		}
		break;
		case YAML::NodeType::Sequence:
		{
			WTSVariant* subAy = WTSVariant::createArray();
			if (yaml_to_variant(item, subAy))
			{
				if (isMap)
					params->append(key.c_str(), subAy, false);
				else
					params->append(subAy, false);
			}
		}
		break;
		case YAML::NodeType::Scalar:
			if (isMap)
				params->append(key.c_str(), item.as<std::string>().c_str());
			else
				params->append(item.as<std::string>().c_str());
			break;
		}
	}

	return true;
}

WTSVariant* WTSCfgLoader::load_from_yaml(const char* content)
{
	YAML::Node root = YAML::Load(content);

	if (root.IsNull())
		return NULL;

	WTSVariant* ret = WTSVariant::createObject();
	if (!yaml_to_variant(root, ret))
	{
		ret->release();
		return NULL;
	}

	return ret;
}

/*
 * @isYaml: false
 */
WTSVariant* WTSCfgLoader::load_from_content(const std::string& content, bool isYaml)
{
    // 加一个自动检测编码的逻辑
    bool isUTF8 = EncodingHelper::isUtf8((unsigned char*) content.data(), content.size());
    std::string buffer;
#ifdef _WIN32 // Win下得是GBK
    if (isUTF8) buffer = UTF8toChar(content);
#else // Linux下得是UTF8
    if (!isUTF8) buffer = ChartoUTF8(content);
#endif
    if (buffer.empty()) buffer = content; // original encoding is right
    //////////////////////////////////////////////////////////////////////////
    if (!isYaml) return load_from_json(buffer.c_str());
    return load_from_yaml(buffer.c_str());
}

WTSVariant* WTSCfgLoader::load_from_file(const char* filename)
{
    if (!StdFile::exists(filename)) return NULL;
    /***************************************************************/
    std::string content;
    StdFile::read_file_content(filename, content);
    if (content.empty()) return NULL;
    /***************************************************************/
    //加一个自动检测编码的逻辑
    bool isUTF8 = EncodingHelper::isUtf8((unsigned char*)content.data(), content.size());
    //By Wesley @ 2022.01.07
#ifdef _WIN32 // Win下得是GBK
    if(isUTF8) content = UTF8toChar(content);
#else // Linux下得是UTF8
    if (!isUTF8) content = ChartoUTF8(content);
#endif
    //////////////////////////////////////////////////////////////////////////
    if (StrUtil::endsWith(filename, ".json")) return load_from_json(content.c_str());
    /***************************************************************/
    if (StrUtil::endsWith(filename, ".yaml") || StrUtil::endsWith(filename, ".yml"))
        return load_from_yaml(content.c_str());
    /***************************************************************/
    return NULL;
}
