#ifndef _COMMON_H_
#define _COMMON_H_

#define show_err(err, name) \
do {\
	int error = (err); 							\
	if (error)	 		 		 			\
		fprintf(stderr, "file %s: line %d (%s): error '%d' during '%s'",	\
           __FILE__, __LINE__, __FUNCTION__, err, name);				\
} while (0)

#define posix_check_cmd(cmd) show_err((cmd), #cmd)

#endif
