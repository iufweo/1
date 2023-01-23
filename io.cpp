#include <fstream>
#include <string>

std::string readAll(std::ifstream& is) {
  std::string ret;
  char* buf;

  is.seekg(0, std::ios_base::end);
  size_t num = (size_t)is.tellg();
  is.seekg(0, std::ios_base::beg);

  buf = new char[num];
  is.read(buf, (std::streamsize)num);

  ret = std::string(buf, num);
  delete[] buf;

  return ret;
}
