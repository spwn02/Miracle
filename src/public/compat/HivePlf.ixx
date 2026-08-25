module;

#include <plf_hive.h>

export module Miracle:HiveBackend;

export namespace Miracle {

template <class T>
using HiveBackend = plf::hive<T>;

} // namespace Miracle
