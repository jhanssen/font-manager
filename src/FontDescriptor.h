#ifndef FONT_DESCRIPTOR_H
#define FONT_DESCRIPTOR_H
#include <napi.h>
#include <uv.h>
#include <stdlib.h>
#include <string>
#include <vector>

enum FontWeight {
  FontWeightUndefined   = 0,
  FontWeightThin        = 100,
  FontWeightUltraLight  = 200,
  FontWeightLight       = 300,
  FontWeightNormal      = 400,
  FontWeightMedium      = 500,
  FontWeightSemiBold    = 600,
  FontWeightBold        = 700,
  FontWeightUltraBold   = 800,
  FontWeightHeavy       = 900
};

enum FontWidth {
  FontWidthUndefined      = 0,
  FontWidthUltraCondensed = 1,
  FontWidthExtraCondensed = 2,
  FontWidthCondensed      = 3,
  FontWidthSemiCondensed  = 4,
  FontWidthNormal         = 5,
  FontWidthSemiExpanded   = 6,
  FontWidthExpanded       = 7,
  FontWidthExtraExpanded  = 8,
  FontWidthUltraExpanded  = 9
};

struct FontDescriptor {
public:
  std::string path;
  std::string postscriptName;
  std::string family;
  std::string style;
  FontWeight weight;
  FontWidth width;
  bool italic;
  bool monospace;

  FontDescriptor(const Napi::Object& obj) {
    postscriptName = getString(obj, "postscriptName");
    family = getString(obj, "family");
    style = getString(obj, "style");
    weight = (FontWeight) getNumber(obj, "weight");
    width = (FontWidth) getNumber(obj, "width");
    italic = getBool(obj, "italic");
    monospace = getBool(obj, "monospace");
  }

  FontDescriptor() {
    weight = FontWeightUndefined;
    width = FontWidthUndefined;
    italic = false;
    monospace = false;
  }

  FontDescriptor(const std::string &path, const std::string &postscriptName,
                 const std::string &family, const std::string &style,
                 FontWeight weight, FontWidth width, bool italic, bool monospace) {
    this->path = path;
    this->postscriptName = postscriptName;
    this->family = family;
    this->style = style;
    this->weight = weight;
    this->width = width;
    this->italic = italic;
    this->monospace = monospace;
  }

  FontDescriptor(const FontDescriptor &desc) {
    path = desc.path;
    postscriptName = desc.postscriptName;
    family = desc.family;
    style = desc.style;
    weight = desc.weight;
    width = desc.width;
    italic = desc.italic;
    monospace = desc.monospace;
  }

  Napi::Object toJSObject(const Napi::Env& env) {
    Napi::Object res = Napi::Object::New(env);

    if (!path.empty()) {
      res.Set(Napi::String::New(env, "path"), Napi::String::New(env, path));
    }
    
    if (!postscriptName.empty()) {
      res.Set(Napi::String::New(env, "postscriptName"), Napi::String::New(env, postscriptName));
    }

    if (!family.empty()) {
      res.Set(Napi::String::New(env, "family"), Napi::String::New(env, family));
    }

    if (!style.empty()) {
      res.Set(Napi::String::New(env, "style"), Napi::String::New(env, style));
    }

    res.Set(Napi::String::New(env, "weight"), Napi::Number::New(env, weight));
    res.Set(Napi::String::New(env, "width"), Napi::Number::New(env, width));
    res.Set(Napi::String::New(env, "italic"), Napi::Boolean::New(env, italic));
    res.Set(Napi::String::New(env, "monospace"), Napi::Boolean::New(env, monospace));

    return res;
  }

private:
  std::string getString(const Napi::Object &obj, const char *name) {
    Napi::Value value = obj.Get(name);

    if (value.IsString()) {
      return value.As<Napi::String>().Utf8Value();
    }

    return std::string();
  }

  int getNumber(const Napi::Object &obj, const char *name) {
    Napi::Value value = obj.Get(name);
    if (value.IsNumber()) {
      return value.As<Napi::Number>().Int32Value();
    }

    return 0;
  }

  bool getBool(const Napi::Object &obj, const char *name) {
    Napi::Value value = obj.Get(name);

    if (value.IsBoolean()) {
      return value.As<Napi::Boolean>().Value();
    }

    return false;
  }
};

class ResultSet : public std::vector<FontDescriptor *> {
public:
  ~ResultSet() {
    for (ResultSet::iterator it = this->begin(); it != this->end(); it++) {
      delete *it;
    }
  }
};

#endif
