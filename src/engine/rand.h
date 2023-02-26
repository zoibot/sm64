#ifndef _RAND_H_
#define _RAND_H_

void init_genrand(unsigned long s);
void genrand_push();
void genrand_pop();

unsigned long genrand_int32(void);

#endif /* _RAND_H_ */
