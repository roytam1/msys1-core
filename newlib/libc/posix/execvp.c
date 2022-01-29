/* execvp.c */

/* This and the other exec*.c files in this directory require 
   the target to provide the _execve syscall.  */

#include <_ansi.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#if defined(__CYGWIN__) || defined(__MSYS__)
static char path_delim;
#define PATH_DELIM path_delim
#else
#define PATH_DELIM ':'
#endif

int
_DEFUN (execvp, (file, argv),
	_CONST char *file _AND
	char * _CONST argv[])
{
  char *path = getenv ("PATH");
  char buf[MAXNAMLEN];
  size_t file_len;

  /* If $PATH doesn't exist, just pass FILE on unchanged.  */
  if (!path)
    return execv (file, argv);

  /* If FILE contains a directory, don't search $PATH.  */
  if (strchr (file, '/')
#if defined(__CYGWIN__) || defined(__MSYS__)
      || strchr (file, '\\')
#endif
      )
    return execv (file, argv);

#if defined(__CYGWIN__) || defined(__MSYS__)
  /* If a drive letter is passed, the path is still an absolute one.
     Technically this isn't true, but Cygwin is currently defined so
     that it is.  */
  if ((isalpha (file[0]) && file[1] == ':')
      || file[0] == '\\')
    return execv (file, argv);
#endif

#if defined(__CYGWIN__) || defined(__MSYS__)
  path_delim = cygwin_posix_path_list_p (path) ? ':' : ';';
#endif

  file_len = strlen (file);
  while (*path)
    {
      /* Find the length of the path element */
      char *path_ptr = strchr (path, PATH_DELIM);
      size_t path_len;
      if (path_ptr)
        path_len = path_ptr - path;
      else
        path_len = strlen (path);
      /* Check if the combined pathname fits into the buffer */
      if (path_len + file_len + 2 > MAXNAMLEN)
        errno = ENAMETOOLONG;
      else {
        /* Combine the path element with the file name */
        char *end_of_string = buf;
        memcpy (end_of_string, path, path_len);
        end_of_string += path_len;
        /* An empty entry means the current directory.  */
        if (path_len != 0 && path[path_len - 1] != '/')
          *end_of_string++ = '/';
        memcpy (end_of_string, file, file_len);
        end_of_string += file_len;
        *end_of_string = 0;
        /* Try executing the resulting pathname */
        if (execv (buf, argv) == -1 && errno != ENOENT)
          return -1;
      }
      path += path_len;
      if (*path == PATH_DELIM)
	path++;			/* skip over delim */
    }

  return -1;
}
