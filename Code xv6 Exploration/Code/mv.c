#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char buf[512];
// IVANAVA: This command performe two funcitons 1) The move function in simple form 2) Rename function
void mv(const char *src, const char *dest)
{
  // Open Source File for Reading
  int fd_src = open(src, O_RDONLY);
  if (fd_src < 0)
  {
    printf("mv: cannot open source file\n");
    exit(1);
  }
  // Open or Create Destination File for Writing
  int fd_dest = open(dest, O_WRONLY | O_CREATE);
  if (fd_dest < 0)
  {
    printf("mv: cannot create/modify destination file\n");
    close(fd_src);
    exit(1);
  }
  // Copy Content from Source to Destination
  int n;
  while ((n = read(fd_src, buf, sizeof(buf))) > 0)
  {
    write(fd_dest, buf, n);
  }
  // Close Files
  close(fd_src);
  close(fd_dest);

  // void delete_file(const char *path)
  // {
  //   if (unlink(path) < 0)
  //   {
  //     printf("mv: cannot delete file\n");
  //     exit(1);
  //   }
}

int main(int argc, char *argv[])
{
  // Check if the right was write the command
  if (argc != 3)
  {
    printf("Usage: mv source_file dest_file\n");
    exit(1);
  }

  if (strcmp(argv[1], argv[2]) == 0)
  {
    printf("mv: source and destination file names are the same\n");
    exit(1);
  }
  // Rename fucntion
  if (link(argv[1], argv[2]) == 0 && unlink(argv[1]) == 0)
  {
    exit(0);
  }
  // Copy the content of the source file to the destination file
  mv(argv[1], argv[2]);

  // Delete the original source file
  // delete_file(argv[1]);

  exit(0);
}
