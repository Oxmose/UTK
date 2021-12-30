/*******************************************************************************
 * @file string.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 27/02/2021
 *
 * @version 1.0
 *
 * @brief Kernel's strings and memory manipulation functions.
 *
 * @details Strings and memory manipulation functions.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __LIB_STRING_H_
#define __LIB_STRING_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stddef.h> /* Standard integer definitions */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Copies characters from the object pointed to by src to the object
 * pointed to by dest.
 *
 * @param[out] dest Pointer to the object to copy to
 * @param[in] src Pointer to the object to copy from
 * @param[in] c Terminating character, converted to unsigned char at first
 * @param[in] count Number of characters to copy
 *
 * @return If the character c was found memccpy returns a pointer
 * to the next character in dest after c, otherwise returns NULL
 * pointer.
 */
void *memccpy(void * restrict dest,
              const void * restrict src,
              int c,
              size_t count);

/**
 * @brief Finds the first occurrence of ch (after conversion to unsigned char as
 * if by ch) in the initial count characters (each interpreted as unsigned char)
 * of the object pointed to by ptr.
 *
 * @param ptr Pointer to the object to be examined
 * @param ch Character to search for
 * @param count Max number of characters to examine
 *
 * @return Pointer to the location of the character, or a null pointer if no
 * such character is found.
 */
void* memchr(const void* ptr, int ch, size_t count);

void *memrchr(const void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
void *memmem(const void *, size_t, const void *, size_t);
void memswap(void *, void *, size_t);
int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, size_t);
char *strcat(char *, const char *);
char *strchr(const char *, int);
char *strrchr(const char *, int);
int strcmp(const char *, const char *);
char *strcpy(char *, const char *);
size_t strcspn(const char *, const char *);
char *strdup(const char *);
char *strndup(const char *, size_t);
char *strerror(int);
char *strsignal(int);
size_t strlen(const char *);
size_t strnlen(const char *, size_t);
char *strncat(char *, const char *, size_t);
size_t strlcat(char *, const char *, size_t);
int strncmp(const char *, const char *, size_t);
char *strncpy(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
char *strpbrk(const char *, const char *);
char *strsep(char **, const char *);
size_t strspn(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char *, const char *);
size_t __strxspn(const char *s, const char *map, int parity);

#endif /* #ifndef __LIB_STRING_H_ */

/************************************ EOF *************************************/
