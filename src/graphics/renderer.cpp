#include "renderer.hpp"
#include "assetmanager.hpp"
#include <iostream>
#include <map>
#include <string>
using namespace sf;
using namespace std;
using namespace hack::logic;

static const struct {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
  unsigned char  pixel_data[32 * 32 * 4 + 1];
} icon = {
  32, 32, 4,
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\24\261'\15\15\255';\14\260)"
  "j\15\257(\231\15\257(\240\14\257(\223\15\260(\207\14\256'{\16\256'n\16\260"
  "(G\0\252\0\3\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\14\256#\26\15\260(t\15\256(\256\15\257(\335"
  "\15\257(\376\15\257(\377\15\257(\377\15\257(\377\15\257(\377\15\257(\377"
  "\15\257(\377\15\257(\377\15\257(\377\15\257(\377\15\256(\344\15\257(\206"
  "\15\256(&\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\14\257(@\16\257'\250\15\257"
  "(\371\15\257(\377\15\257(\377\15\257(\377\15\257(\371\16\257(\316\15\256"
  "(\236\16\256'n\16\260)^\16\257(l\15\257(y\15\257(\206\16\257)\226\15\257"
  ")\334\15\257(\377\15\257(\377\15\257(\377\16\260)\252\0\266$\7\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\21\252\"\17\16\257)p\15\260(\327\15\257(\377"
  "\15\257(\377\15\257(\372\16\260)\252\16\257)]\13\261'.\0\2313\5\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\0\377\0\1\14\261)>\15\257(\237\14\257(\365\15"
  "\257(\377\16\257'\274\0\252\0\3\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\13\260)D\15\256(\344\15\257(\377"
  "\15\257(\377\16\257'\342\14\257'|\13\252+\30\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\13\257+0\15\257(\351\15\257(\377\15\256"
  "(_\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\0\237\40\10\14"
  "\257'\217\15\257(\376\15\257(\377\15\260(\307\15\261'N\0\252\0\3\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\15\256(_\15\257(\377\14\257(\315\377\377\377\0\377"
  "\377\377\0\377\377\377\0\13\256&/\15\256(\324\15\257(\377\15\257(\372\15"
  "\260(z\0\252\0\3\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\14\252$\25\15\257(\377\15\257(\352\377\377\377\0\377\377\377\0\16\254"
  ")%\15\257(\362\15\257(\377\15\260(\327\17\257(3\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\17\257$#\15"
  "\257(\377\15\257(\354\377\377\377\0\0\266$\7\16\257'\317\15\257(\377\15\257"
  "(\262\0\252\34\11\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\15\260(z\15\257(\377\15\256"
  "(\253\377\377\377\0\16\257)\226\15\257(\377\15\257(\335\21\252\"\17\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\17\255)2\15\257(\370\15\257(\377\15\256'O\15\260(M\15"
  "\257(\377\15\257(\370\17\255)2\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\21\252\"\17\15"
  "\257(\335\15\257(\377\15\260(\232\377\377\377\0\15\256(\253\15\257(\377\15"
  "\260(z\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\0\252\34\11\15\257(\262\15\257(\377\15"
  "\257(\320\0\237\40\10\377\377\377\0\15\257(\354\15\257(\377\17\257$#\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\17\257(3\15\260(\327\15\257(\377\15\257(\362\16\254)%\377\377"
  "\377\0\377\377\377\0\15\257(\352\15\257(\377\14\252$\25\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\0\252\0\3\15\260(z\15\257(\372"
  "\15\257(\377\15\256(\324\13\256&/\377\377\377\0\377\377\377\0\377\377\377"
  "\0\14\257(\315\15\257(\377\15\256(_\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\0\252"
  "\0\3\15\261'N\15\260(\307\15\257(\377\15\257(\376\14\257'\217\0\237\40\10"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\15\256(_\15\257"
  "(\377\15\257(\351\13\257+0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\13\252+\30\14\257'|\16\257'\342\15\257(\377\15\257(\377"
  "\15\256(\344\13\260)D\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\0\252\0\3\16\257'\274\15\257(\377\14\257("
  "\365\15\257(\237\14\261)>\0\377\0\1\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\0\2313\5\13\261'.\16\257)]\16\260)\252\15\257(\372\15\257(\377\15\257(\377"
  "\15\260(\327\16\257)p\21\252\"\17\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\0\266$\7\16\260)\252\15\257(\377\15\257(\377\15\257(\377\15\257)\334\16"
  "\257)\226\15\257(\206\15\257(y\16\257(l\16\260)^\16\256'n\15\256(\236\16"
  "\257(\316\15\257(\371\15\257(\377\15\257(\377\15\257(\377\15\257(\371\16"
  "\257'\250\14\257(@\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\15\256(&\15\257"
  "(\206\15\256(\344\15\257(\377\15\257(\377\15\257(\377\15\257(\377\15\257"
  "(\377\15\257(\377\15\257(\377\15\257(\377\15\257(\377\15\257(\376\15\257"
  "(\335\15\256(\256\15\260(t\14\256#\26\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\0\252\0\3\16\260"
  "(G\16\256'n\14\256'{\15\260(\207\14\257(\223\15\257(\240\15\257(\231\14\260"
  ")j\15\255';\24\261'\15\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0",
};

renderer::renderer(int width, int height)
	: window(new RenderWindow(sf::VideoMode(width, height), "hack game")), input(new inputmanager(window))
{
	window->setFramerateLimit(60);
	window->setIcon(icon.width,icon.height,icon.pixel_data);
}

renderer::~renderer(void)
{
	entities.clear();
}

void renderer::run() {
	sf::Text text("hack game");
	
    while (window->isOpen())
    {
		//clear screen
		window->clear();
        
		//handle inputs
		input->tick();
		
        //test text
        window->draw(text);
		
		//render all registered entities
		renderAll();

		//show everything rendered on screen
		window->display();
	}
}

void renderer::renderAll() {
	//iterate over all registered entities
	map<entity*,asset*>::iterator iter;
	for(iter = entities.begin(); iter != entities.end(); iter++) {
		//set position and rotation of entity
		iter->second->setPosition((float)iter->first->getX(), (float)iter->first->getY());
		iter->second->setRotation(iter->first->getAngle());
		//draw sprite
		window->draw(iter->second->getSprite());
	}
}

void renderer::registerEntity(entity& e, const char* typeName) {
	entities[&e] = am.get(typeName);
	entities[&e]->setPosition(e.getX(),e.getY());
	entities[&e]->setRotation(e.getAngle());
	entities[&e]->setSize(e.getWidth(),e.getHeight());
}


void renderer::deleteEntity(entity& e) {
	entities.erase(&e);
}