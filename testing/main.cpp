#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <fmt/core.h>

namespace fs = std::filesystem;

static const std::string skip = "Elapsed time: ";
static const std::string prefix = "// expect: ";
static const std::string notPrefixes[] = {
    "expect",
};

template <typename... Ts>
[[noreturn]] void err(int eval, fmt::format_string<Ts...> fmt, Ts &&...args) {
  fmt::print(stderr, fmt, std::forward<Ts>(args)...);
  fmt::print("\n");
  std::exit(eval);
}

template <typename... Ts>
[[nodiscard]] std::runtime_error error(fmt::format_string<Ts...> fmt,
                                       Ts &&...args) {
  return std::runtime_error(fmt::format(fmt, std::forward<Ts>(args)...));
}

[[nodiscard]] int compare(std::list<std::string> expects,
                          std::list<std::string> results, std::string path) {
  int ret;
  std::size_t numComps;

  auto eb = expects.cbegin();
  auto ee = expects.cend();
  auto rb = results.cbegin();
  auto re = results.cend();

  for (ret = 0, numComps = 1; eb != ee && rb != re; numComps++) {
    if (*eb != *rb) {
      fmt::print("{}, {}, mismatch: '{}' '{}'\n", path, numComps, *eb, *rb);
      ret = 1;
    }
    eb++;
    rb++;
    if ((eb == ee && rb != re) || (eb != ee && rb == re)) {
      fmt::print("{}, {}, count mismatch\n", path, numComps);
      ret = 1;
      break;
    }
  }

  return ret;
}

[[nodiscard]] std::list<std::string> readExpects(std::ifstream &ifs,
                                                 std::string path) {
  std::list<std::string> expects;
  std::size_t lineNum;
  std::string line;
  std::size_t pos;

  for (lineNum = 1; std::getline(ifs, line); lineNum++) {
    if ((pos = line.find(prefix)) != std::string::npos) {
      expects.push_back(line.substr(pos + prefix.size()));
      continue;
    }

    for (const auto &np : notPrefixes) {
      if (line.find(np) == std::string::npos)
        continue;
      fmt::print("{}:{}: warning: potentially misspelled"
                 " expect: '{}'\n",
                 path, lineNum, np);
    }
  }

  return expects;
}

[[nodiscard]] std::list<std::string> getResults(std::string cmd) {
  std::list<std::string> results;

  std::FILE *fp;
  std::unique_ptr<char[]> ptr, tmp;
  char *buf;

  std::istringstream in;
  std::string line;
  std::size_t nr;
  std::size_t size;
  std::size_t total;

  if ((fp = popen(cmd.c_str(), "r")) == nullptr)
    throw error("popen: {}: {}", cmd, std::strerror(errno));

  size = 1024;
  ptr = std::make_unique<char[]>(size);
  buf = ptr.get();
  for (total = 0; (nr = fread(buf + total, 1, size - total, fp)) > 0;) {
    tmp = std::make_unique<char[]>(size * 2);
    std::memcpy(tmp.get(), ptr.get(), size);
    size *= 2;

    ptr = std::move(tmp);
    buf = ptr.get();
    total += nr;
  }
  if (errno != 0)
    throw error("fread: {}", std::strerror(errno));

  in.str(ptr.get());
  while (std::getline(in, line)) {
    if (line.starts_with(skip))
      continue;
    if (line.ends_with('\n'))
      line.erase(line.size() - 1, 1);
    results.push_back(line);
  }

  if (pclose(fp) == -1)
    throw error("pclose: {}", std::strerror(errno));

  return results;
}

[[nodiscard]] int traverseDir(std::string binPath, fs::directory_entry dir) {
  int ret;

  std::size_t numFails, numSuccesses;
  std::size_t numNoExpects, numNoResults;

  std::list<std::string> expects;
  std::list<std::string> results;

  std::string cmd;
  std::string str;

  ret = 0;
  numFails = numSuccesses = numNoExpects = numNoResults = 0;
  for (const auto &ent : fs::recursive_directory_iterator(dir)) {
    std::ifstream ifs;

    str = ent.path().string();
    switch (fs::status(ent).type()) {
    case fs::file_type::directory:
      continue;
    case fs::file_type::regular:
      break;
    default:
      throw error("{} is not a regular file or directory", str);
    }

    ifs.open(ent.path(), std::ios::binary);
    if (!ifs)
      throw error("std::ifstream::open {}", str);
    if ((expects = readExpects(ifs, str)).empty()) {
      numNoExpects++;
      fmt::print("{}: no expects\n", str);
      continue;
    }
    cmd = binPath + " " + str + " 2>&1";
    if ((results = getResults(cmd)).empty()) {
      numNoResults++;
      fmt::print("{}: no results\n", str);
      continue;
    }
    switch (compare(expects, results, str)) {
    case 0:
      numSuccesses++;
      break;
    case 1:
      numFails++;
      ret = 1;
      break;
    }

  }

  fmt::print("Tests: {}, fails: {}, successes: {}, no expects: {},"
             " no results: {}\n",
             numFails + numSuccesses + numNoExpects + numNoResults, numFails,
             numSuccesses, numNoExpects, numNoResults);
  return ret;
}

int main(int argc, char *argv[]) {
  int ret;

  fs::directory_entry bin;
  fs::directory_entry dir;

  if (argc != 3)
    err(2, "argc = {}", argc);

  bin = fs::directory_entry(fs::path(argv[1]));
  if (!fs::exists(bin))
    err(2, "{} does not exist", bin.path().string());
  if (!fs::is_regular_file(bin))
    err(2, "{} is not a regular file", bin.path().string());
  if ((fs::status(bin).permissions() & fs::perms::owner_exec) ==
      fs::perms::none)
    err(2, "{} is not executable", bin.path().string());
  dir = fs::directory_entry(fs::path(argv[2]));
  if (!fs::exists(dir))
    err(2, "{} does not exist", dir.path().string());
  if (!fs::is_directory(dir))
    err(2, "{} is not a directory", dir.path().string());
  try {
    ret = traverseDir(bin.path().string(), dir);
  } catch (std::runtime_error &e) {
    fmt::print(stderr, "error: {}\n", e.what());
    ret = 2;
  }

  return ret;
}
