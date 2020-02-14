#include <stdlib.h>
#include <napi.h>
#include <uv.h>
#include "FontDescriptor.h"

// these functions are implemented by the platform
ResultSet *getAvailableFonts();
ResultSet *findFonts(FontDescriptor *);
FontDescriptor *findFont(FontDescriptor *);
FontDescriptor *substituteFont(const std::string&, const std::string&);

// converts a ResultSet to a JavaScript array
Napi::Array collectResults(const Napi::Env& env, ResultSet *results) {
  Napi::Array res = Napi::Array::New(env, results->size());

  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    res.Set(i++, (*it)->toJSObject(env));
  }

  delete results;
  return res;
}

// converts a FontDescriptor to a JavaScript object
Napi::Value wrapResult(const Napi::Env& env, FontDescriptor *result) {
  if (result == nullptr)
    return env.Null();

  Napi::Object res = result->toJSObject(env);
  delete result;
  return res;
}

// holds data about an operation that will be
// performed on a background thread
struct AsyncRequest {
  uv_work_t work;
  FontDescriptor *desc;     // used by findFont and findFonts
  std::string postscriptName;     // used by substituteFont
  std::string substitutionString; // ditto
  FontDescriptor *result;   // for functions with a single result
  ResultSet *results;       // for functions with multiple results
  Napi::FunctionReference callback;  // the actual JS callback to call when we are done

  AsyncRequest(const Napi::Value &v) {
    work.data = (void *)this;
    callback = Napi::Persistent(v.As<Napi::Function>());
    desc = nullptr;
    result = nullptr;
    results = nullptr;
  }

  ~AsyncRequest() {
    if (desc)
      delete desc;
    // result/results deleted by wrapResult/collectResults respectively
  }
};

// calls the JavaScript callback for a request
void asyncCallback(uv_work_t *work, int /*status*/) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  Napi::Env env = req->callback.Env();
  Napi::HandleScope scope(env);
  Napi::AsyncContext async(env, "asyncCallback");
  Napi::Value info;

  if (req->results) {
    info = collectResults(env, req->results);
  } else if (req->result) {
    info = wrapResult(env, req->result);
  } else {
    info = env.Null();
  }

  req->callback.MakeCallback(req->callback.Value(), { info }, async);
  delete req;
}

void getAvailableFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->results = getAvailableFonts();
}

template<bool async>
Napi::Value getAvailableFonts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (async) {
    if (info.Length() < 1 || !info[0].IsFunction())
      throw Napi::TypeError::New(env, "Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[0]);
    uv_queue_work(uv_default_loop(), &req->work, getAvailableFontsAsync, (uv_after_work_cb) asyncCallback);

    return env.Undefined();
  } else {
    return collectResults(env, getAvailableFonts());
  }
}

void findFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->results = findFonts(req->desc);
}

template<bool async>
Napi::Value findFonts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsObject() || info[0].IsFunction())
    throw Napi::TypeError::New(env, "Expected a font descriptor");

  Napi::Object desc = info[0].As<Napi::Object>();
  FontDescriptor *descriptor = new FontDescriptor(desc);

  if (async) {
    if (info.Length() < 2 || !info[1].IsFunction())
      throw Napi::TypeError::New(env, "Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[1]);
    req->desc = descriptor;
    uv_queue_work(uv_default_loop(), &req->work, findFontsAsync, (uv_after_work_cb) asyncCallback);

    return env.Undefined();
  } else {
    Napi::Object res = collectResults(env, findFonts(descriptor));
    delete descriptor;
    return res;
  }
}

void findFontAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->result = findFont(req->desc);
}

template<bool async>
Napi::Value findFont(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsObject() || info[0].IsFunction())
    throw Napi::TypeError::New(env, "Expected a font descriptor");

  Napi::Object desc = info[0].As<Napi::Object>();
  FontDescriptor *descriptor = new FontDescriptor(desc);

  if (async) {
    if (info.Length() < 2 || !info[1].IsFunction())
      throw Napi::TypeError::New(env, "Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[1]);
    req->desc = descriptor;
    uv_queue_work(uv_default_loop(), &req->work, findFontAsync, (uv_after_work_cb) asyncCallback);

    return env.Undefined();
  } else {
    Napi::Value res = wrapResult(env, findFont(descriptor));
    delete descriptor;
    return res;
  }
}

void substituteFontAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->result = substituteFont(req->postscriptName, req->substitutionString);
}

template<bool async>
Napi::Value substituteFont(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsString())
    throw Napi::TypeError::New(env, "Expected postscript name");

  if (info.Length() < 2 || !info[1].IsString())
    throw Napi::TypeError::New(env, "Expected substitution string");

  const std::string postscriptName = info[0].As<Napi::String>().Utf8Value();
  const std::string substitutionString = info[1].As<Napi::String>().Utf8Value();

  if (async) {
    if (info.Length() < 3 || !info[2].IsFunction())
      throw Napi::TypeError::New(env, "Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[2]);
    req->postscriptName = postscriptName;
    req->substitutionString = substitutionString;
    uv_queue_work(uv_default_loop(), &req->work, substituteFontAsync, (uv_after_work_cb) asyncCallback);

    return env.Undefined();
  } else {
    return wrapResult(env, substituteFont(postscriptName, substitutionString));
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("getAvailableFonts", Napi::Function::New(env, getAvailableFonts<true>));
    exports.Set("getAvailableFontsSync", Napi::Function::New(env, getAvailableFonts<false>));
    exports.Set("findFonts", Napi::Function::New(env, findFonts<true>));
    exports.Set("findFontsSync", Napi::Function::New(env, findFonts<false>));
    exports.Set("findFont", Napi::Function::New(env, findFont<true>));
    exports.Set("findFontSync", Napi::Function::New(env, findFont<false>));
    exports.Set("substituteFont", Napi::Function::New(env, substituteFont<true>));
    exports.Set("substituteFontSync", Napi::Function::New(env, substituteFont<false>));
    return exports;
}

NODE_API_MODULE(fontmanager, Init)
