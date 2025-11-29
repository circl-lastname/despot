#include <stdio.h>
#include <despot-metadata.h>

int main() {
  char* string = "Hello World";
  despot_ctx_t* ctx;
  despot_read_from_mem(&ctx, string, 12);
  
  /*
  FILE* file = fopen("BuildInfo", "rb");
  despot_ctx_t* ctx;
  despot_read_from_file(&ctx, file);
  */
  
  return 0;
}
