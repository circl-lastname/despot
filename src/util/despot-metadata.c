#include <stdio.h>
#include <despot-metadata.h>

void try(char* filename) {
  FILE* file = fopen(filename, "rb");
  despot_ctx_t* ctx;
  puts(despot_result_to_string(despot_read_from_file(&ctx, file)));
  despot_free_ctx(ctx);
  fclose(file);
}

int main() {
  //try("test/03 - Amnesia Was Her Name.flac");
  try("test/soundgirls.flac");
  //try("test/empty");
  //try("test/notflac");
  
  return 0;
}
