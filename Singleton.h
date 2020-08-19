#pragma once
// 线程安全的单例模式，需要C++11的static特性

// 单例模式申明宏，放到类定义中
#define PATTERN_SINGLETON_DECLARE(ClassName)        \
 private:                                           \
  ClassName();                                      \
  ClassName(const ClassName &) = delete;            \
  ClassName &operator=(const ClassName &) = delete; \
                                                    \
 public:                                            \
  static ClassName *Instance();                     \
  virtual ~ClassName();

// 单例模式实现宏，放到类实现处
#define PATTERN_SINGLETON_IMPLEMENT(ClassName) \
  ClassName *ClassName::Instance() {           \
    static ClassName _instance;                \
    return &_instance;                         \
  }

// 扩展单例模式
#include <map>
#define PATTERN_SINGLETON_EX_DECLARE(ClassName) \
 private:                                       \
  ClassName();                                  \
                                                \
 public:                                        \
  static ClassName *Instance(int channel);      \
  virtual ~ClassName();

#define PATTERN_SINGLETON_EX_IMPLEMENT(ClassName)     \
  ClassName *ClassName::Instance(int channel) {       \
    static std::map<int, ClassName *> _instance;      \
    if (_instance.find(channel) == _instance.end()) { \
      _instance[channel] = new ClassName(channel);    \
    }                                                 \
    return _instance[channel];                        \
  }
