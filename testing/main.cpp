#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

static const std::string skip = "Elapsed time: ";
static const std::string prefix = "// expect: ";
static const std::string notPrefixes[] = {
    "expect",
};

[[noreturn]] void err(int eval, std::string msg) {
  std::cerr << msg << '\n';
  std::exit(eval);
}

[[nodiscard]] std::runtime_error error(std::string msg) {
  return std::runtime_error(msg);
}

[[nodiscard]] int compare(std::list<std::string> expects,
                          std::list<std::string> results,
                          std::string path) {
  int ret;
  std::size_t numComps;

  auto eb = expects.cbegin();
  auto ee = expects.cend();
  auto rb = results.cbegin();
  auto re = results.cend();

  for (ret = 0, numComps = 1; eb != ee && rb != re; numComps++) {
    if (*eb != *rb) {
      std::cout << path << ", " << numComps << ", mismatch: '" << *eb << "' '"
                << *rb << "'\n";
      ret = 1;
    }
    eb++;
    rb++;
    if ((eb == ee && rb != re) || (eb != ee && rb == re)) {
      std::cout << path << ", " << numComps << ", count mismatch\n";
      ret = 1;
      break;
    }
  }

  return ret;
}

[[nodiscard]] std::list<std::string> readExpects(std::ifstream& ifs,
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

    for (const auto& np : notPrefixes) {
      if (line.find(np) == std::string::npos)
        continue;
      std::cout << path << ':' << lineNum
                << ": warning: potentially misspelled"
                   " expect: '"
                << np << "'\n";
    }
  }

  return expects;
}

[[nodiscard]] std::list<std::string> getResults(std::string cmd) {
  std::list<std::string> results;

  std::FILE* fp;
  std::unique_ptr<char[]> ptr, tmp;
  char* buf;

  std::istringstream in;
  std::string line;

  std::size_t nr;
  std::size_t size;
  std::size_t total;

  if ((fp = popen(cmd.c_str(), "r")) == nullptr)
    throw error("popen: " + cmd + ": " + std::strerror(errno));

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
    throw error(std::string("fread: ") + std::strerror(errno));

  in.str(ptr.get());
  while (std::getline(in, line)) {
    if (line.starts_with(skip))
      continue;
    if (line.ends_with('\n'))
      line.erase(line.size() - 1, 1);
    results.push_back(line);
  }

  if (pclose(fp) == -1)
    throw error(std::string("pclose: ") + std::strerror(errno));

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
  for (const auto& ent : fs::recursive_directory_iterator(dir)) {
    std::ifstream ifs;

    str = ent.path().string();
    switch (fs::status(ent).type()) {
      case fs::file_type::directory:
        continue;
      case fs::file_type::regular:
        break;
      default:
        throw error(str + " is not a regular file or directory");
    }

    ifs.open(ent.path(), std::ios::binary);
    if (!ifs)
      throw error("std::ifstream::open: " + str);
    if ((expects = readExpects(ifs, str)).empty()) {
      numNoExpects++;
      std::cout << str << ": no expects\n";
      continue;
    }
    cmd = binPath + " " + str + " 2>&1";
    if ((results = getResults(cmd)).empty()) {
      numNoResults++;
      std::cout << str << ": no results\n";
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

  std::cout << "Tests: "
            << numFails + numSuccesses + numNoExpects + numNoResults
            << ", fails: " << numFails << ", success count: " << numSuccesses
            << ", no expects: " << numNoExpects
            << ", no results: " << numNoResults << '\n';
  return ret;
}

int main(int argc, char* argv[]) {
  int ret;

  fs::directory_entry bin;
  fs::directory_entry dir;

  if (argc != 3)
    err(1, "argc = " + std::to_string(argc));

  bin = fs::directory_entry(fs::path(argv[1]));
  if (!fs::exists(bin))
    err(2, bin.path().string() + " does not exist");
  if (!fs::is_regular_file(bin))
    err(2, bin.path().string() + " is not a regular file");
  if ((fs::status(bin).permissions() & fs::perms::owner_exec) ==
      fs::perms::none)
    err(2, bin.path().string() + " is not executable");
  dir = fs::directory_entry(fs::path(argv[2]));
  if (!fs::exists(dir))
    err(2, dir.path().string() + " does not exist");
  if (!fs::is_directory(dir))
    err(2, dir.path().string() + " is not a directory");
  try {
    ret = traverseDir(bin.path().string(), dir);
  } catch (std::runtime_error& e) {
    std::cerr << "error: " << e.what() << '\n';
    ret = 2;
  }

  return ret;
}
