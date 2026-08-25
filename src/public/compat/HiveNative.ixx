export module Miracle:HiveBackend;

import std;

export namespace Miracle {

template <class T>
using HiveBackend = std::hive<T>;

} // namespace Miracle
