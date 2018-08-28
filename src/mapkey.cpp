#include "bpftrace.h"
#include "mapkey.h"

namespace bpftrace {

bool MapKey::operator!=(const MapKey &k) const
{
  return args_ != k.args_;
}

size_t MapKey::size() const
{
  size_t size = 0;
  for (auto &arg : args_)
    size += arg.size;
  return size;
}

std::string MapKey::argument_type_list() const
{
  size_t n = args_.size();
  if (n == 0)
    return "[]";

  std::ostringstream list;
  list << "[";
  for (size_t i = 0; i < n-1; i++)
    list << args_.at(i) << ", ";
  list << args_.at(n-1) << "]";
  return list.str();
}

std::string MapKey::argument_value_list(BPFtrace &bpftrace,
    const std::vector<uint8_t> &data) const
{
  size_t n = args_.size();
  if (n == 0)
    return "";

  std::ostringstream list;
  list << "[";
  int offset = 0;
  for (size_t i = 0; i < n-1; i++)
  {
    const SizedType &arg = args_.at(i);
    list << argument_value(bpftrace, arg, &data.at(offset)) << ", ";
    offset += arg.size;
  }
  const SizedType &arg = args_.at(n-1);
  list << argument_value(bpftrace, arg, &data.at(offset)) << "]";
  return list.str();
}

std::string MapKey::argument_value(BPFtrace &bpftrace,
    const SizedType &arg,
    const void *data)
{
  switch (arg.type)
  {
    case Type::integer:
      switch (arg.size)
      {
        case 8:
          return std::to_string(*(int64_t*)data);
        case 4:
          return std::to_string(*(int32_t*)data);
      }
    case Type::stack:
      return bpftrace.get_stack(*(uint32_t*)data, false);
    case Type::ustack:
      return bpftrace.get_stack(*(uint32_t*)data, true);
    case Type::sym:
      return bpftrace.resolve_sym(*(uint64_t*)data);
    case Type::usym:
      return bpftrace.resolve_usym(*(uint64_t*)data);
    case Type::name:
      return bpftrace.name_ids_[*(uint64_t*)data];
    case Type::string:
      return std::string((char*)data);
  }
  abort();
}

} // namespace bpftrace
