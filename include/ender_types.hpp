#include <memory>
typedef unsigned int uint;

template<typename T>
using uptr = std::unique_ptr<T>;

template<typename T>
using sptr = std::shared_ptr<T>;
