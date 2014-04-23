#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef _TINY_BUS_DEBUG_

    #define show_err(name) \
    do {\
		fprintf(stderr, "file %s: line %d (%s): TINY_BUS_DEBUG: %s",	\
           __FILE__, __LINE__, __FUNCTION__, name);				\
    } while (0)       
    
    #define show_err2(err, name) \
    do {\
    	int error = (err); 							\
    	if (error)	 		 		 			\
    		fprintf(stderr, "file %s: line %d (%s): error '%d' during '%s'",	\
               __FILE__, __LINE__, __FUNCTION__, err, name);				\
    } while (0)
    
#else
    #define show_err(name)
    #define show_err2(err, name)
    
#endif

#define posix_check_cmd(cmd) show_err2((cmd), #cmd)

#endif
