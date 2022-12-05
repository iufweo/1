#include <fstream>
#include <string>

std::string readAll(std::ifstream &is) {
  std::string ret;
  char *buf;

  is.seekg(0, std::ios_base::end);
  auto num = is.tellg();
  is.seekg(0, std::ios_base::beg);

  buf = new char[num];
  is.read(buf, num);

  ret = std::string(buf, num);
  delete[] buf;

  return ret;
}
