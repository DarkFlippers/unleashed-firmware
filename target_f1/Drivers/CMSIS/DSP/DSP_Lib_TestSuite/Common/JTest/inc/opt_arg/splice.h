#ifndef _SPLICE_H_
#define _SPLICE_H_

#define SPLICE(a,b) SPLICE_1(a,b)
#define SPLICE_1(a,b) SPLICE_2(a,b)
#define SPLICE_2(a,b) a##b

#endif /* _SPLICE_H_ */
