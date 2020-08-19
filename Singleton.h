#pragma once
// �̰߳�ȫ�ĵ���ģʽ����ҪC++11��static����

// ����ģʽ�����꣬�ŵ��ඨ����
#define PATTERN_SINGLETON_DECLARE(ClassName)        \
 private:                                           \
  ClassName();                                      \
  ClassName(const ClassName &) = delete;            \
  ClassName &operator=(const ClassName &) = delete; \
                                                    \
 public:                                            \
  static ClassName *Instance();                     \
  virtual ~ClassName();

// ����ģʽʵ�ֺ꣬�ŵ���ʵ�ִ�
#define PATTERN_SINGLETON_IMPLEMENT(ClassName) \
  ClassName *ClassName::Instance() {           \
    static ClassName _instance;                \
    return &_instance;                         \
  }

// ��չ����ģʽ
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
